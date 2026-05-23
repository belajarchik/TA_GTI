#include "roket.h"

extern float gPlayerX;
extern int   gFrame;
extern float gDeltaTime;

//  Helper untuk gambar quad tertutup (normal manual)

static void quad(
    float x0,float y0,float z0,
    float x1,float y1,float z1,
    float x2,float y2,float z2,
    float x3,float y3,float z3)
{
    glBegin(GL_QUADS);
    glVertex3f(x0,y0,z0);
    glVertex3f(x1,y1,z1);
    glVertex3f(x2,y2,z2);
    glVertex3f(x3,y3,z3);
    glEnd();
}

//  COCKPIT  
void cockpit()
{
    glPushMatrix();
    // Geser sedikit ke depan-atas permukaan badan
    glTranslatef(0.0f, 0.12f, -0.15f);
    
    // Condong ke depan agar sejajar leading edge
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    
    // Skala jadi pipih
    glScalef(0.38f, 0.10f, 0.22f);

    // Warna gelap transparan pada kaca kokpit
    glColor3f(0.15f, 0.20f, 0.25f);
    glutSolidSphere(1.0f, 16, 8);

    // Ring highlight tipis
    glColor3f(0.45f, 0.55f, 0.60f);
    glutWireSphere(1.01f, 12, 6);
    glPopMatrix();
}

//  BADAN 
void badan(float wF, float wB, float hT, float zFront, float zBack)
{
    float  w0 = wF,  w1 = wB;   
    float  h  = hT;             

    // 8 vertex : (depan-kiri/kanan atas/bawah) + (belakang-kiri/kanan atas/bawah)
    // Sumbu jet : maju = -Z,  atas = +Y
    float fLU[3] = {-w0,  h, zFront};
    float fRU[3] = { w0,  h, zFront};
    float fLD[3] = {-w0, -h, zFront};
    float fRD[3] = { w0, -h, zFront};

    float bLU[3] = {-w1,  h, zBack};
    float bRU[3] = { w1,  h, zBack};
    float bLD[3] = {-w1, -h, zBack};
    float bRD[3] = { w1, -h, zBack};

    glBegin(GL_QUADS);

    // Atas
    glVertex3fv(fLU); glVertex3fv(fRU);
    glVertex3fv(bRU); glVertex3fv(bLU);

    // Bawah
    glVertex3fv(fLD); glVertex3fv(fRD);
    glVertex3fv(bRD); glVertex3fv(bLD);

    // Kiri
    glVertex3fv(fLU); glVertex3fv(fLD);
    glVertex3fv(bLD); glVertex3fv(bLU);

    // Kanan
    glVertex3fv(fRU); glVertex3fv(fRD);
    glVertex3fv(bRD); glVertex3fv(bRU);

    // Depan 
    glVertex3fv(fLU); glVertex3fv(fRU);
    glVertex3fv(fRD); glVertex3fv(fLD);

    // Belakang 
    glVertex3fv(bLU); glVertex3fv(bRU);
    glVertex3fv(bRD); glVertex3fv(bLD);

    glEnd();
}


//  SAYAP  
void sayap(float span, float zLE, float zTE, float zTip, float chordTip, float hT)
{
    // Vertex atas (+Y) dan bawah (-Y)
    // Sisi kanan (X positif)
    float A[3] = {0.0f,  0.0f,  zLE};
    float B[3] = {span,  0.0f,  zTip};
    float C[3] = {span,  0.0f,  zTip + chordTip};
    float D[3] = {0.0f,  0.0f,  zTE};

    // Tebal tipis atas/bawah
    float h = hT;

    glBegin(GL_QUADS);

    // Atas (Y = +h)
    glVertex3f(A[0], h, A[2]);
    glVertex3f(B[0], h, B[2]);
    glVertex3f(C[0], h, C[2]);
    glVertex3f(D[0], h, D[2]);

    // Bawah (Y = -h)
    glVertex3f(A[0],-h, A[2]);
    glVertex3f(B[0],-h, B[2]);
    glVertex3f(C[0],-h, C[2]);
    glVertex3f(D[0],-h, D[2]);

    glEnd();

    // Leading edge (depan miring)
    glBegin(GL_QUADS);
    glVertex3f(A[0], h, A[2]); glVertex3f(B[0], h, B[2]);
    glVertex3f(B[0],-h, B[2]); glVertex3f(A[0],-h, A[2]);
    glEnd();

    // Trailing edge (belakang miring)
    glBegin(GL_QUADS);
    glVertex3f(D[0], h, D[2]); glVertex3f(C[0], h, C[2]);
    glVertex3f(C[0],-h, C[2]); glVertex3f(D[0],-h, D[2]);
    glEnd();

    // Wingtip (ujung sayap)
    glBegin(GL_QUADS);
    glVertex3f(B[0], h, B[2]); glVertex3f(C[0], h, C[2]);
    glVertex3f(C[0],-h, C[2]); glVertex3f(B[0],-h, B[2]);
    glEnd();
}

