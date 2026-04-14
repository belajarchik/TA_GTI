/*
    VOID RUNNER - Simple Version
    OpenGL + GLUT (Dev-C++)

    TIDAK menggunakan freeglut.
    Gunakan glut32 (GLUT standar untuk Windows).

    Setup Dev-C++:
      Linker: -lopengl32 -lglu32 -lglut32

      File yang dibutuhkan (taruh di folder project):
        glut32.dll
        glut32.lib  (atau libglut32.a untuk MinGW)
        glut.h      (taruh di folder include\GL\)

    Kontrol:
      Arrow Left / A  : Pindah kiri
      Arrow Right / D : Pindah kanan
      SPACE / ENTER   : Mulai / Restart
      ESC             : Keluar
*/

#ifdef _WIN32
#include <windows.h>
#endif

// #include <GL/gl.h>
// #include <GL/glu.h>
#include <GLUT/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

using namespace std;

/* PAKSA DECLARE */
extern "C"
{
    int rand(void);
    void srand(unsigned int seed);
}

/* -- Konstanta --------------------------- */
#define WIN_W 800
#define WIN_H 600
#define MAX_OBS 10

float LANES[3] = {-3.0f, 0.0f, 3.0f};

/* -- State game --------------------------- */
typedef enum
{
    MENU,
    PLAYING,
    DEAD
} State;
State gState = MENU;

int gScore = 0;
int gFrame = 0;
float gSpeed = 5.0f;

int gLane = 1; /* lane player: 0=kiri, 1=tengah, 2=kanan */
float gPlayerX = 0.0f;

/* -- Obstacle ---------------------------- */
typedef struct
{
    float x, z;
    int active;
} Obs;

Obs gObs[MAX_OBS];
float gSpawnTimer = 0.0f;
float gSpawnInterval = 1.5f;
float gLastTime = 0.0f;
float gDeltaTime = 0.0f;

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

/* -- Spawn obstacle ----------------------- */
void spawnObs()
{
    int i;
    for (i = 0; i < MAX_OBS; i++)
    {
        if (!gObs[i].active)
        {
            gObs[i].x = LANES[rand() % 3];
            gObs[i].z = 55.0f;
            gObs[i].active = 1;
            return;
        }
    }
}

/* -- Cek tabrakan ------------------------- */
int checkHit()
{
    int i;
    for (i = 0; i < MAX_OBS; i++)
    {
        if (!gObs[i].active)
            continue;
        if (myabs(gObs[i].z) < 1.8f && myabs(gObs[i].x - gPlayerX) < 1.5f)
            return 1;
    }
    return 0;
}

/* -- Gambar kotak manual ------------------ */
void drawBox(float w, float h, float d)
{
    float hw = w * 0.5f, hh = h * 0.5f, hd = d * 0.5f;
    glBegin(GL_QUADS);
    /* Front */
    glVertex3f(-hw, -hh, hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(-hw, hh, hd);
    /* Back */
    glVertex3f(hw, -hh, -hd);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(hw, hh, -hd);
    /* Left */
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(-hw, hh, -hd);
    /* Right */
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, hh, hd);
    /* Top */
    glVertex3f(-hw, hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(-hw, hh, -hd);
    /* Bottom */
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(-hw, -hh, hd);
    glEnd();
}

/* -- Gambar player (kotak biru) ----------- */
void drawPlayer()
{
    glPushMatrix();
    glTranslatef(gPlayerX, 0.0f, 0.0f);
    /* Badan */
    glColor3f(0.2f, 0.6f, 1.0f);
    drawBox(1.5f, 0.5f, 2.0f);
    /* Sayap kiri */
    glColor3f(0.1f, 0.3f, 0.8f);
    glPushMatrix();
    glTranslatef(-1.2f, 0.0f, 0.3f);
    drawBox(1.0f, 0.1f, 0.9f);
    glPopMatrix();
    /* Sayap kanan */
    glPushMatrix();
    glTranslatef(1.2f, 0.0f, 0.3f);
    drawBox(1.0f, 0.1f, 0.9f);
    glPopMatrix();
    glPopMatrix();
}

/* -- Gambar rintangan (kotak merah) ------- */
void drawObstacle(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glColor3f(1.0f, 0.2f, 0.1f);
    drawBox(1.8f, 1.8f, 1.8f);
    glPopMatrix();
}

/* -- Gambar jalan ------------------------- */
void drawRoad()
{
    float z, zz, offset;

    /* Lantai gelap */
    glColor3f(0.08f, 0.08f, 0.1f);
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, -0.3f, 3.0f);
    glVertex3f(5.0f, -0.3f, 3.0f);
    glVertex3f(5.0f, -0.3f, 70.0f);
    glVertex3f(-5.0f, -0.3f, 70.0f);
    glEnd();

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

    /* Garis horizontal (kesan gerakan) */
    glColor3f(0.2f, 0.2f, 0.2f);
    offset = (float)fmod(gFrame * gSpeed * 0.005, 5.0);
    for (z = 5.0f; z < 70.0f; z += 5.0f)
    {
        zz = z - offset;
        glBegin(GL_LINES);
        glVertex3f(-5.0f, -0.29f, zz);
        glVertex3f(5.0f, -0.29f, zz);
        glEnd();
    }
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
        if (i == gLane)
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

/* -- Layar Menu --------------------------- */
void drawMenu()
{
    begin2D();
    float cx = WIN_W / 2.0f, cy = WIN_H / 2.0f;

    glColor3f(0.3f, 0.8f, 1.0f);
    drawText(cx - 65, cy + 60, "VOID RUNNER 3D");

    glColor3f(0.9f, 0.9f, 0.9f);
    drawText(cx - 105, cy + 25, "Gunakan Arrow Kiri/Kanan atau A/D");
    drawText(cx - 85, cy + 5, "untuk berpindah lane.");

    glColor3f(1.0f, 0.4f, 0.4f);
    drawText(cx - 105, cy - 22, "Nabrak rintangan = langsung MATI!");

    glColor3f(0.3f, 1.0f, 0.5f);
    drawText(cx - 90, cy - 55, "Tekan SPASI / ENTER untuk mulai");

    end2D();
}

/* -- Layar Game Over ---------------------- */
void drawDead()
{
    char buf[64];
    begin2D();
    float cx = WIN_W / 2.0f, cy = WIN_H / 2.0f;

    glColor3f(1.0f, 0.2f, 0.2f);
    drawText(cx - 45, cy + 50, "GAME OVER");

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buf, "Skor akhir: %d", gScore);
    drawText(cx - 55, cy + 18, buf);

    glColor3f(0.3f, 1.0f, 0.5f);
    drawText(cx - 100, cy - 20, "Tekan R / SPASI / ENTER untuk ulangi");

    glColor3f(0.5f, 0.5f, 0.5f);
    drawText(cx - 55, cy - 48, "ESC untuk keluar");

    end2D();
}

/* -- Display callback --------------------- */
void display()
{
    int i;
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
            if (gObs[i].active)
                drawObstacle(gObs[i].x, gObs[i].z);
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

    gLastTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}