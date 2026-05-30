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

// Variabel

// Waktu
float gStartTime = 0.0f;
float gFinishTime = 0.0f;

// Lane
float LANES[3] = {-3.0f, 0.0f, 3.0f};

// Animasi WIN
float gWinAnimTimer = 0.0f;
const float WIN_ANIM_DUR = 2.8f;

/* -- Kamera bebas (WASD) ------------------ */
float gCamYaw = 0.0f;   /* geser pandangan kiri/kanan */
float gCamPitch = 0.0f; /* geser pandangan atas/bawah */
bool gKeys[256] = {};

int gScore = 0;
int gFrame = 0;
int gShotScore = 0;
float gSpeed = 5.0f;
Obs gObs[MAX_OBS];

int gLane = 1;
float gPlayerX = 0.0f;

GLuint textureID;

/* -- Tembakan & efek hancur -------------- */
#define MAX_BULLETS 16
#define MAX_EXPLOSIONS 18
#define MAX_SHIP_PARTS 14

typedef struct
{
    float x, z;
    int active;
} Bullet;

typedef struct
{
    float x, z;
    float timer;
    int active;
} Explosion;

typedef struct
{
    float x, y, z;
    float vx, vy, vz;
    float rx, ry, rz;
    float size;
    int active;
} ShipPart;

Bullet gBullets[MAX_BULLETS];
Explosion gExplosions[MAX_EXPLOSIONS];
ShipPart gShipParts[MAX_SHIP_PARTS];
float gFireCooldown = 0.0f;
float gShipExplosionTimer = 0.0f;

//  Memasukan gambar ke program
void initTexture()
{
    // Membaca file gambar menggunakan fungsi milik dosen
    Image *image = loadBMP("galaxy.bmp");

    // Mendaftarkan tekstur ke OpenGL
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Mengatur agar gambar terlihat tajam dan pas dengan layar
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Mengirim data piksel gambar ke GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);

    delete image; // Menghapus memori gambar sementara karena sudah disimpan di GPU
}

// Fungsi khusus untuk menggambar background galaksi 2D
void drawBackground()
{
    /* Quad digambar 20% lebih besar dari layar di setiap sisi.
       Yang bergeser adalah posisi quad-nya (glTranslatef),
       bukan texcoord — sehingga tidak perlu tiling dan tidak ada seam. */
    const float OVER = 0.20f;

    /* Hitung pan dari kamera, dikurung agar tidak melewati batas quad */
    float panX = -gCamYaw * (OVER / 4.5f);
    float panY = gCamPitch * (OVER / 4.5f);
    if (panX > OVER)
        panX = OVER;
    if (panX < -OVER)
        panX = -OVER;
    if (panY > OVER)
        panY = OVER;
    if (panY < -OVER)
        panY = -OVER;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(panX, panY, 0.0f); /* geser quad, bukan texcoord */

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 1.0f, 1.0f);

    /* Quad lebih besar dari viewport: -OVER s/d 1+OVER
       Texcoord tetap 0..1 → tidak ada tiling, tidak ada seam */
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-OVER, -OVER);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f + OVER, -OVER);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f + OVER, 1.0f + OVER);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-OVER, 1.0f + OVER);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

/* -- Fungsi fabsf sederhana --------------- */
float myabs(float v) { return v < 0 ? -v : v; }

float gCamShake = 0.0f;

float clamp01(float v)
{
    if (v < 0.0f)
        return 0.0f;
    if (v > 1.0f)
        return 1.0f;
    return v;
}

void resetShotsAndEffects()
{
    int i;
    for (i = 0; i < MAX_BULLETS; i++)
        gBullets[i].active = 0;
    for (i = 0; i < MAX_EXPLOSIONS; i++)
        gExplosions[i].active = 0;
    for (i = 0; i < MAX_SHIP_PARTS; i++)
        gShipParts[i].active = 0;

    gFireCooldown = 0.0f;
    gShipExplosionTimer = 0.0f;
}

void fireBullet()
{
    int i;

    if (gState != PLAYING || gFireCooldown > 0.0f)
        return;

    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!gBullets[i].active)
        {
            gBullets[i].x = gPlayerX;
            gBullets[i].z = 3.0f;
            gBullets[i].active = 1;
            gFireCooldown = 0.18f;
            return;
        }
    }
}

void spawnExplosion(float x, float z)
{
    int i;
    for (i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (!gExplosions[i].active)
        {
            gExplosions[i].x = x;
            gExplosions[i].z = z;
            gExplosions[i].timer = 0.0f;
            gExplosions[i].active = 1;
            return;
        }
    }
}

