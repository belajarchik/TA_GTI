#ifdef _WIN32
#include <windows.h>
#endif

// #include <GL/gl.h>
#include <GL/freeglut.h>
// #include <GL/glut.h>
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
float gSpeed = 5.0f;
Obs gObs[MAX_OBS];

int gLane = 1;
float gPlayerX = 0.0f;

GLuint textureID;

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

    #ifndef GL_CLAMP_TO_EDGE
    #define GL_CLAMP_TO_EDGE 0x812F
    #endif

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
    // gluOrtho2D(0.0, 1.0, 0.0, 1.0);
    float aspect = (float)WIN_W / (float)WIN_H;

    if (aspect >= 1.0f)
        gluOrtho2D(-aspect, aspect, -1.0, 1.0);
    else
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(panX, panY, 0.0f); /* geser quad, bukan texcoord */

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glColor3f(1.0f, 1.0f, 1.0f);

    float left, right, bottom, top;

    if (aspect >= 1.0f)
    {
        left = -aspect - OVER;
        right = aspect + OVER;
        bottom = -1.0f - OVER;
        top = 1.0f + OVER;
    }
    else
    {
        left = -1.0f - OVER;
        right = 1.0f + OVER;
        bottom = -1.0f / aspect - OVER;
        top = 1.0f / aspect + OVER;
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(left, bottom);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(right, bottom);

    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(right, top);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(left, top);
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
    gWinAnimTimer = 0.0f;
    gStartTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    gFinishTime = 0.0f;
    gCamYaw = 0.0f;
    gCamPitch = 0.0f;
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
    gluLookAt(gPlayerX * 0.25 + gCamShake, camEyeY, -6.0,
              gPlayerX * 0.1 + gCamYaw, camLookY, 25.0,
              0.0, 1.0, 0.0);

    drawRoad(); /* digambar SATU kali di sini */

    /* --- STATE PLAYING / DEAD --- */
    if (gState == PLAYING || gState == DEAD)
    {
        drawPlayer();
        for (i = 0; i < MAX_OBS; i++)
            if (gObs[i].active)
            {
                drawObsShadow(gObs[i].x, gObs[i].z, i);
                drawObstacle(gObs[i].x, gObs[i].z, i);
            }
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

    if (gState == PLAYING)
    {
        gFrame++;
        gScore = gFrame / 3;
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
    case 13:
        if (gState == MENU || gState == DEAD || gState == WIN)
            initGame();
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

void reshape(int w, int h)
{
    if (h == 0)
        h = 1;

    WIN_W = w;
    WIN_H = h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, (float)w / h, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
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
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKey);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
