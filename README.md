# TA_GTI: Hypernova Escape

Proyek ini adalah game *endless runner* yang dikembangkan dengan C++ dan OpenGL. Proyek ini mendukung lintas platform (macOS dan Windows) menggunakan sistem *build* **CMake**.

## Prasyarat
Sebelum memulai, pastikan sistem Anda sudah memiliki:
1. **CMake** (minimal versi 3.10)
2. **Compiler C++** (GCC/Clang untuk macOS, MinGW untuk Windows)
3. **Library Grafis**:
   - **macOS**: `freeglut` (install via Homebrew: `brew install freeglut`)
   - **Windows**: `freeglut` (pastikan path `include` dan `lib` terdeteksi oleh compiler Anda)

## Cara Build & Run

### 1. Clone Repositori
```bash
git clone [https://github.com/belajarchik/TA_GTI](https://github.com/belajarchik/TA_GTI)
cd TA_GTI

### 2. Konfigurasi dan kompilasi  proyek
mkdir build
cd build
cmake ..
make

### 3. Menjalankan (run) 
# MacOs
./Hypernova_Escape

# Windows
Hypernova_Escape.exe