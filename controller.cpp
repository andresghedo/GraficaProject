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

#include <vector> 
#include "controller.h"
#include "point3.h"
#include "mesh.h"
#include "car.h"

Mesh target((char *) "./obj/birillo.obj");
Point3 targetPoint = Point3(0, 0, -20);
bool generate;
float X_TARGET_MAX = +2.0;
float X_TARGET_MIN = -2.0;
float Z_TARGET_MAX = -5.0;
float Z_TARGET_MIN = -25.0;

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

void Controller::drawBirillo(float carX, float carZ) {
//    targetPoint.setX(carX - 2.0);
//    targetPoint.setZ(carZ - 5);
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glScalef(1.0, 1.0, 1.0);//0.75 1 0.75
    glTranslatef(targetPoint.X(), targetPoint.Y(), targetPoint.Z());
    target.RenderNxV();
    glPopMatrix();
}

void Controller::checkVisibilityTarget(float carX, float carY, float carZ) {
    if((targetPoint.Z() - carZ) > 5) {
        generate = true;
        //printf("[DEBUG] NEWWWW!! TARGET Z: %f  |  CAR Z: %f\n", targetPoint.Z(), carZ);
        float targetX = (X_TARGET_MAX - X_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + X_TARGET_MIN ;
        float targetZ = (Z_TARGET_MAX - Z_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Z_TARGET_MIN ;
        //printf("[DEBUG] NEW TARGET X: %f Z: %f  |  CAR X: %f Z: %f\n", targetX, targetZ, carX, carZ);
        targetPoint.setX(targetX);
        targetPoint.setZ(carZ + targetZ);
    }
}

void Controller::drawTargetCube(float mozzo) {
    /* disegno del primo cubo */
    float targetX, targetZ;
    const float Xmin = -0.25, Xmax = 0.25;
    const float Ymin = 0.2, Ymax = 0.7;
    const float Zmin = -0.25, Zmax = 0.25;

    // se devo rigenerare il cubo
    if(generate) {
        targetX = (X_TARGET_MAX - X_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + X_TARGET_MIN ;
        targetZ = (Z_TARGET_MAX - Z_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Z_TARGET_MIN ;
        generate = false;
    }

    // disegno del cubo con una texture personale su tutti e sei i lati
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 3);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_GEN_MODE, GL_REPLACE );

    glTranslatef(targetPoint.X(), targetPoint.Y(), targetPoint.Z());
//  glTranslatef(2, 2, 2);
//    glRotatef(90, 1, 1, 0);
//  glTranslatef(-2, -2, -2);
    glColor3f(1,1,1);
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    /* Front. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Xmin, Ymin, Zmax);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Xmax, Ymin, Zmax);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Xmax, Ymax, Zmax);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Xmin, Ymax, Zmax);

    /* Down. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Xmin, Ymin, Zmin);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Xmax, Ymin, Zmin);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Xmax, Ymin, Zmax);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Xmin, Ymin, Zmax);

    /* Back. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Xmin, Ymax, Zmin);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Xmax, Ymax, Zmin);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Xmax, Ymin, Zmin);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Xmin, Ymin, Zmin);

    /* Up. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Xmin, Ymax, Zmax);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Xmax, Ymax, Zmax);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Xmax, Ymax, Zmin);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Xmin, Ymax, Zmin);

    /* SideLeft. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Xmin, Ymax, Zmin);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Xmin, Ymax, Zmax);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Xmin, Ymin, Zmax);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Xmin, Ymin, Zmin);

    /* SideRight. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Xmax, Ymax, Zmin);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Xmax, Ymax, Zmax);  
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Xmax, Ymin, Zmax);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Xmax, Ymin, Zmin);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glLineWidth(1);
    glColor3f(0,0,0);
    glBegin(GL_LINE_LOOP);
      glVertex3f(Xmin, Ymin, Zmax);
      glVertex3f(Xmax, Ymin, Zmax);
      glVertex3f(Xmax, Ymax, Zmax);
      glVertex3f(Xmin, Ymax, Zmax);
    glEnd();

    glBegin(GL_LINE_LOOP);
      glVertex3f(Xmin, Ymax, Zmin);
      glVertex3f(Xmax, Ymax, Zmin);  
      glVertex3f(Xmax, Ymin, Zmin);
      glVertex3f(Xmin, Ymin, Zmin);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(Xmin, Ymin, Zmax);
    glVertex3f(Xmin, Ymin, Zmin);

    glVertex3f(Xmax, Ymin, Zmax);
    glVertex3f(Xmax, Ymin, Zmin);

    glVertex3f(Xmin, Ymax, Zmax);
    glVertex3f(Xmin, Ymax, Zmin);

    glVertex3f(Xmax, Ymax, Zmax);
    glVertex3f(Xmax, Ymax, Zmin);      
    glEnd();
//    // se l'aereo ha catturato il cubo
//    if (helix.px >= pos_x - 3 && helix.px <= pos_x + 3 &&
//        helix.py >= pos_y - 3 && helix.py <= pos_y + 3 &&
//        helix.pz >= pos_z - 4 && helix.pz <= pos_z + 4) {
//          punteggio++;
//          deveEssereCreato = true;
//          // printf("Punteggio: %d\n", punteggio);
//    }
    glPopMatrix();
}
