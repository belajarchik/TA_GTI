#include <GL/glut.h>  
#include "menu.h"
#include <stdio.h>

extern int WIN_W;
extern int WIN_H;
extern int gScore;


State gState = MENU; 

// Menghitung lebar string untuk posisi tengah 
int strWidth(void* font, const char* s)
{
    int w = 0;
    while (*s) { 
        w += glutBitmapWidth(font, *s); 
        s++; 
    }
    return w;
}

// Fungsi menggambar kotak (digunakan untuk background)
void fillRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x,   y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y+h);
    glVertex2f(x,   y+h);
    glEnd();
}

// Layar Menu Awal 
/* -- Layar Menu Awal dengan Kotak Panel Modis ── */
void drawMenu()
{
    float cx = WIN_W * 0.5f;
    float cy = WIN_H * 0.5f;
    const char* p; 

    void* fontJudul = GLUT_BITMAP_TIMES_ROMAN_24; 
    void* fontBiasa = GLUT_BITMAP_9_BY_15;             

    begin2D();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Kotak Menu Wadah Utama
    float boxW = 460.0f; // Lebar kotak menu
    float boxH = 260.0f; // Tinggi kotak menu
    float boxX = cx - (boxW * 0.5f);
    float boxY = cy - (boxH * 0.5f) + 10;

    // 1. Isi Kotak (Hitam pekat transparan)
    glColor4f(0.02f, 0.02f, 0.05f, 0.85f); 
    fillRect(boxX, boxY, boxW, boxH);

    // 2. Garis Tepi Kotak / Border (Warna Biru Cyan Neon)
    glColor3f(0.3f, 0.8f, 1.0f); 
    glLineWidth(3.0f); // Menebalkan garis kotak
    glBegin(GL_LINE_LOOP);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxW, boxY);
        glVertex2f(boxX + boxW, boxY + boxH);
        glVertex2f(boxX, boxY + boxH);
    glEnd();
    glLineWidth(1.0f); // Reset ketebalan garis

    // Judul Game
    const char* title = "VOID RUNNER 3D";
    float tw = (float)strWidth(fontJudul, title);
    float titleY = boxY + boxH - 45;

    glColor3f(0.0f, 0.3f, 0.5f); // Shadow
    glRasterPos2f(cx - tw * 0.5f + 2, titleY - 2);
    for (p = title; *p; p++) { glutBitmapCharacter(fontJudul, *p); }

    glColor3f(0.3f, 0.8f, 1.0f); // Teks Utama
    glRasterPos2f(cx - tw * 0.5f, titleY);
    for (p = title; *p; p++) { glutBitmapCharacter(fontJudul, *p); }

    // Petunjuk Kontrol (Ditata rapi ke bawah)
    const char* baris1 = "Gunakan Arrow Kiri/Kanan atau A/D";
    const char* baris2 = "untuk berpindah lane.";
    const char* baris3 = "Nabrak rintangan = langsung MATI!";
    
    float b1w = (float)strWidth(fontBiasa, baris1);
    glColor3f(0.9f, 0.9f, 0.9f);
    glRasterPos2f(cx - b1w * 0.5f, boxY + boxH - 100);
    for (p = baris1; *p; p++) { glutBitmapCharacter(fontBiasa, *p); }

    float b2w = (float)strWidth(fontBiasa, baris2);
    glRasterPos2f(cx - b2w * 0.5f, boxY + boxH - 125);
    for (p = baris2; *p; p++) { glutBitmapCharacter(fontBiasa, *p); }

    float b3w = (float)strWidth(fontBiasa, baris3);
    glColor3f(1.0f, 0.4f, 0.4f);
    glRasterPos2f(cx - b3w * 0.5f, boxY + boxH - 165);
    for (p = baris3; *p; p++) { glutBitmapCharacter(fontBiasa, *p); }

    // Tombol Start
    const char* txtStart = "Tekan SPASI / ENTER untuk mulai";
    float sw = (float)strWidth(fontBiasa, txtStart);
    
    glColor3f(0.1f, 0.1f, 0.1f); // Shadow
    glRasterPos2f(cx - sw * 0.5f + 1, boxY + 34);
    for (p = txtStart; *p; p++) { glutBitmapCharacter(fontBiasa, *p); }

    glColor3f(0.3f, 1.0f, 0.5f); // Teks Utama
    glRasterPos2f(cx - sw * 0.5f, boxY + 35);
    for (p = txtStart; *p; p++) { glutBitmapCharacter(fontBiasa, *p); }

    glDisable(GL_BLEND);
    end2D();
}

// Layar Game Over 
void drawDead()
{
    float cx = WIN_W * 0.5f;
    float cy = WIN_H * 0.5f;
    const char* p; // Variabel penunjuk untuk looping cetak karakter

    void* fontJudul = GLUT_BITMAP_TIMES_ROMAN_24;
    void* font = GLUT_BITMAP_9_BY_15;

    begin2D();

    // Background Hitam Transparan Lembut Overlay Layar Game 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.70f);
    fillRect(0, 0, (float)WIN_W, (float)WIN_H);

    // Judul
    const char* go = "GAME OVER";
    float tw = (float)strWidth(fontJudul, go);
    float titleY = cy + 60;

    // Efek Shadow 
    glColor3f(0.40f, 0.0f, 0.0f);
    glRasterPos2f(cx - tw * 0.5f + 2, titleY - 2);
    for (p = go; *p; p++) {
        glutBitmapCharacter(font, *p);
    }

    // Teks Utama 
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(cx - tw * 0.5f, titleY);
    for (p = go; *p; p++) {
        glutBitmapCharacter(fontJudul, *p);
    }


    // Skor Akhir 
    char scoreBuf[32];
    sprintf(scoreBuf, "SCORE : %d", gScore);
    float sw = (float)strWidth(font, scoreBuf);
    
    glColor3f(0.7f, 0.7f, 0.7f); // Warna abu-abu 
    glRasterPos2f(cx - sw * 0.5f, cy + 20);
    for (p = scoreBuf; *p; p++) {
        glutBitmapCharacter(font, *p);
    }

    // Menu Navigasi "RETRY" & "EXIT" 
    const char* txtRetry = "RETRY  ( R / SPASI )";
    const char* txtExit  = "EXIT  ( ESC )";
    
    float rw = (float)strWidth(font, txtRetry);
    float ew = (float)strWidth(font, txtExit);

    // Baris Retry
    float retryY = cy - 30;
    // Shadow teks RETRY
    glColor3f(0.1f, 0.1f, 0.1f);
    glRasterPos2f(cx - rw * 0.5f + 1, retryY - 1);
    for (p = txtRetry; *p; p++) {
        glutBitmapCharacter(font, *p);
    }
    // Teks utama RETRY 
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(cx - rw * 0.5f, retryY);
    for (p = txtRetry; *p; p++) {
        glutBitmapCharacter(font, *p);
    }

    // Baris Exit
    float exitY = cy - 70;
    // Shadow teks EXIT
    glColor3f(0.1f, 0.1f, 0.1f);
    glRasterPos2f(cx - ew * 0.5f + 1, exitY - 1);
    for (p = txtExit; *p; p++) {
        glutBitmapCharacter(font, *p);
    }
    // Teks utama EXIT 
    glColor3f(0.8f, 0.3f, 0.3f);
    glRasterPos2f(cx - ew * 0.5f, exitY);
    for (p = txtExit; *p; p++) {
        glutBitmapCharacter(font, *p);
    }

    glDisable(GL_BLEND);
    end2D();
}
