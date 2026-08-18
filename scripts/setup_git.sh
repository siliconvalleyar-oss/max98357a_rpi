#!/bin/bash

# setup_git.sh - Configura el repositorio Git para max98357a_rpi

echo "=== Configuracion de Repositorio Git ==="
echo ""

read -p "Nombre de usuario (GitHub/GitLab): " USERNAME
read -p "Nombre del repositorio (default: max98357a_rpi): " REPO_NAME
read -p "Visibilidad (public/private, default: public): " VISIBILITY

REPO_NAME=${REPO_NAME:-max98357a_rpi}
VISIBILITY=${VISIBILITY:-public}

echo ""
echo "Configurando repositorio: $REPO_NAME"
echo "Usuario: $USERNAME"
echo "Visibilidad: $VISIBILITY"
echo ""

# Inicializar git si no existe
if [ ! -d ".git" ]; then
    echo "Inicializando repositorio Git..."
    git init
    git checkout -b main
fi

# Configurar usuario
git config user.name "$USERNAME"
read -p "Email para Git (default: $USERNAME@users.noreply.github.com): " EMAIL
EMAIL=${EMAIL:-$USERNAME@users.noreply.github.com}
git config user.email "$EMAIL"

# Agregar archivos
echo "Agregando archivos..."
git add .

# Commit inicial
if git diff --cached --quiet; then
    echo "No hay cambios para commitear."
else
    echo "Creando commit inicial..."
    git commit -m "Initial commit"
fi

# Verificar si gh esta instalado
if command -v gh &> /dev/null; then
    echo ""
    echo "GitHub CLI detectado. Creando repositorio remoto..."
    read -p "Token de GitHub (deja vacio si ya esta autenticado): " TOKEN

    if [ -n "$TOKEN" ]; then
        echo "$TOKEN" | gh auth login --with-token
    fi

    if gh auth status &> /dev/null; then
        gh repo create "$REPO_NAME" --"$VISIBILITY" --source=. --remote=origin --push
        echo "Repositorio creado y codigo subido correctamente."
    else
        echo "No autenticado en GitHub. Creando localmente..."
        echo "Ejecuta: gh auth login"
    fi
else
    echo ""
    echo "GitHub CLI (gh) no encontrado."
    echo "Para crear el repositorio manualmente:"
    echo "1. Ve a https://github.com/new"
    echo "2. Crea un repositorio llamado: $REPO_NAME"
    echo "3. Ejecuta estos comandos:"
    echo ""
    echo "   git remote add origin https://github.com/$USERNAME/$REPO_NAME.git"
    echo "   git branch -M main"
    echo "   git push -u origin main"
    echo ""
fi

echo ""
echo "=== Configuracion completada ==="
