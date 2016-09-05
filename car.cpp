// car.cpp
// implementazione dei metodi definiti in car.h

#include <stdio.h>
#include <math.h>
#include <iostream> 
#include <stdlib.h> 
#include <math.h> 

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector> // la classe vector di STL 

#include "car.h"
#include "point3.h"
#include "mesh.h"

// var globale di tipo mesh
Mesh carlinga((char *) "./obj/Ferrari_chassis.obj"); // chiama il costruttore
Mesh wheelBR1((char *) "./obj/Ferrari_wheel_back_R.obj");
Mesh wheelFR1((char *) "./obj/Ferrari_wheel_front_R.obj");
Mesh wheelBR2((char *) "./obj/Ferrari_wheel_back_R_metal.obj");
Mesh wheelFR2((char *) "./obj/Ferrari_wheel_front_R_metal.obj");
Mesh pista((char *) "./obj/pista.obj");
Mesh poke((char *) "./obj/plane.obj");
int distanceLine = 25;

extern bool useEnvmap; // var globale esterna: per usare l'evnrionment mapping
extern bool useHeadlight; // var globale esterna: per usare i fari
extern bool useShadow; // var globale esterna: per generare l'ombra

// Funzione che prepara tutto per usare un env map

// TEXTURE MACCHINA CARROZZERIA
void SetupEnvmapTexture() {
    //printf("[DEBUG] Texture per l'ambiente...\n");
    // facciamo binding con la texture 1
    glBindTexture(GL_TEXTURE_2D, 1);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Env map
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    glColor3f(1, 1, 1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
    //glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
}

// funzione che prepara tutto per creare le coordinate texture (s,t) da (x,y,z)
// Mappo l'intervallo [ minY , maxY ] nell'intervallo delle T [0..1]
//     e l'intervallo [ minZ , maxZ ] nell'intervallo delle S [0..1]

// TEXTURE RUOTE 
void SetupWheelTexture(Point3 min, Point3 max) {
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    
    // ulilizzo le coordinate OGGETTO
    // cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
    // in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    float sz = 1.0 / (max.Z() - min.Z());
    float ty = 1.0 / (max.Y() - min.Y());
    float s[4] = {0, 0, sz, -min.Z() * sz};
    float t[4] = {0, ty, 0, -min.Y() * ty};
    glTexGenfv(GL_S, GL_OBJECT_PLANE, s);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, t);
}

// DoStep: facciamo un passo di fisica (a delta_t costante)
//
// Indipendente dal rendering.
//
// ricordiamoci che possiamo LEGGERE ma mai SCRIVERE

void Car::DoStep(bool LeftKey, bool RightKey, bool AccKey, bool DecKey) {
    // computiamo l'evolversi della macchina
    static int i = 5;

    float vxm, vym, vzm; // velocita' in spazio macchina

    // da vel frame mondo a vel frame macchina
    float cosf = cos(facing * M_PI / 180.0);
    float sinf = sin(facing * M_PI / 180.0);
    vxm = +cosf * vx - sinf*vz;
    vym = vy;
    vzm = +sinf * vx + cosf*vz;

    // gestione dello sterzo
    if (LeftKey) sterzo += velSterzo;
    if (RightKey) sterzo -= velSterzo;
    sterzo *= velRitornoSterzo; // ritorno a volante dritto

    if (AccKey) vzm -= accMax; // accelerazione in avanti 
    if (DecKey) vzm += accMax; // accelerazione indietro

    // attirti (semplificando)
    vxm *= attritoX;
    vym *= attritoY;
    vzm *= attritoZ;

    // l'orientamento della macchina segue quello dello sterzo
    // (a seconda della velocita' sulla z)
    facing = facing - (vzm * grip) * sterzo;

    // rotazione mozzo ruote (a seconda della velocita' sulla z)
    float da; //delta angolo
    da = (360.0 * vzm) / (2.0 * M_PI * raggioRuotaA);
    mozzoA += da;
    da = (360.0 * vzm) / (2.0 * M_PI * raggioRuotaP);
    mozzoP += da;

    // ritorno a vel coord mondo
    vx = +cosf * vxm + sinf*vzm;
    vy = vym;
    vz = -sinf * vxm + cosf*vzm;

    // posizione = posizione + velocita * delta t (ma delta t e' costante)
    px += vx;
    py += vy;
    pz += vz;
    //printf("[DEBUG] FISICA   PX: %f | PY: %f | PZ: %f \n", px, py, pz);
}

