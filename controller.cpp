/* 
 * File:   controller.cpp
 * Author: andrea
 * 
 * Created on 5 settembre 2016, 15.47
 */

#include <stdio.h>
#include <math.h> 
#include <time.h>
#include <cstdlib>

#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <vector> 
#include "controller.h"
#include "point3.h"
#include "mesh.h"
#include "car.h"
#include "constants.h"

Point3 targetPoint = Point3(0, 0, Constant::INITIAL_TARGET_Z);
Point3 triangleTopPoint = Point3(0, 0, 0);
bool generate;
bool isTnt = false;
bool isGoal = true;
bool gameOver = false;
bool playerLoose = false;
int punteggio = 0;
int tntChecked = 0;
int tnt = 0;
int goalChecked = 0;
int goal = 0;
clock_t startTime;
double endTime = 0;

void Controller::Init() {
    srand(time(NULL));
    startTime = clock();
    for (int i = 0; i < NKEYS; i++) key[i] = false;
}

double Controller::getSeconds() {
    clock_t currentTime = clock();
    clock_t clockTicksTaken = currentTime - startTime;
    return clockTicksTaken / (double) CLOCKS_PER_SEC;
}

void Controller::EatKey(int keycode, int* keymap, bool pressed_or_released) {
    for (int i = 0; i < NKEYS; i++) {
        if (keycode == keymap[i]) key[i] = pressed_or_released;
    }
}

// da invocare quando e' stato premuto/rilasciato un jbutton

void Controller::Joy(int keymap, bool pressed_or_released) {
    key[keymap] = pressed_or_released;
}

bool isInTarget() {
//    printf("TARGET X:%f Z:%f\n", targetPoint.X(), targetPoint.Z());
    return ((triangleTopPoint.X() >= targetPoint.X() - Constant::DIM_CUBE) && (triangleTopPoint.X() <= targetPoint.X() + Constant::DIM_CUBE))&&((triangleTopPoint.Z() >= targetPoint.Z() - Constant::DIM_CUBE) && (triangleTopPoint.Z() <= targetPoint.Z() + Constant::DIM_CUBE));
}

