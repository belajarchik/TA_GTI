#ifndef OBSTACLE_H
#define OBSTACLE_H

#ifdef _WIN32
#include <windows.h>
#endif
// #include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>

/* PAKSA DECLARE */
extern "C"
{
    int rand(void);
    void srand(unsigned int seed);
}

// max dari obstacle
#define MAX_OBS 10

// Ukuran shadow
#define SHADOW_W 0.8f  // setengah lebar shadow
#define SHADOW_H 1.2f  // setengah panjang shadow
#define SHADOW_Y 0.01f // sedikit di atas lantai

// obstacle
typedef struct
{
    float x, z;
    int active;
} Obs;

extern float LANES[3];
extern Obs gObs[MAX_OBS];
extern float gSpawnTimer;
extern float gSpawnInterval;
extern float gLastTime;
extern float gDeltaTime;
extern int gFrame;

void drawBox(float w, float h, float d);
void drawChunk(float w, float h, float d, float rx, float ry, float rz, float angle);

void drawAsteroid(float x, float z, int id);
void drawObstacle(float x, float z, int id);

void spawnObs();
void drawObsShadow(float x, float z, int id);

#endif