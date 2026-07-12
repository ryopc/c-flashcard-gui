# Installation Guide

## Linux (Ubuntu/Debian)

### Option 1: PPA（推奨）

```bash
# PPA を追加
sudo add-apt-repository ppa:game_ryo/flashcard

# パッケージリストを更新
sudo apt-get update

# インストール
sudo apt-get install flashcard

# 実行
flashcard
```

### Option 2: ソースからビルド

**依存ライブラリをインストール:**

```bash
# Ubuntu/Debian
sudo apt-get install -y build-essential libraylib-dev pkg-config

# Fedora/RHEL
sudo dnf install -y gcc raylib-devel

# Arch
sudo pacman -S base-devel raylib
```

**ビルド:**

```bash
git clone https://github.com/ryopc/c-flashcard-gui.git
cd c-flashcard-gui
make

# 実行
./bin/flashcard

# （オプション）システムワイドインストール
sudo install -D -m 0755 bin/flashcard /usr/local/bin/flashcard
```

## Windows

### Cross-compile from Linux

**MinGW-w64 をインストール:**

```bash
sudo apt-get install -y mingw-w64
```

**Raylib for Windows をビルド:**

```bash
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP CC=x86_64-w64-mingw32-gcc -j$(nproc)

mkdir -p /opt/raylib-mingw64/{lib,include}
cp raylib.a /opt/raylib-mingw64/lib/
cp *.h /opt/raylib-mingw64/include/
```

**クロスコンパイル:**

```bash
cd ../c-flashcard-gui
export RAYLIB_WINDOWS_PATH=/opt/raylib-mingw64
make windows

# 出力: bin/flashcard.exe
```

## macOS

**Homebrew でインストール（将来対応予定）:**

```bash
brew install flashcard
```

またはソースからビルド：

```bash
brew install raylib pkg-config
git clone https://github.com/ryopc/c-flashcard-gui.git
cd c-flashcard-gui
make
./bin/flashcard
```