bool generateTarget(float carX, float carZ) {

    float targetX, targetZ, newX, newZ;
    if ((((float) rand()) / (float) RAND_MAX) > Constant::PROBABILITY_OF_GOAL) { // genero un TNT
        isTnt = true;
        isGoal = false;
        tnt += 1;
        targetX = (Constant::X_POS_TNT_MAX - Constant::X_POS_TNT_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::X_POS_TNT_MIN;
        targetZ = (Constant::Z_POS_TNT_MAX - Constant::Z_POS_TNT_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::Z_POS_TNT_MIN;
        newX = carX + targetX;
        newZ = carZ + targetZ;
    } else { // genero un GOAL
        isTnt = false;
        isGoal = true;
        goal += 1;
        targetX = (Constant::X_POS_GOAL_MAX - Constant::X_POS_GOAL_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::X_POS_GOAL_MIN;
        targetZ = (Constant::Z_POS_GOAL_MAX - Constant::Z_POS_GOAL_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::Z_POS_GOAL_MIN;
        newX = targetX;
        newZ = carZ + targetZ;
    }
    if (carZ + targetZ < -425) {
        targetPoint.setX(-900);
        targetPoint.setZ(-900);
    } else {
        targetPoint.setX(newX);
        targetPoint.setZ(newZ);
    }
}

void Controller::checkVisibilityTarget(float carX, float carZ) {
    if ((targetPoint.Z() - carZ) > Constant::LIMIT_VISIBILITY_TARGET) {
        generate = true;
        if (isGoal)
            punteggio -= 1;
    } else if (isInTarget() && isGoal) {
        goalChecked += 1;
        punteggio += 1;
        generate = true;
    } else if (isInTarget() && isTnt) {
        tntChecked += 1;
        punteggio -= 1;
        generate = true;
    } else {
        generate = false;
    }

    if (generate) {
        generateTarget(triangleTopPoint.X(), triangleTopPoint.Z());
    }
    if ((triangleTopPoint.Z() < -430) && ((triangleTopPoint.X() > -5.0) && (triangleTopPoint.X() < 5.0))) {
        gameOver = true;
        endTime = Controller::getSeconds();
    }
    if (Constant::GAME_LIMIT_SECONDS - Controller::getSeconds() <= 0) {
        gameOver = true;
        playerLoose = true;
    }
    //printf("PUNTEGGIO: %d\n", punteggio);
}

void Controller::drawTargetCube(float mozzo) {

    // disegno del cubo con una texture personale su tutti e sei i lati
    glPushMatrix();
    if (isTnt)
        glBindTexture(GL_TEXTURE_2D, 3);
    else
        glBindTexture(GL_TEXTURE_2D, 6);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_GEN_MODE, GL_REPLACE);

    glTranslatef(targetPoint.X(), targetPoint.Y(), targetPoint.Z());
    glRotatef(90, 0, 1, 0);
    glColor3f(1, 1, 1);
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    /* Front. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);

    /* Down. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);

    /* Back. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);

    /* Up. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);

    /* SideLeft. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);

    /* SideRight. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glLineWidth(1);
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);

    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MIN_TARGET, Constant::DIM_Z_MIN_TARGET);

    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MIN_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);

    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MAX_TARGET);
    glVertex3f(Constant::DIM_X_MAX_TARGET, Constant::DIM_Y_MAX_TARGET, Constant::DIM_Z_MIN_TARGET);
    glEnd();
    glPopMatrix();
}

void drawLineToDebug(float x1, float y1, float z1, float x2, float y2, float z2, float colorR, float colorG, float colorB) {
    glColor3f(colorR, colorG, colorB); // GREEN
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
}

void Controller::drawTriangleForTarget(float facing, float carX, float carZ) {
    float angle = 360 - facing;
    float cosA = cos(angle * M_PI / 180.0);
    float sinA = sin(angle * M_PI / 180.0);
    float Z2B = 2.5; // distanza da baricentro Z
    float Z2BTop = 2.5; // distanza da baricentro Z
    float X2B = 0.1; // distanza da baricentro X
    float X2BTop = 0; // distanza da baricentro Z
    float y = 0.5;

    float X1 = X2B * cosA - Z2B * sinA;
    float Z1 = X2B * sinA + Z2B * cosA;

    float X2 = -X2B * cosA - Z2B * sinA;
    float Z2 = -X2B * sinA + Z2B * cosA;

    float X3 = -X2BTop * cosA - Z2BTop * sinA;
    float Z3 = -X2BTop * sinA + Z2BTop * cosA;

    float xA = carX - X1;
    float zA = carZ - Z1;

    float xB = carX - X2;
    float zB = carZ - Z2;

    //TOP DEL TRIANGOLO
    float xC = carX - X3;
    float zC = carZ - Z3;

    triangleTopPoint.setX(xC);
    triangleTopPoint.setZ(zC);
    //printf("MIRINO X: %f    Z: %f\n", triangleTopPoint.X(), triangleTopPoint.Z());

    //    drawLineToDebug(xA, +1, zA, xA, 0, zA);
    //    drawLineToDebug(xB, +1, zB, xB, 0, zB);
    //    drawLineToDebug(xC, +1, zC, xC, 0, zC);

    // X and +
    drawLineToDebug(xA, 0.6, zA, xB, 0.3, zB, 1, 0, 0); //x
    drawLineToDebug(xB, 0.6, zB, xA, 0.3, zA, 1, 0, 0); //x
    drawLineToDebug(xA, 0.45, zA, xB, 0.45, zB, 1, 0, 0); //+
    drawLineToDebug(xC, 0.3, zC, xC, 0.6, zC, 1, 0, 0); //+

    glEnd();
}

void Controller::drawLightTorciaStatua() {
    glEnable(GL_LIGHT4);
    GLfloat ambient[] = {1, 0.0, 0.0, 1.0};
    glLightfv(GL_LIGHT4, GL_AMBIENT, ambient);
    GLfloat diffuse[] = {0.4, 0, 0, 1.0};
    glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuse);
    //GLfloat specular[] = { 1.0, 0.0, 1.0, 0.0 };
    //glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    GLfloat position[] = {Constant::POS_TORCIA_X, Constant::POS_TORCIA_Y, Constant::POS_TORCIA_Z, 1}; // ultima comp=0 => luce direzionale
    glLightfv(GL_LIGHT4, GL_POSITION, position);
    GLfloat spot_direction[] = {0, 0, -1.0, 0};
    glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 1.0);
    glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 90.0);
    glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, 0);
    float attenuation = 1;
    if (SDL_GetTicks() % 3 == 0)
        attenuation = 0.02;
    glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, attenuation);
    //glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 12.0);
}

float Controller::getTargetX() {
    return targetPoint.X();
};

float Controller::getTargetZ() {
    return targetPoint.Z();
};

bool Controller::isTargetGoal() {
    return isGoal;
};

bool Controller::isTargetTnt() {
    return isTnt;
};

bool Controller::isGameOver() {
    //    return (Controller::getSeconds() > 4);
    return gameOver;
};

bool Controller::isPlayerLoose() {
    return playerLoose;
};

int Controller::getScore() {
    return punteggio;
};

int Controller::getGoal() {
    return goal;
};

int Controller::getGoalChecked() {
    return goalChecked;
};

int Controller::getTnt() {
    return tnt;
};

int Controller::getTntChecked() {
    return tntChecked;
};

void Controller::SDL_GL_DrawText(TTF_Font *font, // font
        char fgR, char fgG, char fgB, char fgA, // colore testo
        char bgR, char bgG, char bgB, char bgA, // colore background
        char *text, int x, int y) { // qualità del testo
    SDL_Color tmpfontcolor = {fgR, fgG, fgB, fgA};
    SDL_Color tmpfontbgcolor = {bgR, bgG, bgB, bgA};
    SDL_Surface *initial;
    SDL_Surface *intermediary;
    SDL_Rect location;
    int w, h;

    /* Usiamo SDL_TTF per il rendering del testo */
    initial = NULL;
    initial = TTF_RenderText_Shaded(font, text, tmpfontcolor, tmpfontbgcolor);


    /* Convertiamo il testo in un formato conosciuto */
    w = initial->w;
    h = initial->h;

    /* Allochiamo una nuova surface RGB */
    intermediary = SDL_CreateRGBSurface(0, w, h, 32,
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    /* Copiamo il contenuto dalla prima alla seconda surface */
    SDL_BlitSurface(initial, 0, intermediary, 0);

    /* Informiamo GL della nuova texture */
    glBindTexture(GL_TEXTURE_2D, -1);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, intermediary->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (initial != NULL) {
        location.x = x;
        location.y = y;
    }

    glLineWidth(2);
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(location.x - 2, location.y - 2);
    glVertex2f(location.x + w + 2, location.y - 2);
    glVertex2f(location.x + w + 2, location.y + h + 2);
    glVertex2f(location.x - 2, location.y + h + 2);
    glEnd();

    /* prepariamoci al rendering del testo */
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, -1);
    glColor3f(1.0f, 1.0f, 1.0f);

    /* Disegnamo un quads come location del testo */
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(location.x, location.y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(location.x + w, location.y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(location.x + w, location.y + h);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(location.x, location.y + h);
    glEnd();

    /* Disegnamo un contorno al quads */
    glColor3f(0.0f, 0.0f, 0.0f);
    //  glBegin(GL_LINE_STRIP);
    //    glVertex2f((GLfloat)location.x-1, (GLfloat)location.y-1);
    //    glVertex2f((GLfloat)location.x + w +1, (GLfloat)location.y-1);
    //    glVertex2f((GLfloat)location.x + w +1, (GLfloat)location.y + h +1);
    //    glVertex2f((GLfloat)location.x-1    , (GLfloat)location.y + h +1);
    //    glVertex2f((GLfloat)location.x-1, (GLfloat)location.y-1);
    //  glEnd();

    /* Bad things happen if we delete the texture before it finishes */
    glFinish();

    /* return the deltas in the unused w,h part of the rect */
    location.w = initial->w;
    location.h = initial->h;

    /* Clean up */
    glDisable(GL_TEXTURE_2D);
    SDL_FreeSurface(initial);
    SDL_FreeSurface(intermediary);

}

void Controller::drawGameOverLayout(SDL_Window *win, TTF_Font *font, int scrH, int scrW) {
    // settiamo il viewport
    glViewport(0, 0, scrW, scrH);

    // colore di sfondo (fuori dal mondo)
    glClearColor(0, 0, 0, 1);

    // riempe tutto lo screen buffer di pixel color sfondo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(2.0 / scrW, 2.0 / scrH, 1);

    glLineWidth(2);

    // conversione della variabile punteggio
    char punti[3];
    sprintf(punti, "%d", punteggio);

    char stringa_punti[] = "Punteggio: ";
    char str_game_over[] = "GAME OVER!!";
    char continuare[] = "Premi un tasto\n per continuare";
    
    TTF_Font *fontTitle;
    fontTitle = TTF_OpenFont("./ttf/amatic.ttf", 100);
    if (font == NULL) {
        fprintf(stderr, "Impossibile caricare il font.\n");
    }
    
    char tnt[20];
    sprintf(tnt, "TNT:  %d / %d   ", Controller::getTntChecked(), Controller::getTnt());
    char goal[20];
    sprintf(goal, "GOAL:  %d / %d   ", Controller::getGoalChecked(), Controller::getGoal());
    char point[20];
    sprintf(point, "SCORE:  %d   ", Controller::getScore());
    char time[20];
    sprintf(time, "TIME:  %f ", endTime);

    Controller::SDL_GL_DrawText(fontTitle, 0, 0, 0, 0, 210, 210, 210, 255, str_game_over, scrW / 2 - 120, scrH  - 120);
    Controller::SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, point, 50, scrH / 2 + 100);
    Controller::SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, goal, 50, scrH / 2 + 30);
    Controller::SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, tnt, 50, scrH / 2 -40);
    Controller::SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, time, 50, scrH / 2 - 110);
    Controller::SDL_GL_DrawText(font, 0, 0, 0, 0, 210, 210, 210, 255, continuare, 50, scrH / 2 - 180);
    glFinish();

    SDL_GL_SwapWindow(win);
}