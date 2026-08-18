# max98357a_rpi

Proyecto C++ para Raspberry Pi que controla un amplificador de audio MAX98357A y una pantalla OLED SSD1306.

## Requisitos
- Raspberry Pi (32 o 64 bits)
- bcm2835
- ALSA (libasound2-dev)
- Git

## Compilacion
```bash
make
```

## Instalacion de dependencias
```bash
chmod +x scripts/install_deps.sh
./scripts/install_deps.sh
```

## Uso
```bash
sudo ./bin/App
```

## Version
La version se muestra al inicio y con `--version`:
```bash
./bin/App --version
```

## Repositorio Git
Para configurar el repositorio:
```bash
chmod +x scripts/setup_git.sh
./scripts/setup_git.sh
```

## Compatibilidad
Compatible con Raspberry Pi de 32 bits y 64 bits.
