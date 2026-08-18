#!/bin/bash

# install_deps.sh - Instala dependencias para el proyecto max98357a_rpi
# Compatible con Raspberry Pi OS 32 y 64 bits

set -e

echo "=== Instalando dependencias para max98357a_rpi ==="

# Detectar arquitectura
ARCH=$(uname -m)
echo "Arquitectura detectada: $ARCH"

# Actualizar paquetes
echo "[1/5] Actualizando lista de paquetes..."
apt-get update -qq

# Dependencias basicas
echo "[2/5] Instalando dependencias basicas..."
apt-get install -y -qq build-essential git i2c-tools

# bcm2835
echo "[3/5] Instalando libreria bcm2835..."
if ! ldconfig -p | grep -q libbcm2835; then
    apt-get install -y -qq libbcm2835-dev || {
        echo "Instalando bcm2835 desde fuente..."
        TMPDIR=$(mktemp -d)
        cd "$TMPDIR"
        wget -q http://www.airspayce.com/mikem/bcm2835/bcm2835-1.71.tar.gz
        tar xzf bcm2835-1.71.tar.gz
        cd bcm2835-1.71
        ./configure
        make
        make check
        make install
        cd /
        rm -rf "$TMPDIR"
    }
else
    echo "bcm2835 ya instalada."
fi

# ALSA
echo "[4/5] Instalando ALSA..."
apt-get install -y -qq libasound2-dev alsa-utils

# Configurar I2C
echo "[5/5] Habilitando I2C..."
if ! grep -q "i2c-bcm2835" /etc/modules 2>/dev/null; then
    echo "i2c-bcm2835" >> /etc/modules
fi

echo ""
echo "=== Dependencias instaladas correctamente ==="
echo "Para compilar: make"
echo "Para ejecutar: sudo ./bin/App"