void spawnShipExplosion()
{
    int i;
    float dirs[MAX_SHIP_PARTS][3] = {
        {-1.6f, 1.0f, -1.2f}, {-0.9f, 1.5f, 0.4f}, {0.8f, 1.4f, -0.5f},
        {1.5f, 0.9f, 1.0f}, {-0.4f, 1.9f, 1.5f}, {0.3f, 1.6f, -1.6f},
        {-1.2f, 0.6f, 1.4f}, {1.2f, 0.7f, -1.4f}, {-0.2f, 2.1f, 0.1f},
        {0.6f, 1.2f, 1.9f}, {-0.7f, 1.1f, -1.8f}, {1.8f, 0.8f, 0.0f},
        {-1.8f, 0.8f, 0.0f}, {0.0f, 2.2f, 1.0f}};

    gShipExplosionTimer = 0.0f;
    for (i = 0; i < MAX_SHIP_PARTS; i++)
    {
        gShipParts[i].x = gPlayerX;
        gShipParts[i].y = 0.0f;
        gShipParts[i].z = 1.8f;
        gShipParts[i].vx = dirs[i][0] * 2.8f;
        gShipParts[i].vy = dirs[i][1] * 2.2f;
        gShipParts[i].vz = dirs[i][2] * 2.3f;
        gShipParts[i].rx = 35.0f + i * 19.0f;
        gShipParts[i].ry = 80.0f + i * 23.0f;
        gShipParts[i].rz = 55.0f + i * 17.0f;
        gShipParts[i].size = 0.18f + (i % 4) * 0.05f;
        gShipParts[i].active = 1;
    }
}

void drawBullet(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0.10f, z);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glColor4f(0.15f, 0.95f, 1.0f, 0.90f);
    glutSolidSphere(0.11f, 14, 8);

    glScalef(0.55f, 0.55f, 1.9f);
    glColor4f(0.00f, 0.45f, 1.0f, 0.35f);
    glutSolidSphere(0.18f, 12, 8);

    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawExplosion(float x, float z, float timer)
{
    int i;
    float t = clamp01(timer / 0.65f);
    float fade = 1.0f - t;
    float radius = 0.25f + t * 1.8f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glPushMatrix();
    glTranslatef(x, 0.18f, z);
    glColor4f(1.0f, 0.38f, 0.05f, fade * 0.55f);
    glutSolidSphere(radius, 18, 10);
    glColor4f(1.0f, 0.9f, 0.2f, fade * 0.75f);
    glutSolidSphere(radius * 0.45f, 14, 8);
    glPopMatrix();

    glDisable(GL_BLEND);

    for (i = 0; i < 8; i++)
    {
        float ang = (float)i * 0.785398f + timer * 4.0f;
        float px = x + cos(ang) * radius * (0.55f + (i % 3) * 0.16f);
        float py = 0.10f + sin(timer * 6.0f + i) * 0.25f + t * 1.1f;
        float pz = z + sin(ang) * radius * (0.55f + (i % 2) * 0.20f);

        glPushMatrix();
        glTranslatef(px, py, pz);
        glRotatef(timer * 250.0f + i * 23.0f, 0.5f, 1.0f, 0.2f);
        glScalef(1.0f - t * 0.5f, 1.0f - t * 0.5f, 1.0f - t * 0.5f);
        glColor3f(0.42f, 0.35f, 0.28f);
        drawBox(0.18f, 0.14f, 0.20f);
        glPopMatrix();
    }
}

void drawShipExplosion()
{
    int i;
    float t = clamp01(gShipExplosionTimer / 1.6f);
    float fade = 1.0f - t;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glPushMatrix();
    glTranslatef(gPlayerX, 0.2f + t * 0.8f, 1.8f);
    glColor4f(1.0f, 0.22f, 0.05f, fade * 0.75f);
    glutSolidSphere(0.6f + t * 1.4f, 18, 10);
    glColor4f(0.15f, 0.75f, 1.0f, fade * 0.35f);
    glutSolidSphere(0.25f + t * 0.8f, 14, 8);
    glPopMatrix();
    glDisable(GL_BLEND);

    for (i = 0; i < MAX_SHIP_PARTS; i++)
    {
        if (!gShipParts[i].active)
            continue;

        glPushMatrix();
        glTranslatef(gShipParts[i].x, gShipParts[i].y, gShipParts[i].z);
        glRotatef(gShipExplosionTimer * gShipParts[i].rx, 1.0f, 0.0f, 0.0f);
        glRotatef(gShipExplosionTimer * gShipParts[i].ry, 0.0f, 1.0f, 0.0f);
        glRotatef(gShipExplosionTimer * gShipParts[i].rz, 0.0f, 0.0f, 1.0f);
        glColor3f(0.20f + (i % 3) * 0.08f, 0.22f, 0.24f);
        drawBox(gShipParts[i].size * 1.4f, gShipParts[i].size * 0.7f, gShipParts[i].size);
        glPopMatrix();
    }
}