//void drawCube(); // questa e' definita altrove (quick hack)
void drawAxis(); // anche questa

void drawPista() {
    glPushMatrix();
    glColor3f(0.4, 0.4, .8);
    glScalef(0.1, 0.1, 0.1);//0.75 1 0.75
    glTranslatef(0, 3, 0);// se la alzo la macchina si immerge nella pista X Y Z
    pista.RenderNxV();
    //pista.RenderNxF();
    glPopMatrix();
}

void drawExtremeSX() {
    
    printf("[DEBUG] BBMAX X: %f  Y: %f  Z: %f \n",poke.bbmax.X(),poke.bbmax.Y(),poke.bbmax.Z());
    printf("[DEBUG] BBMIN X: %f  Y: %f  Z: %f \n",poke.bbmin.X(),poke.bbmin.Y(),poke.bbmin.Z());
    
    float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    glPushMatrix();
    glDisable(GL_LIGHTING); // disabilitato le luci
    glColor3f(1, 1, 1);
    glScalef(0.1, 1, 500);
    glTranslatef(distanceLine, 0.01, 0);// se la alzo la macchina si immerge nella poke X Y Z
    //glTranslatef(x, y, z);
    poke.RenderNxV();
    //poke.RenderNxF();
    glPopMatrix();
    glEnable(GL_LIGHTING); // abilito le luci
}

void drawExtremeDX() {
    
    printf("[DEBUG] BBMAX X: %f  Y: %f  Z: %f \n",poke.bbmax.X(),poke.bbmax.Y(),poke.bbmax.Z());
    printf("[DEBUG] BBMIN X: %f  Y: %f  Z: %f \n",poke.bbmin.X(),poke.bbmin.Y(),poke.bbmin.Z());
    
    float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    glPushMatrix();
    glDisable(GL_LIGHTING); // disabilitato le luci
    glColor3f(1, 1, 1);
    glScalef(0.1, 1, 500);
    glTranslatef(-distanceLine, 0.01, 0);// se la alzo la macchina si immerge nella poke X Y Z
    //glTranslatef(x, y, z);
    poke.RenderNxV();
    //poke.RenderNxF();
    glPopMatrix();
    glEnable(GL_LIGHTING); // abilito le luci
}

void Car::Init() {
    // inizializzo lo stato della macchina
    px = pz = facing = 0; // posizione e orientamento
    py = 0.0;

    mozzoA = mozzoP = sterzo = 0; // stato
    vx = vy = vz = 0; // velocita' attuale

    //velSterzo=3.4;         // A
    velSterzo = 2.4; // A
    velRitornoSterzo = 0.93; // B, sterzo massimo = A*B / (1-B)

    accMax = 0.0011;

    // attriti: percentuale di velocita' che viene mantenuta
    // 1 = no attrito
    // <<1 = attrito grande
    attritoZ = 0.991; // piccolo attrito sulla Z (nel senso di rotolamento delle ruote)
    attritoX = 0.8; // grande attrito sulla X (per non fare slittare la macchina)
    attritoY = 1.0; // attrito sulla y nullo

    // Nota: vel max = accMax*attritoZ / (1-attritoZ)

    raggioRuotaA = 0.25;
    raggioRuotaP = 0.35;

    grip = 0.45; // quanto il facing macchina si adegua velocemente allo sterzo
}

// attiva una luce di openGL per simulare un faro della macchina

