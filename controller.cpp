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

void Controller::drawTarget(float carX, float carZ) {
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
        printf("[DEBUG] NEWWWW!! TARGET Z: %f  |  CAR Z: %f\n", targetPoint.Z(), carZ);
        float targetX = (X_TARGET_MAX - X_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + X_TARGET_MIN ;
        float targetZ = (Z_TARGET_MAX - Z_TARGET_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Z_TARGET_MIN ;
        printf("[DEBUG] NEW TARGET X: %f Z: %f  |  CAR X: %f Z: %f\n", targetX, targetZ, carX, carZ);
        targetPoint.setX(targetX);
        targetPoint.setZ(carZ + targetZ);
    }

}

