/*
 *  CLASSE Car
 * 
 *  Classe che implementa metodi relativi alla macchina
 * 
 */
#include <stdio.h>
#include <math.h>
#include <iostream> 
#include <stdlib.h> 
#include <math.h> 
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include "car.h"
#include "point3.h"
#include "mesh.h"
#include "constants.h"

/* variabili globali di tipo MESH */
Mesh carlinga((char *) "./obj/Ferrari_chassis.obj"); // chiama il costruttore
Mesh wheelBR1((char *) "./obj/Ferrari_wheel_back_R.obj");
Mesh wheelFR1((char *) "./obj/Ferrari_wheel_front_R.obj");
Mesh wheelBR2((char *) "./obj/Ferrari_wheel_back_R_metal.obj");
Mesh wheelFR2((char *) "./obj/Ferrari_wheel_front_R_metal.obj");
Mesh striscia((char *) "./obj/street_line.obj");
Mesh statua((char *) "./obj/statua_corpo.obj");
Mesh fuocoStatua((char *) "./obj/statua_fuoco.obj");

/* variabile che indica se utilizzare le texture o colori RGB */
extern bool useEnvmap;
/* variabile che indica se utilizzare i fari o meno */
extern bool useHeadlight;
/* variabile che indica se utilizzare l'ombra dell'auto o meno */
extern bool useShadow;

/* texturizzazione della carrozzeria */
void SetupEnvmapTexture() {
    glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_CARROZZERIA);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glColor3f(1, 1, 1);
}

/* texturizzazione delle gomme dell'auto */
void SetupWheelTexture(Point3 min, Point3 max) {
    glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_LOGO_GOMME);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    /* ulilizzo le coordinate OGGETTO cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
       in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso */
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    float sz = 1.0 / (max.Z() - min.Z());
    float ty = 1.0 / (max.Y() - min.Y());
    float s[4] = {0, 0, sz, -min.Z() * sz};
    float t[4] = {0, ty, 0, -min.Y() * ty};
    glTexGenfv(GL_S, GL_OBJECT_PLANE, s);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, t);
}

/* compio un passo di fisica della macchina */
void Car::DoStep(bool LeftKey, bool RightKey, bool AccKey, bool DecKey) {

    float vxm, vym, vzm;
    float cosf = cos(facing * M_PI / 180.0);
    float sinf = sin(facing * M_PI / 180.0);
    vxm = +cosf * vx - sinf*vz;
    vym = vy;
    vzm = +sinf * vx + cosf*vz;

    /* gestione dello sterzo */
    if (LeftKey) sterzo += velSterzo;
    if (RightKey) sterzo -= velSterzo;
    /* ritorno al volante dritto */
    sterzo *= velRitornoSterzo;

    /* accelerazione in avanti */
    if (AccKey) vzm -= accMax;
    /* accelerazione indietro */
    if (DecKey) vzm += accMax;

    /* attriti */
    vxm *= attritoX;
    vym *= attritoY;
    vzm *= attritoZ;

    /* l'orientamento della macchina segue quello dello sterzo(a seconda della velocita' sulla z) */
    facing = facing - (vzm * grip) * sterzo;

    /* rotazione mozzo ruote (a seconda della velocita' sulla z) */
    /* delta angolo */
    float da;
    da = (360.0 * vzm) / (2.0 * M_PI * raggioRuotaA);
    mozzoA += da;
    da = (360.0 * vzm) / (2.0 * M_PI * raggioRuotaP);
    mozzoP += da;

    /* ritorno a vel coord mondo */
    vx = +cosf * vxm + sinf*vzm;
    vy = vym;
    vz = -sinf * vxm + cosf*vzm;

    /* posizione = posizione + velocita * delta t (ma delta t e' costante) */
    px += vx;
    py += vy;
    pz += vz;

    /* metto dei limiti di posizione all'auto */
    if (pz > Constant::CAR_LIMIT_Z_SUP)
        pz = Constant::CAR_LIMIT_Z_SUP;
    if (pz < Constant::CAR_LIMIT_Z_INF)
        pz = Constant::CAR_LIMIT_Z_INF;
    if (px > Constant::CAR_LIMIT_X)
        px = Constant::CAR_LIMIT_X;
    if (px < -Constant::CAR_LIMIT_X)
        px = -Constant::CAR_LIMIT_X;
}