void Car::DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const {
    int usedLight = GL_LIGHT1 + lightN;

    if (useHeadlight) {
        glEnable(usedLight);

        float col0[4] = {0.8, 0.8, 0.0, 1};
        glLightfv(usedLight, GL_DIFFUSE, col0);

        float col1[4] = {0.5, 0.5, 0.0, 1};
        glLightfv(usedLight, GL_AMBIENT, col1);

        float tmpPos[4] = {x, y, z, 1}; // ultima comp=1 => luce posizionale
        glLightfv(usedLight, GL_POSITION, tmpPos);

        float tmpDir[4] = {0, 0, -1, 0}; // ultima comp=1 => luce posizionale
        glLightfv(usedLight, GL_SPOT_DIRECTION, tmpDir);

        glLightf(usedLight, GL_SPOT_CUTOFF, 30);
        glLightf(usedLight, GL_SPOT_EXPONENT, 5);

        glLightf(usedLight, GL_CONSTANT_ATTENUATION, 0);
        glLightf(usedLight, GL_LINEAR_ATTENUATION, 1);
    } else
        glDisable(usedLight);
}


// funzione che disegna tutti i pezzi della macchina
// (carlinga, + 4 route)
// (da invocarsi due volte: per la macchina, e per la sua ombra)
// (se usecolor e' falso, NON sovrascrive il colore corrente
//  e usa quello stabilito prima di chiamare la funzione)

void Car::RenderAllParts(bool usecolor) const {

    // drawCarlinga(); // disegna la carliga con pochi parallelepidedi

    // disegna la carliga con una mesh
    glPushMatrix();
    glScalef(-0.05, 0.05, -0.05); // patch: riscaliamo la mesh di 1/10 
    if (!useEnvmap) {
        if (usecolor) glColor3f(1, 0, 0); // colore rosso, da usare con Lighting
    } else {
        if (usecolor) SetupEnvmapTexture(); // METTO LA TEXTURE DEL CORPO DELLA MACCHINA
    }
    carlinga.RenderNxV(); // rendering delle mesh carlinga usando normali per vertice, DISEGNA IL CORPO DELLA MACCHINA PROPRIO
    if (usecolor) glEnable(GL_LIGHTING);
    // QUESTO CICLO FOR DISEGNA LE RUOTE ...
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
        wheelFR1.RenderNxF(); // la ruota viene meglio FLAT SHADED - normali per faccia
        // provare x credere
        glDisable(GL_TEXTURE_2D);
        if (usecolor) glColor3f(0.9, 0.9, 0.9);
        wheelFR2.RenderNxV();
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
        wheelBR1.RenderNxF();
        glDisable(GL_TEXTURE_2D);
        if (usecolor) glColor3f(0.9, 0.9, 0.9);
        wheelBR2.RenderNxV();
        glPopMatrix();
    }// FINE DISEGNO RUOTE
    glPopMatrix();
}

// disegna a schermo

void Car::Render() const {
    // sono nello spazio mondo

    //drawAxis(); // disegno assi spazio mondo
    glPushMatrix();

    glTranslatef(px, py, pz);       // QUI GLI DAI LE NUOVE XYZ DELLA MACCHINA
    glRotatef(facing, 0, 1, 0);     // QUI RUOTA 

    // sono nello spazio MACCHINA
    //drawAxis(); // disegno assi spazio macchina

    DrawHeadlight(-0.3, 0, -1, 0, useHeadlight); // accendi faro sinistro
    DrawHeadlight(+0.3, 0, -1, 1, useHeadlight); // accendi faro destro

    RenderAllParts(true);// DISEGNA TUTTA LA MACCHINA TRANNE L'OMBRA

    // DISEGNA L'OMBRA VERDE!
    if (useShadow) {
        glColor3f(0, 1, 0); // OMBRA VERDE
        glTranslatef(0, 0.03, 0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
        glScalef(1.01, 0, 1.01); // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
        glDisable(GL_LIGHTING); // niente lighing per l'ombra
        RenderAllParts(false); // disegno la macchina appiattita

        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
}
