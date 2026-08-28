# Learnings

Aprendizajes reales del proyecto **max98357a** (Raspberry Pi + MAX98357A I2S +
OLED SSD1306). La sección de Versionado se adaptó del borrador genérico original;
el resto reemplaza el contenido ajeno (Flutter/Velocity) por lo verificado aquí.

---

## Versionado

Versionado semántico en el archivo `VERSION` (`MAJOR.MINOR.PATCH`).

- **Cada push incrementa la versión en `+0.0.1`** (se suma 1 al PATCH).
- **Ciclo de parche obligatorio 0-9:** tras `x.y.9` el siguiente es `x.(y+1).0`.
  Nunca `x.y.11` ni saltos. Ej.: `0.1.9` -> `0.2.0`.
- El archivo `VERSION` (sin prefijo `v`) debe coincidir siempre con el último tag.
- **Cada push lleva su tag anotado `v<VERSION>`** (ej. `v0.1.1`). No se pushea sin tag.
- Reglas:
  - Conventional commits: `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`, `test:`.
  - No eliminar tags publicados; si hay error, se crea un tag nuevo con el siguiente número.
  - No retroceder de versión.
  - No saltar números de versión.
- El proyecto continúa desde su `VERSION` actual (`0.1.0`); el primer tag de esta
  regla es `v0.1.0` y el siguiente push será `v0.1.1`. (Se adapta el "empieza en
  1.0.0" del borrador original a la versión real del proyecto.)

### Flujo por push
1. Obtener el último tag publicado (ej. `v0.1.0`).
2. `VERSION` debe coincidir con ese tag (sin `v`).
3. Calcular siguiente versión: `tag + 0.0.1` respetando el ciclo 0-9.
4. Actualizar `VERSION`.
5. Commit + push + crear tag `v<VERSION>` + `git push origin --tags`.

---

## Hardware / Audio

- `bcm2835` requiere root para GPIO/I2C/SPI; el programa corre con `sudo` en la Pi.
- El MAX98357A es un amplificador I2S digital: **sin entrada analógica ni registro
  de volumen** (no usa I2C). El volumen se atenúa en software (`setVolume` escala
  las muestras PCM antes de enviarlas).
- MAX98357A necesita I2S sí o sí: BCLK=GPIO18, LRC=GPIO19, DIN=GPIO21. Son pines
  fijos del periférico PCM (ALT0) y **no se remapean** a otros GPIO.
- `GAIN` y `SD_MODE` son entradas de propósito general (no un bus). Con
  `no-sdmode` el driver no gestiona `SD_MODE` (chip siempre habilitado por pull-up).
- Para que aparezca la tarjeta MAX98357A hay que cargar
  `dtoverlay=max98357a,no-sdmode` en `/boot/firmware/config.txt` y reiniciar.
  El dispositivo `"default"` de ALSA va al jack bcm2835 (vía PulseAudio), no al ampli.
- En Pi con header de 40 pines hay **un solo bus I2S**. Dos overlays I2S
  (p.ej. `inmp441-bare` + `max98357a`) entran en conflicto porque comparten los
  pines de reloj. El micrófono queda desactivado por ahora.
- CM5 / Pi5 (RP1): I2S0/I2S1 comparten pines (un bus); I2S2 existe en silicio pero
  no está expuesto. El bus es **full-duplex** (hasta 4 lanes TX + 4 RX), por lo que
  mic y DAC pueden coexistir en el mismo bus con un overlay combinado (una sola
  sound card con DAI capture + playback).

---

## Software

- El buffer del `SSD1306` debe asignarse en el constructor; si es `nullptr`,
  `memset` en `OLEDclearBuffer` causa segfault que mata el programa antes del audio.
- ALSA `snd_pcm_writei` puede devolver escrituras cortas; hay que iterar hasta
  enviar todos los frames.
- Siempre verificar los valores de retorno de las funciones de init de hardware.
- Compilación y ejecución son **remotas** (en la Pi), no en la máquina de desarrollo.

---

## Git / Push (adaptado a este proyecto)

- Push vía SSH: `git@github.com:siliconvalleyar-oss/max98357a_rpi.git`.
  No usar HTTPS con token en la URL (el remote HTTPS pedía autenticación).
- Cada push lleva su tag (ver sección Versionado).
