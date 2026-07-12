#!/bin/bash
set -e

echo "========================================="
echo "   ryopc Flashcard App Installer"
echo "========================================="

# 1. Check and install prerequisites (flatpak, curl)
echo "[1/3] Checking system environment..."
if ! command -v flatpak &> /dev/null; then
    echo "Flatpak is not installed. Installing Flatpak (sudo password may be required)..."
    sudo apt update && sudo apt install -y flatpak
fi

if ! command -v curl &> /dev/null; then
    echo "Installing curl..."
    sudo apt update && sudo apt install -y curl
fi

# 2. Download the latest .flatpak bundle from GitHub Releases
echo "[2/3] Downloading the latest package from GitHub (ryopc/c-flashcard-gui)..."
TMP_DIR=$(mktemp -d)
curl -L -o "$TMP_DIR/Flashcard.flatpak" "https://github.com/ryopc/c-flashcard-gui/releases/latest/download/Flashcard.flatpak"

# 3. Install the application via Flatpak
echo "[3/3] Installing the application into your system..."
flatpak install --user "$TMP_DIR/Flashcard.flatpak" -y

# Clean up
rm -rf "$TMP_DIR"

echo "========================================="
echo "🎉 Installation completed successfully!"
echo "========================================="
echo "You can launch the app by running:"
echo "  flatpak run com.ryopc.Flashcard"
echo ""
echo "Alternatively, you can find 'Flashcard' with its custom icon in your desktop application launcher!"
