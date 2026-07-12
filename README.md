# Flashcard GUI - Raylib Edition

A lightweight, cross-platform flashcard application built with C and Raylib.

## Features

- **Cross-platform**: Linux and Windows (via MinGW-w64 cross-compilation)
- **Lightweight**: Minimal dependencies, fast startup
- **Simple CSV format**: Easy to manage flashcard data
- **Clean UI**: Raylib-based interface with Catppuccin Mocha color scheme
- **Study modes**: Question-answer reveal, scoring system

## Project Structure

```
c-flashcard-gui/
├── src/
│   ├── main.c           # Main loop, UI rendering, event handling
│   ├── flashcard.c      # Flashcard logic (load, shuffle, state)
│   └── flashcard.h      # Header file
├── data/
│   └── cards.csv        # Sample flashcard data
├── Makefile             # Build automation (Linux + Windows cross-compile)
├── README.md            # This file
└── .gitignore
```

## Build Requirements

### Linux

**Ubuntu/Debian:**

```bash
sudo apt-get install -y build-essential libraylib-dev pkg-config
```

**Fedora/RHEL:**

```bash
sudo dnf install -y gcc raylib-devel
```

**Arch:**

```bash
sudo pacman -S base-devel raylib
```

### Windows (Cross-compile from Linux)

**Ubuntu/Debian:**

```bash
sudo apt-get install -y mingw-w64 mingw-w64-tools
```

You'll also need a pre-compiled Raylib for MinGW-w64. See [Building Raylib for Windows](#building-raylib-for-windows) below.

## Building

### Linux (Native)

```bash
make
# Or simply:
make linux
```

Output: `bin/flashcard`

### Windows (Cross-compile)

1. **Prepare Raylib for Windows:**
   - Download or build Raylib for MinGW-w64 (see instructions below)
   - Set the path:
     ```bash
     export RAYLIB_WINDOWS_PATH=/path/to/raylib/mingw-w64
     ```

2. **Compile:**
   ```bash
   make windows
   ```

Output: `bin/flashcard.exe`

## Building Raylib for Windows

If you don't have a pre-built Raylib for MinGW-w64, build it from source:

```bash
# Clone Raylib
git clone https://github.com/raysan5/raylib.git
cd raylib/src

# Build for Windows
make PLATFORM=PLATFORM_DESKTOP CC=x86_64-w64-mingw32-gcc -j$(nproc)

# Install to a custom path (optional)
mkdir -p /opt/raylib-mingw64
cp ../src/raylib.a /opt/raylib-mingw64/lib/
cp -r ../src/*.h /opt/raylib-mingw64/include/

# Then use in flashcard build:
export RAYLIB_WINDOWS_PATH=/opt/raylib-mingw64
make windows
```

## Data Format (CSV)

Flashcards are stored in CSV format: `question,answer`

**Example (`data/cards.csv`):**

```csv
ls -la,List files with hidden files and details
ps aux,Display running processes in detail
df -h,Show disk usage in human-readable format
```

### Special Characters

Newlines and commas are escaped:
- `\n` → newline
- `\t` → tab
- `\r` → carriage return
- `\\` → backslash
- `\,` → comma

**Example:**

```csv
What is a newline?,A control character\nused to end lines
Use comma\,carefully,Always escape commas in CSV
```

## Running

### Linux

```bash
./bin/flashcard
```

### Windows

Double-click `bin/flashcard.exe` or run from command prompt:

```cmd
bin\flashcard.exe
```

## Usage

1. **Study Mode:**
   - Click **Show Answer** to reveal the answer
   - Click **Correct** if you knew it, **Incorrect** if you didn't
   - Progress through all cards

2. **Results:**
   - View your score
   - Click **Try Again** to shuffle and restart

3. **Edit Cards:**
   - Modify `data/cards.csv` with your editor
   - Restart the application to load changes

## Troubleshooting

### "pkg-config: command not found" (Linux)

```bash
# Ubuntu/Debian
sudo apt-get install -y pkg-config

# Fedora
sudo dnf install -y pkgconfig
```

### Windows build fails: "raylib.h not found"

1. Ensure Raylib is built for MinGW-w64
2. Set `RAYLIB_WINDOWS_PATH` environment variable
3. Verify the path contains `include/raylib.h` and `lib/libraylib.a`

### "libraylib.so not found" (Linux runtime)

```bash
# Install raylib runtime
sudo apt-get install -y libraylib2
```

## Development Notes

### Memory Management

- All allocations use standard `malloc`/`free`
- CSV parser handles escape sequences
- Deck state is managed via `FlashcardDeck` struct

### Thread Safety

- No multithreading used; single-threaded event loop
- Safe for casual use

### Performance

- 60 FPS rendering (configurable in `main.c`)
- CSV loading is O(n)
- Shuffle is Fisher-Yates O(n)

## License

MIT License. See repo for details.

## References

- [Raylib Documentation](https://www.raylib.com/)
- [Raylib GitHub](https://github.com/raysan5/raylib)
- [Catppuccin Color Scheme](https://catppuccin.com/)