void updateEffects()
{
    int i;

    for (i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (!gExplosions[i].active)
            continue;
        gExplosions[i].timer += gDeltaTime;
        if (gExplosions[i].timer > 0.65f)
            gExplosions[i].active = 0;
    }

    if (gShipExplosionTimer < 1.6f)
    {
        gShipExplosionTimer += gDeltaTime;
        for (i = 0; i < MAX_SHIP_PARTS; i++)
        {
            if (!gShipParts[i].active)
                continue;
            gShipParts[i].x += gShipParts[i].vx * gDeltaTime;
            gShipParts[i].y += gShipParts[i].vy * gDeltaTime;
            gShipParts[i].z += gShipParts[i].vz * gDeltaTime;
            gShipParts[i].vy -= 2.8f * gDeltaTime;
            if (gShipExplosionTimer >= 1.6f)
                gShipParts[i].active = 0;
        }
    }
}

void updateBullets()
{
    int i, j;

    if (gFireCooldown > 0.0f)
    {
        gFireCooldown -= gDeltaTime;
        if (gFireCooldown < 0.0f)
            gFireCooldown = 0.0f;
    }

    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!gBullets[i].active)
            continue;

        gBullets[i].z += 42.0f * gDeltaTime;
        if (gBullets[i].z > 72.0f)
        {
            gBullets[i].active = 0;
            continue;
        }

        for (j = 0; j < MAX_OBS; j++)
        {
            if (!gObs[j].active)
                continue;

            if (myabs(gBullets[i].z - gObs[j].z) < 1.1f &&
                myabs(gBullets[i].x - gObs[j].x) < 0.95f)
            {
                spawnExplosion(gObs[j].x, gObs[j].z);
                gObs[j].active = 0;
                gBullets[i].active = 0;
                gShotScore += 25;
                gCamShake += 0.22f;
                break;
            }
        }
    }
}

/* -- Reset / mulai game ------------------ */
void initGame()
{
    int i;
    gState = PLAYING;
    gScore = 0;
    gShotScore = 0;
    gFrame = 0;
    gSpeed = 5.0f;
    gLane = 1;
    gPlayerX = LANES[1];
    gSpawnTimer = 0.0f;
    gSpawnInterval = 1.5f;
    gWinAnimTimer = 0.0f;
    gStartTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    gFinishTime = 0.0f;
    gCamYaw = 0.0f;
    gCamPitch = 0.0f;
    resetShotsAndEffects();
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

        /* Z: half-length roket (1.63) + radius asteroid (0.9) = 2.5
           X: half-width fins  (0.55) + radius asteroid (0.9) = 1.3  */
        if (myabs(gObs[i].z - 1.8f) < 1.5f && myabs(gObs[i].x - gPlayerX) < 1.3f)
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
    drawText(15, 14, "A/D atau Arrow = gerak kiri/kanan | Space/F = tembak | ESC = keluar");

    end2D();
}

