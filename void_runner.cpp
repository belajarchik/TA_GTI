/*
    Kontrol:
      Arrow Left / A  : Pindah kiri
      Arrow Right / D : Pindah kanan
      SPACE / ENTER   : Mulai / Restart
      ESC             : Keluar
*/

#ifdef _WIN32
#include <windows.h>
#endif

//#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "roket.h"
#include "imageloader.h"
#include "obstacle.h"
#include "menu.h"

using namespace std;

/* PAKSA DECLARE */
extern "C"
{
    int rand(void);
    void srand(unsigned int seed);
}

/* -- Konstanta --------------------------- */
int WIN_W = 800;
int WIN_H = 600;

float LANES[3] = {-3.0f, 0.0f, 3.0f};

// State game
int gScore = 0;
int gFrame = 0;
float gSpeed = 5.0f;
Obs gObs[MAX_OBS];

int gLane = 1;
float gPlayerX = 0.0f;

GLuint textureID; 

//  Memasukan gambar ke program
void initTexture() {
    // Membaca file gambar menggunakan fungsi milik dosen
    Image* image = loadBMP("galaxy.bmp"); 
    
    // Mendaftarkan tekstur ke OpenGL
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Mengatur agar gambar terlihat tajam dan pas dengan layar
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Mengirim data piksel gambar ke GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 
                 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
                 
    delete image; // Menghapus memori gambar sementara karena sudah disimpan di GPU
}

// Fungsi khusus untuk menggambar background galaksi 2D
void drawBackground() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0); // Membuat koordinat layar datar (0,0) sampai (1,1)
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST); // Matikan depth test supaya menempel di paling belakang
    glEnable(GL_TEXTURE_2D);  // Aktifkan mode tekstur 2D seperti contoh dosen
    glBindTexture(GL_TEXTURE_2D, textureID); // Ikat ID tekstur galaxy.bmp

    glColor3f(1.0f, 1.0f, 1.0f); // Reset warna ke putih murni agar gambar tidak gelap
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f); // Kiri bawah
        glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f); // Kanan bawah
        glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f); // Kanan atas
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f); // Kiri atas
    glEnd();

    glDisable(GL_TEXTURE_2D); // Matikan tekstur kembali
    glEnable(GL_DEPTH_TEST);  // Hidupkan depth test lagi untuk objek 3D

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* -- Fungsi fabsf sederhana --------------- */
float myabs(float v) { return v < 0 ? -v : v; }

float gCamShake = 0.0f;

/* -- Reset / mulai game ------------------ */
void initGame()
{
    int i;
    gState = PLAYING;
    gScore = 0;
    gFrame = 0;
    gSpeed = 5.0f;
    gLane = 1;
    gPlayerX = LANES[1];
    gSpawnTimer = 0.0f;
    gSpawnInterval = 1.5f;
    for (i = 0; i < MAX_OBS; i++)
        gObs[i].active = 0;
}

/* -- Cek tabrakan ------------------------- */
int checkHit()
{
    int i;
    for (i = 0; i < MAX_OBS; i++)
    {
        if (!gObs[i].active)
            continue;
        if (myabs(gObs[i].z - 1.8f) < 1.7f && myabs(gObs[i].x - gPlayerX) < 0.9f)
            return 1;
    }
    return 0;
}

/* -- Gambar jalan ------------------------- */
void drawRoad()
{
    float z, zz, offset;

    /* Garis tepi kiri & kanan */
    glColor3f(0.6f, 0.6f, 0.6f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(-5.0f, -0.29f, 3.0f);
    glVertex3f(-5.0f, -0.29f, 70.0f);
    glVertex3f(5.0f, -0.29f, 3.0f);
    glVertex3f(5.0f, -0.29f, 70.0f);
    glEnd();

    /* Garis pembatas lane */
    glColor3f(0.35f, 0.35f, 0.35f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex3f(-1.5f, -0.29f, 3.0f);
    glVertex3f(-1.5f, -0.29f, 70.0f);
    glVertex3f(1.5f, -0.29f, 3.0f);
    glVertex3f(1.5f, -0.29f, 70.0f);
    glEnd();
}

/* -- Gambar teks 2D ----------------------- */
void drawText(float x, float y, const char *str)
{
    glRasterPos2f(x, y);
    while (*str)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *str);
        str++;
    }
}

/* -- Switch ke mode 2D untuk HUD ---------- */
void begin2D()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}

