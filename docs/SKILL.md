# SKILL.md — Conocimiento del proyecto max98357a

Documentación viva de todo lo aprendido trabajando en este proyecto
(Raspberry Pi + amplificador **MAX98357A** I2S + OLED **SSD1306**). Está pensado
como "skill" de referencia para cualquier sesión futura.

> La política estricta de versionado/tags vive en `docs/LEARNINGS.md`. Aquí se
> resume y el resto es conocimiento técnico.

---

## 1. Arquitectura rápida

- `main.cpp` → crea `Device::Device_t` y llama `run()`.
- `Device_t` (`src/engine/Device_t.cpp`) orquesta:
  `initBcm2835 → initOLED → displayInfo → playTestTone → playMp3Folder → delay → cleanup`.
- Audio: `Sound::Max98357A` (tono) y `Sound::MP3Player` → `MPG123Decoder` (MP3 vía libmpg123 + ALSA).
- OLED: driver `SSD1306` de terceros (bcm2835 I2C, addr 0x3C).
- El MAX98357A se controla **solo por I2S**; el OLED **solo por I2C**. No comparten bus.

---

## 2. MAX98357A (amplificador I2S)

- Es digital Class-D: **no tiene entrada analógica** ni registro de configuración.
  No usa I2C ni SPI. Se configura por pines hardwired.
- **Necesita I2S sí o sí** (no hay ruta analógica alternativa).
- Pines I2S (fijos, no remapeables — función ALT0 del periférico PCM):
  - BCLK → GPIO18
  - LRC/WS → GPIO19
  - DIN (al amp) → GPIO21 (PCM_DOUT)
  - (entrada, p.ej. mic) → GPIO20 (PCM_DIN)
- `GAIN`: flotante ≈ 3 dB (default), GND = 12 dB, VCC = 15 dB. Pin de propósito general.
- `SD_MODE`: enable activo-alta. Con `no-sdmode` el driver no lo gestiona y el chip
  queda siempre habilitado por su pull-up en la placa.
- **No tiene control de volumen por hardware.** El volumen se baja atenuando las
  muestras PCM en software (`setVolume` escala `int16` antes de `snd_pcm_writei`).

---

## 3. OLED SSD1306

- I2C, dirección 0x3C. Requiere `bcm2835` con root.
- **Bug crítico corregido:** `SSD1306` declaraba `uint8_t* buffer = nullptr` y no lo
  asignaba. `OLEDclearBuffer()` hacía `memset(nullptr,...)` → segfault en `initOLED()`,
  que corre **antes** del audio, matando el programa (por eso "no sonaba nada ni el OLED").
  Ahora el buffer se reserva en el constructor y se libera en el destructor.
- `initOLED()` redirige `stderr` a `/dev/null` para silenciar warnings del driver y
  dibuja texto ("MAX98357A / App v") para verificar visualmente.

---

## 4. I2S en Raspberry Pi y el conflicto mic/DAC

- La App abría `"default"`, que por PulseAudio apunta al **jack bcm2835**, no al ampli.
  Por eso aunque hubiera sonado, no iba al MAX98357A.
- Para que exista la tarjeta MAX98357A hay que cargar el overlay en `/boot/firmware/config.txt`:
  ```
  dtparam=i2s=on
  #dtoverlay=inmp441-bare          # mic desactivado: conflicto I2S con max98357a
  dtoverlay=max98357a,no-sdmode
  ```
  y reiniciar. Tras el reboot aparece `card N: MAX98357A [MAX98357A]`.
- **Conflicto:** el micrófono INMP441 (I2S, entrada) y el DAC MAX98357A (I2S, salida)
  comparten los pines de reloj (GPIO18/19). En Pi de 40 pines hay **un solo bus I2S**,
  así que dos overlays I2S se pelean. Decisión: micrófono desactivado por ahora.
- La App ahora auto-detecta la tarjeta MAX98357A con `max98357aDevice()` (escanea ALSA
  y usa `plughw:CARD=<n>,0`; fallback `"default"`), en `Max98357A` y `MPG123Decoder`.

### 4.1 CM5 / Pi5 (chip RP1 / BCM2712)
- El RP1 tiene 3 instancias I2S (I2S0, I2S1, I2S2), pero:
  - I2S0 (master) e I2S1 (slave) **comparten los mismos pines** GPIO18-21 → es un mux, no dos buses.
  - I2S2 es independiente pero **no está expuesto** externamente en CM5/Pi5 (reservado).