/* -- Display callback --------------------- */
void display()
{
    int i;
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)WIN_W / WIN_H, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Kamera — WIN_ANIM mendongak secara bertahap */
    float camEyeY = 4.0f;
    float camLookY = gCamPitch;
    if (gState == WIN_ANIM)
    {
        float ease = (gWinAnimTimer / WIN_ANIM_DUR);
        ease *= ease;
        camEyeY += ease * 2.5f;
        camLookY += ease * 4.0f;
    }
    gluLookAt(
        gPlayerX * 0.25 + gCamShake, camEyeY, -6.0,
        gPlayerX * 0.1 + gCamYaw, camLookY, 25.0,
        0.0, 1.0, 0.0);

    drawRoad(); /* digambar SATU kali di sini */

    /* --- STATE PLAYING / DEAD --- */
    if (gState == PLAYING || gState == DEAD)
    {
        if (gState == DEAD && gShipExplosionTimer < 1.6f)
            drawShipExplosion();
        else if (gState == PLAYING)
            drawPlayer();

        for (i = 0; i < MAX_BULLETS; i++)
            if (gBullets[i].active)
                drawBullet(gBullets[i].x, gBullets[i].z);

        for (i = 0; i < MAX_OBS; i++)
            if (gObs[i].active)
            {
                drawObsShadow(gObs[i].x, gObs[i].z, i);
                drawObstacle(gObs[i].x, gObs[i].z, i);
            }

        for (i = 0; i < MAX_EXPLOSIONS; i++)
            if (gExplosions[i].active)
                drawExplosion(gExplosions[i].x, gExplosions[i].z, gExplosions[i].timer);

        drawHUD();
    }

    /* --- STATE WIN_ANIM: roket terbang --- */
    if (gState == WIN_ANIM)
    {
        float t = gWinAnimTimer / WIN_ANIM_DUR;
        float ease = t * t;
        float offY = ease * 14.0f;
        float offZ = ease * 8.0f;
        float pitch = -ease * 65.0f;
        float sc = 1.0f - t * 0.92f;
        if (sc < 0.03f)
            sc = 0.03f;

        /* Asteroid beku tetap terlihat */
        for (i = 0; i < MAX_OBS; i++)
            if (gObs[i].active)
            {
                drawObsShadow(gObs[i].x, gObs[i].z, i);
                drawObstacle(gObs[i].x, gObs[i].z, i);
            }

        for (i = 0; i < MAX_EXPLOSIONS; i++)
            if (gExplosions[i].active)
                drawExplosion(gExplosions[i].x, gExplosions[i].z, gExplosions[i].timer);

        /* Roket dengan transform animasi */
        float savedX = gPlayerX;
        gPlayerX = 0.0f;
        glPushMatrix();
        glTranslatef(savedX, offY, offZ);
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);
        glScalef(sc, sc, sc);
        drawPlayer();
        glPopMatrix();
        gPlayerX = savedX;

        drawHUD();
    }

    if (gState == MENU)
        drawMenu();
    if (gState == DEAD)
        drawDead();
    if (gState == WIN)
        drawWin(gScore, gFinishTime - gStartTime);

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

    updateEffects();

    if (gState == PLAYING)
    {
        gFrame++;
        gScore = gFrame / 3 + gShotScore;
        gSpeed = 5.0f + gFrame * 0.004f;
        gSpawnInterval = 1.5f - gFrame * 0.001f;
        if (gSpawnInterval < 0.6f)
            gSpawnInterval = 0.6f;

        /* --- CEK MENANG --- */
        if (gScore >= 1000 && gState == PLAYING)
        {
            gFinishTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
            gWinAnimTimer = 0.0f;
            gState = WIN_ANIM;
        }

        /* Gerak player smooth ke lane target */
        targetX = LANES[gLane];
        diff = targetX - gPlayerX;
        gPlayerX += diff * 15.0f * gDeltaTime;
        if (myabs(diff) < 0.01f)
            gPlayerX = targetX;

        gCamShake += diff * 5.0f * gDeltaTime;

        // redam biar balik normal
        gCamShake *= 0.9f;

        /* -- Gerak kamera bebas (WASD) -- */
        float cs = 4.5f * gDeltaTime;

        if (gKeys['w'] || gKeys['W'])
            gCamPitch += cs;
        if (gKeys['s'] || gKeys['S'])
            gCamPitch -= cs;
        if (gKeys['a'] || gKeys['A'])
            gCamYaw -= cs;
        if (gKeys['d'] || gKeys['D'])
            gCamYaw += cs;

        /* Batas pandangan agar tidak terlalu jauh */
        if (gCamPitch > 6.0f)
            gCamPitch = 6.0f;
        if (gCamPitch < -6.5f)
            gCamPitch = -6.5f;
        if (gCamYaw > 10.5f)
            gCamYaw = 10.5f;
        if (gCamYaw < -10.5f)
            gCamYaw = -10.5f;

        /* Gerak obstacle ke depan */
        for (i = 0; i < MAX_OBS; i++)
        {
            if (!gObs[i].active)
                continue;
            gObs[i].z -= gSpeed * gDeltaTime;
            if (gObs[i].z < -6.0f)
                gObs[i].active = 0;
        }

        updateBullets();

        /* Spawn */
        gSpawnTimer += gDeltaTime;
        if (gSpawnTimer >= gSpawnInterval)
        {
            gSpawnTimer = 0.0f;
            spawnObs();
        }

        /* Cek tabrakan */
        if (checkHit())
        {
            spawnShipExplosion();
            gState = DEAD;
        }
    }

    glutPostRedisplay();

    if (gState == WIN_ANIM)
    {
        gWinAnimTimer += gDeltaTime;
        if (gWinAnimTimer >= WIN_ANIM_DUR)
            gState = WIN;
    }
}

/* -- Input keyboard ----------------------- */
void keyboard(unsigned char key, int x, int y)
{
    gKeys[key] = true;

    switch (key)
    {
    case 27:
        exit(0);
        break;
    case ' ':
        if (gState == PLAYING)
            fireBullet();
        else if (gState == MENU || gState == DEAD || gState == WIN)
            initGame();
        break;
    case 13:
        if (gState == MENU || gState == DEAD || gState == WIN)
            initGame();
        break;
    case 'f':
    case 'F':
        fireBullet();
        break;
    case 'r':
    case 'R':
        if (gState == DEAD || gState == WIN)
            initGame();
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y)
{
    gKeys[key] = false;
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
    glutCreateWindow("Hypernova Escape");

    glEnable(GL_DEPTH_TEST);

    initTexture();

    gLastTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
