/*
    VOID RUNNER - Simple Version
    OpenGL + GLUT


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

int gLane = 1;
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
        if (myabs(gObs[i].z - 1.8f) < 1.7f && myabs(gObs[i].x - gPlayerX) < 0.9f)
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

/* -- Gambar player (roket luar angkasa) ----------- */
void drawPlayer()
{
    glPushMatrix();
    glTranslatef(gPlayerX, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 1.8f);

    /* === BADAN UTAMA (fuselage putih-abu) === */
    glColor3f(0.92f, 0.92f, 0.95f);
    drawBox(0.30f, 0.30f, 1.75f);

    /* Selubung belakang — sedikit lebih lebar */
    glColor3f(0.82f, 0.83f, 0.88f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.55f);
    drawBox(0.38f, 0.38f, 0.55f);
    glPopMatrix();

    /* === NOSE CONE bertingkat mengerucut ke +z === */
    glColor3f(0.88f, 0.88f, 0.92f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.02f);
    drawBox(0.24f, 0.24f, 0.38f);
    glPopMatrix();

    glColor3f(0.80f, 0.80f, 0.86f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.28f);
    drawBox(0.16f, 0.16f, 0.28f);
    glPopMatrix();

    glColor3f(0.70f, 0.70f, 0.78f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.48f);
    drawBox(0.09f, 0.09f, 0.18f);
    glPopMatrix();

    /* Ujung paling runcing */
    glColor3f(0.60f, 0.60f, 0.70f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.62f);
    drawBox(0.04f, 0.04f, 0.10f);
    glPopMatrix();

    /* === CINCIN AKSEN ORANYE === */
    glColor3f(1.0f, 0.42f, 0.07f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.55f);
    drawBox(0.315f, 0.315f, 0.09f);
    glPopMatrix();

    glColor3f(1.0f, 0.42f, 0.07f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.22f);
    drawBox(0.315f, 0.315f, 0.07f);
    glPopMatrix();

    /* === JENDELA (porthole biru) === */
    glColor3f(0.25f, 0.55f, 0.95f);
    glPushMatrix();
    glTranslatef(0.155f, 0.06f, 0.62f);
    drawBox(0.04f, 0.09f, 0.09f);
    glPopMatrix();

    /* === SECTION MESIN === */
    glColor3f(0.48f, 0.50f, 0.58f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.98f);
    drawBox(0.40f, 0.40f, 0.32f);
    glPopMatrix();

    /* === 4 SIRIP EKOR (fins) === */
    glColor3f(0.88f, 0.28f, 0.06f);

    /* Atas */
    glPushMatrix();
    glTranslatef(0.0f, 0.40f, -0.92f);
    drawBox(0.065f, 0.55f, 0.62f);
    glPopMatrix();

    /* Bawah */
    glPushMatrix();
    glTranslatef(0.0f, -0.40f, -0.92f);
    drawBox(0.065f, 0.55f, 0.62f);
    glPopMatrix();

    /* Kiri */
    glPushMatrix();
    glTranslatef(-0.40f, 0.0f, -0.92f);
    drawBox(0.55f, 0.065f, 0.62f);
    glPopMatrix();

    /* Kanan */
    glPushMatrix();
    glTranslatef(0.40f, 0.0f, -0.92f);
    drawBox(0.55f, 0.065f, 0.62f);
    glPopMatrix();

    /* Ujung sirip (sedikit lebih gelap) */
    glColor3f(0.65f, 0.18f, 0.04f);
    glPushMatrix();
    glTranslatef(0.0f, 0.68f, -1.05f);
    drawBox(0.055f, 0.18f, 0.38f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -0.68f, -1.05f);
    drawBox(0.055f, 0.18f, 0.38f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.68f, 0.0f, -1.05f);
    drawBox(0.18f, 0.055f, 0.38f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.68f, 0.0f, -1.05f);
    drawBox(0.18f, 0.055f, 0.38f);
    glPopMatrix();

    /* === NOZZLE MESIN === */
    glColor3f(0.15f, 0.15f, 0.20f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.20f);
    drawBox(0.26f, 0.26f, 0.16f);
    glPopMatrix();

    /* Bibir nozzle (ring gelap) */
    glColor3f(0.10f, 0.10f, 0.14f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.30f);
    drawBox(0.30f, 0.30f, 0.06f);
    glPopMatrix();

    /* === API MESIN (animasi flicker) === */
    float flicker = 0.78f + 0.22f * (float)sin(gFrame * 0.45f);
    float flicker2 = 0.70f + 0.30f * (float)sin(gFrame * 0.60f + 1.2f);

    /* Lapisan luar — oranye */
    glColor3f(1.0f * flicker, 0.48f * flicker, 0.04f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.42f);
    drawBox(0.22f * flicker, 0.22f * flicker, 0.18f);
    glPopMatrix();

    /* Lapisan tengah — kuning */
    glColor3f(1.0f, 0.82f * flicker2, 0.18f * flicker2);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.52f);
    drawBox(0.13f * flicker2, 0.13f * flicker2, 0.14f);
    glPopMatrix();

    /* Inti api — putih */
    glColor3f(1.0f, 0.97f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.60f);
    drawBox(0.06f, 0.06f, 0.09f);
    glPopMatrix();

    glPopMatrix();
}