/* disegno la statua */
void drawStatua() {
    glPushMatrix();
    glColor3f(0.4, 0.6, 0.5);
    glTranslatef(2.5, -1.0, -450);
    glScalef(Constant::STATUE_SCALE_PARAMETR, Constant::STATUE_SCALE_PARAMETR, Constant::STATUE_SCALE_PARAMETR);
    statua.RenderNxV();
    /* colore rosso e disegno la fiaccola */
    glColor3f(0.8, 0.0, 0.0);
    fuocoStatua.RenderNxV();
    glPopMatrix();
}

/* disegno la linea stradale continua SX */
void drawExtremeSX() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glScalef(0.1, 1, Constant::START_Z_STREET_LINES);
    glTranslatef(Constant::DISTANCE_LINES, 0.01, 0);
    striscia.RenderNxV();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

/* disegno la linea stradale continua DX */
void drawExtremeDX() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glScalef(0.1, 1, Constant::START_Z_STREET_LINES);
    glTranslatef(-Constant::DISTANCE_LINES, 0.01, 0);
    striscia.RenderNxV();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

/* disegno la linea centrale tratteggiata */
void drawMiddleLine() {
    float posZ = 0;
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 300; i++) {
        float z = Constant::START_Z_STREET_LINES / 2.5 + posZ;
        float limitInf = -430 / 2.5;

        if (z > limitInf) {
            glPushMatrix();
            glColor3f(1, 1, 1);
            glScalef(0.1, 1, 2.5);
            glTranslatef(0, 0.01, z);
            striscia.RenderNxV();
            glPopMatrix();

            posZ -= 3.0;
        }
    }
    glEnable(GL_LIGHTING);
}

/* inizializzazione dell'oggetto Car */
void Car::Init() {
    /* inizializzo lo stato della macchina */
    px = Constant::INITIAL_CAR_X;
    pz = Constant::INITIAL_CAR_Z;
    facing = 0;
    py = 0.0;
    /* stato */
    mozzoA = mozzoP = sterzo = 0;
    /* velocità attuale */
    vx = vy = vz = 0;

    velSterzo = 2.4;
    velRitornoSterzo = 0.93;

    accMax = 0.0011;

    /* attriti: percentuale di velocita' che viene mantenuta
     1 = no attrito
     <<1 = attrito grande */
    /* piccolo attrito sulla Z (nel senso di rotolamento delle ruote) */
    attritoZ = 0.991;
    /* grande attrito sulla X (per non fare slittare la macchina) */
    attritoX = 0.8;
    /* attrito sulla y nullo */
    attritoY = 1.0;

    /* Nota: vel max = accMax*attritoZ / (1-attritoZ) */
    raggioRuotaA = 0.25;
    raggioRuotaP = 0.35;

    /* quanto il facing macchina si adegua velocemente allo sterzo */
    grip = 0.45;
}

/* attivazione luce come faro della macchina */
void Car::DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const {
    int usedLight = GL_LIGHT1 + lightN;

    if (useHeadlight) {
        glEnable(usedLight);

        float col0[4] = {0.8, 0.8, 0.0, 1};
        glLightfv(usedLight, GL_DIFFUSE, col0);

        float col1[4] = {0.5, 0.5, 0.0, 1};
        glLightfv(usedLight, GL_AMBIENT, col1);

        float tmpPos[4] = {x, y, z, 1};
        glLightfv(usedLight, GL_POSITION, tmpPos);

        float tmpDir[4] = {0, 0, -1, 0};
        glLightfv(usedLight, GL_SPOT_DIRECTION, tmpDir);

        glLightf(usedLight, GL_SPOT_CUTOFF, 30);
        glLightf(usedLight, GL_SPOT_EXPONENT, 5);

        glLightf(usedLight, GL_CONSTANT_ATTENUATION, 0);
        glLightf(usedLight, GL_LINEAR_ATTENUATION, 1);
    } else
        glDisable(usedLight);
}

