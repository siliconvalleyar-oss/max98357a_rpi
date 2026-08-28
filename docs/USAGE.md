# Usage

## Run
```bash
sudo ./bin/App
```

## Show version
```bash
./bin/App --version
```

## Volume
El MAX98357A no tiene control de volumen por hardware (ganancia fija),
así que el volumen se ajusta atenuando las muestras digitales (0.0 a 1.0).
Por defecto es 0.4.

```bash
sudo ./bin/App --volume 0.3
sudo ./bin/App --volume=0.6
```
