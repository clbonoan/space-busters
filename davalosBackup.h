#ifndef DAVALOS_H
#define DAVALOS_H

#include <GL/gl.h>

const int MAX_ENEMIES = 30;

struct Enemy {
    float x, y;
    float vx, vy;
    float ax, ay;
    float speed;
    bool active;
};

//enemy armies
extern Enemy zorpArmy[MAX_ENEMIES];
extern Enemy wiblobArmy[MAX_ENEMIES];

// texture handles
extern GLuint zorpTexture;
extern GLuint wiblobTexture;

//boss
void moveBossesTowardPlayer();
void renderBosses();


// function declarations
void initEnemies();
void spawnEnemy();
void moveEnemiesTowardPlayer();
void renderEnemies();
void updateEnemySpawnTimer();
void hitEnemy(float x, float y);

//animation intro
extern void drawIntro();
extern bool introDone;
extern void drawNebulaBackground();

//score
extern void drawScore();

#endif