/* funzione che disegna tutti i pezzi della macchina
   (carlinga, + 4 route)
   (da invocarsi due volte: per la macchina, e per la sua ombra)
   (se usecolor e' falso, NON sovrascrive il colore corrente
   e usa quello stabilito prima di chiamare la funzione) */
void Car::RenderAllParts(bool usecolor) const {

    /* disegna la carliga con una mesh */
    glPushMatrix();
    glScalef(-0.05, 0.05, -0.05);
    if (!useEnvmap) {
        /* colore rosso se non uso texture */
        if (usecolor) glColor3f(1, 0, 0);
    } else {
        /* metto la texture alla carrozzeria */
        if (usecolor) SetupEnvmapTexture();
    }
    carlinga.RenderNxV();
    if (!usecolor) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }

    if (usecolor) glEnable(GL_LIGHTING);
    /* ciclo per disegno di ruote e gomme */
    for (int i = 0; i < 2; i++) {
        // i==0 -> disegno ruote destre.
        // i==1 -> disegno ruote sinistre.
        int sign;
        if (i == 0) sign = 1;
        else sign = -1;
        glPushMatrix();
        if (i == 1) {
            glTranslatef(0, +wheelFR1.Center().Y(), 0);
            glRotatef(180, 0, 0, 1);
            glTranslatef(0, -wheelFR1.Center().Y(), 0);
        }

        glTranslate(wheelFR1.Center());
        glRotatef(sign*sterzo, 0, 1, 0);
        glRotatef(-sign*mozzoA, 1, 0, 0);
        glTranslate(-wheelFR1.Center());

        if (usecolor) glColor3f(.6, .6, .6);
        if (usecolor) SetupWheelTexture(wheelFR1.bbmin, wheelFR1.bbmax);
        wheelFR1.RenderNxF(); // Gomma davanti
        // provare x credere
        glDisable(GL_TEXTURE_2D);
        if (usecolor) glColor3f(0.9, 0.9, 0.9);
        wheelFR2.RenderNxV(); // cerchione davanti
        glPopMatrix();

        glPushMatrix();
        if (i == 1) {
            glTranslatef(0, +wheelBR1.Center().Y(), 0);
            glRotatef(180, 0, 0, 1);
            glTranslatef(0, -wheelBR1.Center().Y(), 0);
        }

        glTranslate(wheelBR1.Center());
        glRotatef(-sign*mozzoA, 1, 0, 0);
        glTranslate(-wheelBR1.Center());

        if (usecolor) glColor3f(.6, .6, .6);
        if (usecolor) SetupWheelTexture(wheelBR1.bbmin, wheelBR1.bbmax);
        wheelBR1.RenderNxF(); //gomme dietro
        glDisable(GL_TEXTURE_2D);
        if (usecolor) glColor3f(0.9, 0.9, 0.9);
        wheelBR2.RenderNxV(); //cerchione dietro

        if (usecolor) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glPopMatrix();
    }// FINE DISEGNO RUOTE
    glPopMatrix();
}

/* renderizzo la macchina */
void Car::Render() const {
    /* sono nello spazio mondo */
    glPushMatrix();
    /* mi sposto nel mondo Car */
    glTranslatef(px, py, pz);
    glRotatef(facing, 0, 1, 0);
    /* accendi fari SX e DX */
    DrawHeadlight(-0.3, 0, -1, 0, useHeadlight);
    DrawHeadlight(+0.3, 0, -1, 1, useHeadlight);

    /* disegna tutto a parte l'ombra */
    RenderAllParts(true);

    /* se useShadow disegna l'ombra */
    if (useShadow) {
        glColor3f(0.1, 0.1, 0.1);
        /* alzo l'ombra di un epsilon per evitare z-fighting con il pavimento */
        glTranslatef(0, 0.03, 0);
        /* appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X  */
        glScalef(1.01, 0, 1.01);
        /* niente lighing per l'ombra */
        glDisable(GL_LIGHTING);
        /* disegno la macchina appiattita */
        RenderAllParts(false);

        glEnable(GL_LIGHTING);
    }
    glPopMatrix();

    glPopMatrix();
}
