#ifndef MENU_H
#define MENU_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glut.h>
#include <stdio.h>

/* ── State Game ── */
typedef enum
{
    MENU,
    PLAYING,
    DEAD
} State;

// Deklarasi variabel global
extern State gState; 
extern int gScore;
extern int WIN_W;
extern int WIN_H;

// Fungsi Helper UI 
void begin2D();
void end2D();
void fillRect(float x, float y, float w, float h);
int  strWidth(void* font, const char* s);

// Fungsi Utama Layar Menu & Game Over 
void initStars();       
void drawStars();       
void drawMenuBg();     
void drawDeadBg();     

void drawMenu();       
void drawDead();       

#endif