// ---------------------------------------------
//  ENGINE NOZZLE  –  dua nosel di buritan
// ---------------------------------------------
void nozzle(float offsetX)
{
    glPushMatrix();
    glTranslatef(offsetX, -0.05f, 0.65f);   // posisi di belakang badan
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);     // hadap belakang

    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);

    // Ring luar (gelap)
    glColor3f(0.18f, 0.18f, 0.18f);
    gluCylinder(q, 0.14f, 0.11f, 0.18f, 16, 2);

    // Tutup depan ring
    glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
    gluDisk(q, 0.08f, 0.14f, 16, 1);
    glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

    // Pijar mesin (kuning-oranye)
    glTranslatef(0.0f, 0.0f, 0.18f);
    glColor3f(1.0f, 0.55f, 0.05f);
    gluDisk(q, 0.0f, 0.09f, 16, 1);

    gluDeleteQuadric(q);
    glPopMatrix();
}

// buat seluruh jet

void drawPlayer()
{
    static float prevPlayerX  = 0.0f;   // posisi frame sebelumnya
    static float gRollAngle   = 0.0f;   // sudut roll saat ini
 
    // Hitung seberapa cepat & ke mana jet bergerak
    float speed    = gPlayerX - prevPlayerX;   // positif = gerak kanan
    prevPlayerX    = gPlayerX;
 
    // Target: miring saat bergerak, tegak (0) saat diam
    float targetRoll = -speed * 120.0f;

    gRollAngle += (targetRoll - gRollAngle) * 0.12f;

    glPushMatrix();

    // Posisi horizontal & sedikit ke depan layar
    glTranslatef(gPlayerX, 0.0f, 1.8f);

    // Hadapkan jet maju ke depan (sumbu arah kamera)
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

    // Roll saat manuver
    glRotatef(gRollAngle, 0.0f, 0.0f, 1.0f);

    // warna abu-abu gelap 
    glColor3f(0.22f, 0.23f, 0.24f);

    // Badan tengah 
    // depan lebih lancip
    badan(0.22f, 0.38f, 0.07f, -0.65f, -0.10f);
    // tengah paling lebar
    badan(0.38f, 0.42f, 0.09f, -0.10f,  0.30f);
    // belakang melebar ke trailing edge
    glColor3f(0.20f, 0.21f, 0.22f); // mengubah warna menjadi abu-abu gelap
    badan(0.42f, 0.50f, 0.08f,  0.30f,  0.65f);

    // Sayap kanan
    glColor3f(0.20f, 0.21f, 0.22f);
    glPushMatrix();
    glTranslatef(0.42f, 0.0f, 0.0f);   // sambung dari tepi badan
    sayap(1.05f, -0.65f, 0.65f, 0.05f, 0.55f, 0.045f); 
    glPopMatrix();

    // -- Sayap kiri mirror X
    glPushMatrix();
    glTranslatef(-0.42f, 0.0f, 0.0f);
    glScalef(-1.0f, 1.0f, 1.0f);
    sayap(1.05f, -0.65f, 0.65f, 0.05f, 0.55f, 0.045f);
    glPopMatrix();

    // cockpit
    cockpit();

    // Engine
    nozzle( 0.18f);   
    nozzle(-0.18f);   

    // cahaya exhaust kanan
    glPushMatrix();
    glTranslatef( 0.18f, -0.05f, 0.78f);
    glutSolidSphere(0.045f, 8, 6);
    glPopMatrix();
    
    // cahaya exhaust kiri
    glPushMatrix();
    glTranslatef(-0.18f, -0.05f, 0.78f);
    glutSolidSphere(0.045f, 8, 6);
    glPopMatrix();

    glPopMatrix();
}