- El datasheet CM5 lista "One I2S Master + One I2S Slave" → **un solo bus I2S real**.
- Lo bueno: ese bus es **full-duplex con hasta 4 lanes TX y 4 RX** (SDO[0..3]/SDI[0..3]
  en GPIO20-27). Por tanto mic (INMP441 en SDI/GPIO20) y DAC (MAX98357A en SDO/GPIO21)
  **pueden coexistir en el mismo bus** con un overlay combinado: una sola `simple-audio-card`
  con dos DAI links (capture + playback) sobre `i2s0`. Así se evita el conflicto de
  "dos tarjetas sobre el mismo controlador".
- Alternativas: (a) overlay combinado full-duplex; (b) en CM4 usar `dtoverlay=i2s2`
  (segundo bus físico, no aplica en CM5); (c) mic por USB.

---

## 5. Volumen (atenciación digital)

- `Max98357A::setVolume(float)`, `MPG123Decoder::setVolume(float)`, expuesto vía
  `MP3Player::setVolume` y `Device_t::setVolume`.
- `Device_t` aplica `kVolume = 0.4f` por defecto; se puede sobreescribir por CLI:
  `./bin/App --volume 0.3` o `--volume=0.6` (rango 0.0–1.0).
- Implementación: en `play()`/`decodeAndPlay()` se escala cada muestra `int16` por el factor.

---

## 6. Compilación y ejecución (REMOTA)

- **No correr en la máquina de desarrollo**: `bcm2835` necesita GPIO/I2C reales y root.
  `make run` local falla con "Cannot start I2C, Running root?".
- Compilar y ejecutar en la Pi vía SSH:
  ```bash
  ssh joy@raspberry.local "cd /home/joy/src/max98357a_rpi && git pull && make clean && make -j4"
  ssh joy@raspberry.local "cd /home/joy/src/max98357a_rpi && sudo ./bin/App --volume 0.4"
  ```
- El binario funcional es el de la Pi (`bin/App` compilado allí), no el que genere `make` en x86.

---

## 7. Git, versionado y tags

- Remote en SSH: `git@github.com:siliconvalleyar-oss/max98357a_rpi.git` (el HTTPS fallaba por auth).
- Regla (detalle en `docs/LEARNINGS.md`): **cada push incrementa `VERSION` en `+0.0.1`**
  (ciclo de parche 0-9: tras `x.y.9` → `x.(y+1).0`), y lleva su tag anotado `v<VERSION>`.
  `VERSION` (sin `v`) debe coincidir con el último tag. Conventional commits.
- Tags creados con `git tag -a v<VERSION> -m "..."` y subidos con `git push origin --tags`.
- No eliminar tags publicados; no retroceder versión; no saltar números.

### Estados de la sesión (histórico de tags)
- `v0.1.0-<sha>` ×6: pushes iniciales de la sesión (antes de adoptar el esquema estricto).
- `v0.1.1` en adelante: bajo la regla de `+0.0.1` por push (este commit).

---

## 8. Comandos de verificación en la Pi

```bash
aplay -l                      # listar tarjetas de reproducción
cat /proc/asound/cards        # tarjetas actuales
sudo dtoverlay max98357a,no-sdmode   # cargar overlay en caliente (prueba)
sudo dtoverlay -r inmp441-bare       # quitar overlay en caliente
speaker-test -D plughw:CARD=MAX98357A -c2 -r44100 -t sine -l 1   # tono de prueba
sudo ./bin/App --version      # versión
```

---

## 9. Lecciones de código (ALSA)

- `snd_pcm_writei` puede devolver **escrituras cortas** (menos frames que los pedidos);
  hay que iterar `offset += written` hasta enviar todos los frames, o solo suena un blip.
- Ante `-EPIPE` (underrun) hacer `snd_pcm_prepare` y reintentar.
- Siempre verificar retornos de init (`snd_pcm_open`, `snd_pcm_hw_params_*`).

---

## 10. Roadmap pendiente

- Adaptar el micrófono INMP441 en el mismo bus I2S (overlay full-duplex combinado) o moverlo a USB.
- HMC5883L (magnetómetro) sigue como stub (`read()` vacío; genera warnings de parámetro sin usar).
- MP3: `playMp3Folder` avanza con `next()`; `loadDir` ordena alfabéticamente.
