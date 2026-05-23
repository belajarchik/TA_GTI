#ifndef MENU_H
#define MENU_H

#ifdef _WIN32
#include <windows.h>
#endif

// #include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdio.h>

/* ── State Game ── */
typedef enum
{
    MENU,
    PLAYING,
    DEAD,
    WIN_ANIM,
        WIN
} State;

// Deklarasi variabel global
extern State gState;
extern int gScore;
extern int WIN_W;
extern int WIN_H;

// Fungsi Helper UI
extern void drawText(float x, float y, const char *str);
void begin2D();
void end2D();
void fillRect(float x, float y, float w, float h);
int strWidth(void *font, const char *s);

// Fungsi Utama Layar Menu & Game Over
void initStars();
void drawStars();
void drawMenuBg();
void drawDeadBg();

void drawMenu();
void drawDead();
void drawWin(int score, float seconds);

#endif