/* -- Gambar satu kotak kasar (chunk batu) -- */
void drawChunk(float w, float h, float d, float rx, float ry, float rz, float angle)
{
    glPushMatrix();
    glRotatef(angle, rx, ry, rz);
    drawBox(w, h, d);
    glPopMatrix();
}

/* -- Gambar asteroid berbatu ------------------ */
void drawAsteroid(float x, float z, int id)
{
    /* Tiap asteroid punya ukuran & rotasi berbeda berdasarkan id */
    float scale = 0.85f + (id % 4) * 0.1f;
    float rSpeed = 18.0f + (id % 6) * 7.0f;
    float rAngle = (float)fmod(gFrame * rSpeed * 0.01, 360.0);
    float axX = 0.3f + (id % 3) * 0.25f;
    float axZ = 0.2f + (id % 5) * 0.15f;

    glPushMatrix();
    glTranslatef(x, 0.1f * scale, z);

    /* Tumbling rotation — tiap asteroid berputar dengan sumbu unik */
    glRotatef(rAngle, axX, 1.0f, axZ);
    glScalef(scale, scale, scale);

    /* -- Badan utama (abu gelap kecokelatan) -- */
    glColor3f(0.40f, 0.36f, 0.30f);
    drawBox(1.3f, 1.15f, 1.4f);

    /* -- Tonjolan atas-kiri -- */
    glColor3f(0.33f, 0.29f, 0.24f);
    drawChunk(0.75f, 0.6f, 0.65f, 0.3f, 0.8f, 0.5f, 32.0f);

    /* -- Tonjolan bawah-kanan -- */
    glColor3f(0.30f, 0.27f, 0.22f);
    glPushMatrix();
    glTranslatef(0.5f, -0.45f, 0.3f);
    drawChunk(0.65f, 0.55f, 0.7f, 0.7f, 0.2f, 0.6f, -28.0f);
    glPopMatrix();

    /* -- Tonjolan sisi kiri -- */
    glColor3f(0.35f, 0.31f, 0.26f);
    glPushMatrix();
    glTranslatef(-0.62f, 0.15f, -0.2f);
    drawChunk(0.6f, 0.75f, 0.5f, 0.5f, 0.3f, 0.8f, 20.0f);
    glPopMatrix();

    /* -- Tonjolan depan -- */
    glColor3f(0.28f, 0.25f, 0.20f);
    glPushMatrix();
    glTranslatef(0.2f, 0.35f, 0.6f);
    drawChunk(0.55f, 0.5f, 0.6f, 0.1f, 0.9f, 0.4f, -35.0f);
    glPopMatrix();

    /* -- Tonjolan belakang -- */
    glColor3f(0.32f, 0.28f, 0.23f);
    glPushMatrix();
    glTranslatef(-0.25f, -0.2f, -0.62f);
    drawChunk(0.7f, 0.45f, 0.55f, 0.6f, 0.4f, 0.2f, 18.0f);
    glPopMatrix();

    /* -- Kawah 1 (cekungan gelap di permukaan atas) -- */
    glColor3f(0.16f, 0.14f, 0.11f);
    glPushMatrix();
    glTranslatef(0.1f, 0.62f, 0.05f);
    glScalef(1.0f, 0.15f, 1.0f); /* pipih = kesan kawah */
    drawBox(0.42f, 0.3f, 0.38f);
    glPopMatrix();

    /* -- Kawah 2 (sisi depan) -- */
    glColor3f(0.14f, 0.12f, 0.10f);
    glPushMatrix();
    glTranslatef(-0.2f, 0.1f, 0.68f);
    glScalef(0.15f, 1.0f, 1.0f);
    drawBox(0.3f, 0.32f, 0.28f);
    glPopMatrix();

    /* -- Bercak terang (highlight mineral) -- */
    glColor3f(0.55f, 0.50f, 0.42f);
    glPushMatrix();
    glTranslatef(0.35f, 0.55f, 0.35f);
    glScalef(1.0f, 0.12f, 1.0f);
    drawBox(0.2f, 0.18f, 0.18f);
    glPopMatrix();

    glColor3f(0.50f, 0.46f, 0.38f);
    glPushMatrix();
    glTranslatef(-0.4f, 0.3f, 0.4f);
    glScalef(1.0f, 0.12f, 1.0f);
    drawBox(0.15f, 0.15f, 0.16f);
    glPopMatrix();

    glPopMatrix();
}

/* -- Wrapper -- */
void drawObstacle(float x, float z, int id)
{
    drawAsteroid(x, z, id);
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
                drawObstacle(gObs[i].x, gObs[i].z, i);
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