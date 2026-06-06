#include "obstacle.h"

float gSpawnTimer = 0.0f;
float gSpawnInterval = 1.5f;
float gLastTime = 0.0f;
float gDeltaTime = 0.0f;

/* -- Gambar kotak manual ------------------ */
void drawBox(float w, float h, float d)
{
    float hw = w * 0.5f, hh = h * 0.5f, hd = d * 0.5f;
    glBegin(GL_QUADS);
    /* depan */
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(-hw, hh, hd);
    /* belakang */
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(hw, hh, -hd);
    /* kiri */
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(-hw, hh, -hd);
    /* kanan */
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, hh, hd);
    /* atas */
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-hw, hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(-hw, hh, -hd);
    /* bawah */
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(-hw, -hh, hd);
    glEnd();
}

/* -- Gambar satu kotak kasar (chunk batu) -- */
void drawChunk(float w, float h, float d, float rx, float ry, float rz, float angle)
{
    glPushMatrix();
    glRotatef(angle, rx, ry, rz);
    drawBox(w, h, d);
    glPopMatrix();
}

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

// Wrapper
void drawObstacle(float x, float z, int id)
{
    drawAsteroid(x, z, id);
}

// Spawn obstacle
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

void drawObsShadow(float x, float z, int id)
{
    float scale = 0.85f + (id % 4) * 0.1f;
    float shadowY = -0.28f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    // inti bayangan hitam pekat pas di bawah asteroid
    float w1 = (0.75f + 0.15f) * scale;
    float h1 = (0.80f + 0.15f) * scale;
    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    glBegin(GL_QUADS);
    glVertex3f(x - w1, shadowY, z - h1);
    glVertex3f(x + w1, shadowY, z - h1);
    glVertex3f(x + w1, shadowY, z + h1);
    glVertex3f(x - w1, shadowY, z + h1);
    glEnd();

    // luar lebih besar, lebih transparan
    float w2 = w1 * 1.6f;
    float h2 = h1 * 1.6f;
    glColor4f(0.0f, 0.0f, 0.0f, 0.22f);
    glBegin(GL_QUADS);
    glVertex3f(x - w2, shadowY, z - h2);
    glVertex3f(x + w2, shadowY, z - h2);
    glVertex3f(x + w2, shadowY, z + h2);
    glVertex3f(x - w2, shadowY, z + h2);
    glEnd();

    // terluar tipis
    float w3 = w1 * 2.2f;
    float h3 = h1 * 2.2f;
    glColor4f(0.0f, 0.0f, 0.0f, 0.08f);
    glBegin(GL_QUADS);
    glVertex3f(x - w3, shadowY, z - h3);
    glVertex3f(x + w3, shadowY, z - h3);
    glVertex3f(x + w3, shadowY, z + h3);
    glVertex3f(x - w3, shadowY, z + h3);
    glEnd();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
