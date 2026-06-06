cmake_minimum_required(VERSION 3.10)
project(Hypernova_Escape)

# Standar C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Tambahkan direktori include agar header bisa diakses dari mana saja
include_directories(include)

# 2. Ambil semua file .cpp dari folder src secara otomatis
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 3. Pengaturan dependensi library grafis (Cross-Platform)
if(APPLE)
    find_library(GLUT_FRAMEWORK GLUT)
    find_library(OPENGL_FRAMEWORK OpenGL)
    set(GRAPHICS_LIBS ${GLUT_FRAMEWORK} ${OPENGL_FRAMEWORK})
elseif(WIN32)
    # Mencari library di Windows
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    set(GRAPHICS_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
endif()

# 4. Buat executable
add_executable(Hypernova_Escape ${SOURCES})

# 5. Link library ke executable
target_link_libraries(Hypernova_Escape ${GRAPHICS_LIBS})

# Otomatis menyalin folder assets ke direktori build
add_custom_command(TARGET Hypernova_Escape POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:Hypernova_Escape>/assets"
)