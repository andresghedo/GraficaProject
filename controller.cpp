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

#include <vector> 
#include "controller.h"
#include "point3.h"
#include "mesh.h"
#include "car.h"

Mesh recinzione((char *) "./obj/recinzione.obj");
Point3 targetPoint = Point3(0, 0, -20);
Point3 triangleTopPoint = Point3(0, 0, 0);
bool generate;
int punteggio = 0;
float X_POS_TARGET_MAX = +5.0;
float X_POS_TARGET_MIN = -5.0;
float Z_POS_TARGET_MAX = -35.0;
float Z_POS_TARGET_MIN = -35.0;
// CUBE TARGET DIMENSION
const float DIM_CUBE = 0.8;
const float DIM_X_MIN_TARGET = -DIM_CUBE, DIM_X_MAX_TARGET = DIM_CUBE;
const float DIM_Y_MIN_TARGET = 0.15, DIM_Y_MAX_TARGET = 1.75;
const float DIM_Z_MIN_TARGET = -DIM_CUBE, DIM_Z_MAX_TARGET = DIM_CUBE;

void Controller::Init() {
    srand(time(NULL));
    for (int i = 0; i < NKEYS; i++) key[i] = false;
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

void Controller::checkVisibilityTarget(float carX, float carY, float carZ) {
    if((targetPoint.Z() - carZ) > 5) {
        generate = true;
        punteggio -=1;
    }else if(((triangleTopPoint.X() >= targetPoint.X()-DIM_CUBE) && (triangleTopPoint.X() <= targetPoint.X()+DIM_CUBE))&&((triangleTopPoint.Z() >= targetPoint.Z()-DIM_CUBE) && (triangleTopPoint.Z() <= targetPoint.Z()+DIM_CUBE))) {
        punteggio += 1;
        generate = true;
    } else {
        generate = false;
    }

    if(generate) {
        float targetX = (X_POS_TARGET_MAX - X_POS_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + X_POS_TARGET_MIN ;
        float targetZ = (Z_POS_TARGET_MAX - Z_POS_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Z_POS_TARGET_MIN ;
        targetPoint.setX(targetX);
        targetPoint.setZ(carZ + targetZ);
    }
    printf("PUNTEGGIO: %d\n", punteggio);
}

void Controller::drawTargetCube(float mozzo) {

    // disegno del cubo con una texture personale su tutti e sei i lati
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 3);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_GEN_MODE, GL_REPLACE );

    glTranslatef(targetPoint.X(), targetPoint.Y(), targetPoint.Z());
    glRotatef(90, 0, 1, 0);
    glColor3f(1,1,1);
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    /* Front. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);

    /* Down. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);

    /* Back. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);

    /* Up. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);

    /* SideLeft. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);

    /* SideRight. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glLineWidth(1);
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
      glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
      glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
      glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
      glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
    glEnd();

    glBegin(GL_LINE_LOOP);
      glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
      glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
      glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);
      glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);

    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MAX_TARGET);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MIN_TARGET, DIM_Z_MIN_TARGET);

    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
    glVertex3f(DIM_X_MIN_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);

    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MAX_TARGET);
    glVertex3f(DIM_X_MAX_TARGET, DIM_Y_MAX_TARGET, DIM_Z_MIN_TARGET);
    glEnd();
    glPopMatrix();
}

void drawLineToDebug(float x1, float y1, float z1, float x2, float y2, float z2) {
    glColor3f(0, 0, 1);// GREEN
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
}

void Controller::drawTriangleForTarget(float facing, float carX, float carZ) {
    float angle = 360 - facing;
    float cosA = cos(angle*M_PI/180.0);
    float sinA = sin(angle*M_PI/180.0);
    float Z2B = 1.0; // distanza da baricentro Z
    float Z2BTop = 2.5; // distanza da baricentro Z
    float X2B = 0.5; // distanza da baricentro X
    float X2BTop = 0; // distanza da baricentro Z
    float y = 0.5;

    float X1  = X2B * cosA - Z2B * sinA;
    float Z1  = X2B * sinA + Z2B * cosA;

    float X2  = -X2B * cosA - Z2B * sinA;
    float Z2  = -X2B * sinA + Z2B * cosA;

    float X3  = -X2BTop * cosA - Z2BTop * sinA;
    float Z3  = -X2BTop * sinA + Z2BTop * cosA;

    float xA = carX-X1;
    float zA = carZ-Z1;

    float xB = carX-X2;
    float zB = carZ-Z2;

    //TOP DEL TRIANGOLO
    float xC = carX-X3;
    float zC = carZ-Z3;

    triangleTopPoint.setX(xC);
    triangleTopPoint.setZ(zC);

    drawLineToDebug(xA, +1, zA, xA, -1, zA);
    drawLineToDebug(xB, +1, zB, xB, -1, zB);
    drawLineToDebug(xC, +1, zC, xC, -1, zC);

    glBegin(GL_TRIANGLES);
    glColor3f(0.1, 0.2, 0.3);
    glVertex3f(xA, y, zA);
    glVertex3f(xB, y, zB);
    glVertex3f(xC, y, zC);
    glEnd();
}

void Controller::drawLightTorciaStatua() {
    glEnable(GL_LIGHT4);
    GLfloat ambient[] = { 1, 0.0, 0.0, 1.0 };
    glLightfv(GL_LIGHT4, GL_AMBIENT, ambient);
    GLfloat diffuse[] = { 0.4, 0, 0, 1.0 };
    glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuse);
    //GLfloat specular[] = { 1.0, 0.0, 1.0, 0.0 };
    //glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    GLfloat position[] = {0.0, 70, -440, 1}; // ultima comp=0 => luce direzionale
    glLightfv(GL_LIGHT4, GL_POSITION, position);
    GLfloat spot_direction[] = { 0, 0, -1.0, 0 };
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

void Controller::drawRecinzione() {
    
    glBindTexture(GL_TEXTURE_2D, 6);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Env map
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glColor3f(1, 1, 1);
   
    glPushMatrix();
    glDisable(GL_LIGHTING); // disabilitato le luci
    glColor3f(1, 1, 1);
    glScalef(0.5, 0.5, 0.5);
    glRotatef(90, 0, 1, 0);
    glTranslatef(0, 0, 40);
    recinzione.RenderNxV();
    for(int i=0; i<=400; i++){ 
        glTranslatef(4.5, 0, 0);
        recinzione.RenderNxV();
    }
    glPopMatrix();
    //glEnable(GL_LIGHTING); // abilito le luci
    
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}