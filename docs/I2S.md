# I2S y el proyecto MAX98357A + OLED

Documentación técnica de cómo funciona el audio I2S en este proyecto, los
problemas encontrados y cómo convivir el amplificador MAX98357A con un
micrófono INMP441 (I2S) en Raspberry Pi, incluyendo el caso del Compute
Module 5 (CM5) / Pi 5.

---

## 1. El MAX98357A y el I2S

El MAX98357A es un amplificador Class-D **digital**: no tiene entrada
analógica ni interfaz de control (no usa I2C ni SPI). Se configura
únicamente por pines hardwired y recibe el audio por I2S.

### 1.1 ¿Necesita I2S sí o sí?
**Sí.** El chip solo acepta audio digital serie por sus líneas
`BCLK` / `LRC`(WS) / `DIN`. No existe ruta analógica alternativa (ni el
jack ni el HDMI de la Pi lo pueden alimentar). Sus formatos aceptados son
I2S, Left-Justified y TDM, todos sobre el mismo bus de reloj+datos.

### 1.2 ¿Necesita I2C?
**No.** El I2C de este proyecto es exclusivo del OLED SSD1306
(SDA→GPIO2, SCL→GPIO3). El ampli no tiene registros que escribir.
Los pines `GAIN` y `SD_MODE` del MAX98357A son entradas de propósito
general (no un bus):
- `GAIN`: flotante = 3 dB (default), GND = 12 dB, VCC = 15 dB.
- `SD_MODE`: enable activo-alta. Con `no-sdmode` el driver no lo gestiona
  y el chip queda siempre habilitado por su pull-up.

### 1.3 Pines I2S (fijos, no remapeables)
El controlador PCM/I2S del BCM es de función alternativa fija en silicio;

| Señal I2S    | GPIO Pi | Función ALT0  |
|--------------|---------|---------------|
| BCLK / SCK   | GPIO18  | PCM_CLK       |
| LRC / WS     | GPIO19  | PCM_FS        |
| DIN (al amp) | GPIO21  | PCM_DOUT      |
| (entrada, p.ej. mic) | GPIO20 | PCM_DIN  |

No se pueden mover a otros GPIO sin emular I2S por software (bit-banging),
que el overlay del kernel no soporta. Por eso el `HARDWARE.md` coincide
exactamente con el estándar.

---

## 2. Problema encontrado: "no suena nada, tampoco el OLED"

### 2.1 Causa raíz (software)
`SSD1306` declaraba `uint8_t* buffer = nullptr` pero nunca lo asignaba.
`OLEDclearBuffer()` hacía `memset(this->buffer, 0, ...)` sobre `nullptr`
→ **segfault en `initOLED()`**, que corre *antes* del audio. El programa
moría antes de `playTestTone()`, por eso no sonaba nada ni el OLED.

Corregido en `src/oled/SSD1306_OLED.cpp`: se reserva/libera el buffer en
el constructor/destructor. Además `initOLED()` ahora dibuja
"MAX98357A / App v" para verificar visualmente el OLED.

### 2.2 Causa raíz (hardware/ALSA): no existía la tarjeta MAX98357A
La App abría `"default"`, que por PulseAudio apunta al **jack bcm2835**,
no al ampli. Y además el overlay del DAC no estaba cargado:

- `aplay -l` solo mostraba: bcm2835 Headphones (0), vc4-hdmi (1/3) e
  **inmp441-bare** (2, micrófono de entrada I2S).
- No había ninguna tarjeta MAX98357A.

Al cargar dinámicamente `sudo dtoverlay max98357a` daba
`Failed to apply overlay` porque el bus I2S ya estaba ocupado por
`inmp441-bare`.

### 2.3 Por qué el micrófono y el DAC entraban en conflicto
Ambos son I2S y comparten obligatoriamente los pines de reloj
(GPIO18/19). El Pi (modelo con header de 40 pines) tiene **un solo
controlador I2S**; dos overlays (`inmp441-bare` y `max98357a`) crean cada
uno su propia sound card y se disputan el mismo controlador → solo uno
puede enlazarse.

**Decisión:** el micrófono queda desactivado por ahora (ver sección 5 para
adaptarlo más adelante).

### 2.4 Solución aplicada
En `/boot/firmware/config.txt`:

```
#dtoverlay=inmp441-bare   # desactivado: conflicto I2S con max98357a
dtoverlay=max98357a,no-sdmode
```

(Requiere `dtparam=i2s=on`, que ya estaba presente.) Tras `sudo reboot`,
`aplay -l` muestra:

```
card 3: MAX98357A [MAX98357A], device 0: bcm2835-i2s-HiFi HiFi-0
```

### 2.5 La App ahora elige el MAX98357A automáticamente
`src/sound/max98357a.cpp` expone `max98357aDevice()` que escanea ALSA y
devuelve `plughw:CARD=<n>,0` si encuentra una tarjeta cuyo nombre
contenga "max98357a"; si no, cae a `"default"`. Se usa en
`Max98357A::init` y en `MPG123Decoder::initAlsa`. Así el audio va al ampli
y no al jack.

### 2.6 Verificación
- Tono directo al dispositivo:
  `speaker-test -D plughw:CARD=MAX98357A -c2 -r44100 -t sine -l 1`
- End-to-end: `sudo ./bin/App` → `[OLED] SSD1306 inicializado`,
  `[Audio] Tono de prueba reproducido` y reproducción de los MP3 de `mp3/`.

---

## 3. El bus I2S en Raspberry Pi (modelos con header de 40 pines)