void end2D()
{
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* -- HUD ---------------------------------- */
void drawHUD()
{
    char buf[64];
    int i;
    begin2D();

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buf, "SCORE : %d", gScore);
    drawText(15, WIN_H - 25, buf);

    sprintf(buf, "SPEED : %.1f", gSpeed);
    drawText(15, WIN_H - 45, buf);

    /* Indikator lane */
    glColor3f(0.5f, 0.5f, 0.5f);
    drawText(15, WIN_H - 68, "LANE:");
    for (i = 0; i < 3; i++)
    {
        if ((2 - i) == gLane)
            glColor3f(0.3f, 1.0f, 0.4f);
        else
            glColor3f(0.4f, 0.4f, 0.4f);
        sprintf(buf, "[%d]", i + 1);
        drawText(80 + i * 30, WIN_H - 68, buf);
    }

    glColor3f(0.45f, 0.45f, 0.45f);
    drawText(15, 14, "A/D atau Arrow = gerak kiri/kanan | ESC = keluar");

    end2D();
}

/* -- Display callback --------------------- */
void display()
{
    int i;
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    drawBackground();

    /* Proyeksi perspektif */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(60.0, (double)WIN_W / WIN_H, 0.1, 200.0);

    /* Kamera mengikuti player sedikit */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(gPlayerX * 0.25 + gCamShake, 4.0, -6.0, gPlayerX * 0.1, 0.0, 25.0, 0.0, 1.0, 0.0);

    drawRoad();

    if (gState == PLAYING || gState == DEAD)
    {
        drawPlayer();
        for (i = 0; i < MAX_OBS; i++)
            if (gObs[i].active) {
            	drawObsShadow(gObs[i].x, gObs[i].z, i);
                drawObstacle(gObs[i].x, gObs[i].z, i);
			}
        drawHUD();
    }

    if (gState == MENU)
        drawMenu();
    if (gState == DEAD)
        drawDead();

    glutSwapBuffers();
}

/* -- Update ------------ */
void update(int val)
{
    int i;
    float now, diff, targetX;

    glutTimerFunc(16, update, 0);

    now = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    gDeltaTime = now - gLastTime;
    if (gDeltaTime > 0.05f)
        gDeltaTime = 0.05f;
    gLastTime = now;

    if (gState == PLAYING)
    {
        gFrame++;
        gScore = gFrame / 3;
        gSpeed = 5.0f + gFrame * 0.004f;
        gSpawnInterval = 1.5f - gFrame * 0.001f;
        if (gSpawnInterval < 0.6f)
            gSpawnInterval = 0.6f;

        /* Gerak player smooth ke lane target */
        targetX = LANES[gLane];
        diff = targetX - gPlayerX;
        gPlayerX += diff * 10.0f * gDeltaTime;
        if (myabs(diff) < 0.01f)
            gPlayerX = targetX;

        gCamShake += diff * 5.0f * gDeltaTime;

        // redam biar balik normal
        gCamShake *= 0.9f;

        /* Gerak obstacle ke depan */
        for (i = 0; i < MAX_OBS; i++)
        {
            if (!gObs[i].active)
                continue;
            gObs[i].z -= gSpeed * gDeltaTime;
            if (gObs[i].z < -6.0f)
                gObs[i].active = 0;
        }

        /* Spawn */
        gSpawnTimer += gDeltaTime;
        if (gSpawnTimer >= gSpawnInterval)
        {
            gSpawnTimer = 0.0f;
            spawnObs();
        }

        /* Cek tabrakan */
        if (checkHit())
            gState = DEAD;
    }

    glutPostRedisplay();
}

/* -- Input keyboard ----------------------- */
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case ' ':
    case 13:
        if (gState != PLAYING)
            initGame();
        break;
    case 'r':
    case 'R':
        if (gState == DEAD)
            initGame();
        break;
    case 'd':
    case 'D':
        if (gState == PLAYING && gLane > 0)
            gLane--;
        break;
    case 'a':
    case 'A':
        if (gState == PLAYING && gLane < 2)
            gLane++;
        break;
    }
}

void specialKey(int key, int x, int y)
{
    if (gState != PLAYING)
        return;
    if (key == GLUT_KEY_LEFT && gLane < 2)
        gLane++;
    if (key == GLUT_KEY_RIGHT && gLane > 0)
        gLane--;
}

/* -- Main --------------------------------- */
int main(int argc, char **argv)
{
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(150, 80);
    glutCreateWindow("Void Runner - OpenGL GLUT");

    glEnable(GL_DEPTH_TEST);
    
    initTexture();

    gLastTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
