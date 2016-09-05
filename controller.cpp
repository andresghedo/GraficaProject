/* 
 * File:   controller.cpp
 * Author: andrea
 * 
 * Created on 5 settembre 2016, 15.47
 */

#include "controller.h"
#include <stdio.h>
#include <math.h> 

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector> 

#include "point3.h"
#include "mesh.h"

Mesh target((char *) "./obj/birillo.obj");

void Controller::Init() {
    for (int i = 0; i < NKEYS; i++) key[i] = false;
}

void Controller::EatKey(int keycode, int* keymap, bool pressed_or_released) {
    printf("[DEBUG]Premuto un tasto...\n");
    for (int i = 0; i < NKEYS; i++) {
        if (keycode == keymap[i]) key[i] = pressed_or_released;
    }
}

// da invocare quando e' stato premuto/rilasciato un jbutton

void Controller::Joy(int keymap, bool pressed_or_released) {
    key[keymap] = pressed_or_released;
}

void Controller::drawTarget() {
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0);
    glScalef(1.0, 1.0, 1.0);//0.75 1 0.75
    glTranslatef(1, 0, -20);// se la alzo la macchina si immerge nella pista X Y Z
    target.RenderNxV();
    //pista.RenderNxF();
    glPopMatrix();
}