- **Pi 3B/3B+/4B/5/Zero 2 W**: **un solo** controlador I2S/PCM
  (`i2s@7e203000`) en GPIO18/19/20/21. Un solo bus I2S a la vez.
- **Compute Module 4 (CM4)**: expone un **segundo I2S** (`i2s2`) vía
  overlay `dtoverlay=i2s2` en otros pines (GPIO28‑31 en el carrier CM4
  IO). Allí sí pueden coexistir mic y DAC en buses distintos.

---

## 4. I2S en Compute Module 5 (CM5) y Pi 5 (RP1 / BCM2712)

El CM5 usa el chip de E/S **RP1**. Según el datasheet CM5 (RP‑008180),
el white‑paper "Using the I2S peripherals on Raspberry Pi SBCs"
(RP‑009699) y los foros oficiales de Raspberry Pi:

- El RP1 tiene **tres** instancias I2S en silicio: **I2S0, I2S1, I2S2**.
- **I2S0** = clock-producer (master), **I2S1** = clock-consumer (slave).
  Ambos **comparten los mismos pines** (GPIO18‑21 + lanes 22‑27). Es un
  mux, no dos buses: no se usan simultáneamente.
- **I2S2** es independiente, pero está mapeado a pines del bank 1/2
  (GPIO28‑33) que **no se exponen externamente en el CM5** (reservados).
  Respuesta oficial del foro: *"I2S2 is independent from I2S0/1. But for
  Pi 5/CM5, it is not accessible as it is only available on pins in bank
  1 or 2."*
- El datasheet del CM5 lista: *"One I2S Master interface (ISC0),
  quadruple lane"* + *"One I2S, Slave interface (ISC1), quadruple lane"*
  → **un solo bus I2S real** en los GPIO del usuario.

**Conclusión CM5:** no hay un segundo bus I2S limpio (como sí en CM4).

### 4.1 Lo bueno: el bus es full-duplex con muchos lanes
El único bus I2S del CM5/Pi 5 soporta **hasta 4 lanes TX y 4 lanes RX**
(`SDO[0..3]` / `SDI[0..3]` en GPIO20‑27). Es decir, en **un mismo bus**
puedes tener a la vez:
- **Playback → MAX98357A** en `SDO` (GPIO21 = DIN del amp)
- **Capture → INMP441** en `SDI` (GPIO20 = DOUT del mic)

compartiendo `BCLK` (GPIO18) y `WS` (GPIO19). El bus queda full-duplex.

---

## 5. Cómo trabajar con AMBOS dispositivos (mic + DAC)

El conflicto actual no es de hardware: es que `inmp441-bare` y
`max98357a` crean **dos sound cards separadas** que se pelean por el mismo
controlador I2S. Para usar micrófono y amplificador **simultáneamente**
en el mismo bus I2S hay dos caminos:

### 5.1 (Recomendado en CM5/Pi 5) Overlay full-duplex combinado
Crear **un solo** device-tree overlay con una `simple-audio-card` que
defina **dos DAI links** sobre el mismo `i2s0`:
- DAI capture → INMP441 (SDI, GPIO20)
- DAI playback → MAX98357A (SDO, GPIO21)

Ambos comparten BCLK/LRC. Esto evita el conflicto de "dos tarjetas sobre
el mismo bus" y deja el I2S full-duplex. Requiere escribir/compilar un
`.dtbo` propio (no existe un overlay estándar que combine estos dos
chips).

Consideraciones al armar el overlay combinado:
- El MAX98357A es clock-consumer (esclavo): el bus debe ser master
  (I2S0, clock-producer).
- INMP441 también es esclavo, así que ambos conviven bien como esclavos
  bajo el mismo maestro I2S0.
- Formato I2S estándar, 44100 Hz (todos los I2S del RP1 comparten un
  mismo reloj `RP1_CLK_I2S`, por lo que mic y DAC deben usar la misma
  frecuencia de muestreo).

### 5.2 (Si se usara CM4) Segundo bus I2S físico
En CM4 se habilita `dtoverlay=i2s2` y se asigna el mic a I2S2 y el DAC a
I2S0, cada uno en su bus. No aplica en CM5.

### 5.3 (Alternativa simple) Mic por USB
Mover el INMP441 a una tarjeta de audio USB y dejar el I2S solo para el
MAX98357A. Sin conflictos y sin overlay personalizado.

---

## 6. Resumen de comandos útiles

```bash
# Ver tarjetas de reproducción
aplay -l
cat /proc/asound/cards

# Ver overlays I2S disponibles
ls /boot/overlays/ | grep -iE 'max98357a|inmp441|i2s'

# Cargar/quitar overlay en caliente (solo pruebas; para persistir, config.txt)
sudo dtoverlay max98357a,no-sdmode
sudo dtoverlay -r inmp441-bare

# Verificar sonido en el ampli
speaker-test -D plughw:CARD=MAX98357A -c2 -r44100 -t sine -l 1

# Configuración persistente en /boot/firmware/config.txt
dtparam=i2s=on
#dtoverlay=inmp441-bare
dtoverlay=max98357a,no-sdmode
```

---

## 7. Referencias
- Datasheet Compute Module 5 — Raspberry Pi (RP‑008180).
- White‑paper "Using the I2S peripherals on Raspberry Pi SBCs" — Raspberry Pi (RP‑009699).
- Foros oficiales Raspberry Pi: hilos t=384249 (I2S2 no accesible en Pi5/CM5), t=394118 (I2S en CM5), t=375661 (redefinir pines I2S en Pi5).
- `docs/HARDWARE.md`, `docs/ARCHITECTURE.md` de este proyecto.
