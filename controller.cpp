/*
 *  CLASSE Controller
 * 
 *  Classe che implementa metodi utili in tutto il progetto richiamabili dal main.gcc
 * 
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

/* posizione XYZ del target corrente(sia esso un goal o un TNT) */
Point3 targetPoint = Point3(0, 0, Constant::INITIAL_TARGET_Z);
/* posizione XYZ del mirino */
Point3 mirinoPoint = Point3(0, 0.4, 0);
/* booleano che decreta la generazione di un target o meno */
bool generate;
/* booleano che decreta se il target è un TNT */
bool isTnt = false;
/* booleano che decreta se il target è un Goal */
bool isGoal = true;
/* booleano che decreta la fine del gioco */
bool gameOver = false;
/* booleano che decreta se il player ha perso o meno */
bool playerLoose = false;
/* punteggio corrente dell'utente */
int punteggio = 0;
/* TNT fatti esplodere */
int tntChecked = 0;
/* TNT generati */
int tnt = 0;
/* Goal catturati */
int goalChecked = 0;
/* Goal generati */
int goal = 0;
/* tempo di inizio gioco */
clock_t startTime;
/* tempo di concludione del gioco */
double endTime = 0;

/* inizializzaione del Controller */
void Controller::Init() {
    srand(time(NULL));
    startTime = clock();
    for (int i = 0; i < NKEYS; i++) key[i] = false;
}

/* torna il numero di secondi da quando è iniziato il gioco */
double Controller::getSeconds() {
    clock_t currentTime = clock();
    clock_t clockTicksTaken = currentTime - startTime;
    return clockTicksTaken / (double) CLOCKS_PER_SEC;
}

/* memorizzazione del tasto premuto dall'utente */
void Controller::EatKey(int keycode, int* keymap, bool pressed_or_released) {
    for (int i = 0; i < NKEYS; i++) {
        if (keycode == keymap[i]) key[i] = pressed_or_released;
    }
}

/* da invocare quando e' stato premuto/rilasciato un jbutton */
void Controller::Joy(int keymap, bool pressed_or_released) {
    key[keymap] = pressed_or_released;
}

/* metodo che sancisce se il mirino ha centrato il target(sia esso un goal o un tnt) */
bool isInTarget() {
    return ((mirinoPoint.X() >= targetPoint.X() - Constant::DIM_CUBE) && (mirinoPoint.X() <= targetPoint.X() + Constant::DIM_CUBE))&&((mirinoPoint.Z() >= targetPoint.Z() - Constant::DIM_CUBE) && (mirinoPoint.Z() <= targetPoint.Z() + Constant::DIM_CUBE));
}

/* genera un target e setta la nuova posizione. Con una certa probabilità sarà un goal piuttosto che un tnt */
bool generateTarget(float carX, float carZ) {

    float targetX, targetZ, newX, newZ;
    if ((((float) rand()) / (float) RAND_MAX) > Constant::PROBABILITY_OF_GOAL) { /* genero un TNT */
        isTnt = true;
        isGoal = false;
        tnt += 1;
        targetX = (Constant::X_POS_TNT_MAX - Constant::X_POS_TNT_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::X_POS_TNT_MIN;
        targetZ = (Constant::Z_POS_TNT_MAX - Constant::Z_POS_TNT_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::Z_POS_TNT_MIN;
        newX = carX + targetX;
        newZ = carZ + targetZ;
    } else { /* genero un GOAL */
        isTnt = false;
        isGoal = true;
        goal += 1;
        targetX = (Constant::X_POS_GOAL_MAX - Constant::X_POS_GOAL_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::X_POS_GOAL_MIN;
        targetZ = (Constant::Z_POS_GOAL_MAX - Constant::Z_POS_GOAL_MIN) * ((((float) rand()) / (float) RAND_MAX)) + Constant::Z_POS_GOAL_MIN;
        newX = targetX;
        newZ = carZ + targetZ;
    }
    /* se supero il limite posizione all "infinito" */
    if (carZ + targetZ < Constant::LIMIT_GENERATE_TARGET) {
        targetPoint.setX(-9000);
        targetPoint.setZ(-9000);
    } else { /* altrimenti setto le nuove posizioni */
        targetPoint.setX(newX);
        targetPoint.setZ(newZ);
    }
}

/* controllo i vincoli di gioco(punteggi, visibilità target, ecc ...) */
void Controller::checkConstraintsGame(float carZ) {
    /* se ho passato un target senza catturarlo del limite ne genero un altro e
       se era un goal tolgo un punto */
    if ((targetPoint.Z() - carZ) > Constant::LIMIT_VISIBILITY_TARGET) {
        generate = true;
        if (isGoal)
            punteggio -= 1;
    } else if (isInTarget() && isGoal) { /* se catturo un goal +1 */
        goalChecked += 1;
        punteggio += 1;
        generate = true;
    } else if (isInTarget() && isTnt) { /* se catturo un tnt -1 */
        tntChecked += 1;
        punteggio -= 1;
        generate = true;
    } else {
        generate = false;
    }

    /* genero un nuovo target */
    if (generate) {
        generateTarget(mirinoPoint.X(), mirinoPoint.Z());
    }
    /* decreto il GAME OVER se passo la scacchiera in prossimità della statua */
    if ((mirinoPoint.Z() < Constant::LIMIT_GENERATE_TARGET - 5) && ((mirinoPoint.X() > -5.0) && (mirinoPoint.X() < 5.0))) {
        gameOver = true;
        endTime = Controller::getSeconds();
    }
    /* se scade il tempo decreto la perdita del player */
    if (Constant::GAME_LIMIT_SECONDS - Controller::getSeconds() <= 0) {
        gameOver = true;
        playerLoose = true;
    }
}

void Controller::drawTargetCube() {

    // disegno del cubo con una texture personale su tutti e sei i lati
    glPushMatrix();
    if (isTnt)
        glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_TNT);
    else
        glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_SELFIE);
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

void drawReverseLightPolygon(float x, float y, float z, float facing, bool retroLight) {

    // disegno del cubo con una texture personale su tutti e sei i lati
    glPushMatrix();

    if(retroLight)
        glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_LIGHT_ON);
    else
        glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_LIGHT_OFF);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_GEN_MODE, GL_REPLACE);

    glTranslatef(x, y, z);
    glRotatef(facing, 0, 1, 0);
    glColor3ub(255,255,255);
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    /* Front. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);

    /* Down. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);

    /* Back. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    /* Up. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    /* SideLeft. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    /* SideRight. */
    glTexCoord2f(0.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glLineWidth(1);
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MIN_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MIN_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);

    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MAX_REVERSE_LIGHT);
    glVertex3f(Constant::DIM_X_MAX_REVERSE_LIGHT, Constant::DIM_Y_MAX_REVERSE_LIGHT, Constant::DIM_Z_MIN_REVERSE_LIGHT);
    glEnd();
    glPopMatrix();
}

/* disegna un linea tra due punti della view */
void drawLine(float x1, float y1, float z1, float x2, float y2, float z2, float colorR, float colorG, float colorB) {
    glColor3f(colorR, colorG, colorB);
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
}

/*
 Disegno del mirino così composto:

           1     2     3
             \   |   /
              \  |  / 
               \ | /
          4 ____\|/____ 5
                /|\
               / | \
              /  |  \
             /   |   \ 
           6     8     9
 */
void Controller::drawMirino(float facing, float carX, float carZ, bool draw) {
    /* calcoli sull'angolo per applicare la rotazione */
    float angle = 360 - facing;
    float cosA = cos(angle * M_PI / 180.0);
    float sinA = sin(angle * M_PI / 180.0);
    /* distanze dal baricentro della macchina */
    float Z2B = 2.5;
    float Z2BTop = 2.5;
    float X2B = 0.1;
    float X2BTop = 0;

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

    float xC = carX - X3;
    float zC = carZ - Z3;

    /* centro del mirino */
    mirinoPoint.setX(xC);
    mirinoPoint.setZ(zC);

    if(draw) {
    /* disegno della linea 1-9 */
    drawLine(xA, 0.6, zA, xB, 0.3, zB, 1, 0, 0);
    /* disegno della linea 3-6 */
    drawLine(xB, 0.6, zB, xA, 0.3, zA, 1, 0, 0);
    /* disegno della linea 4-5 */
    drawLine(xA, 0.45, zA, xB, 0.45, zB, 1, 0, 0);
    /* disegno della linea 2-8 */
    drawLine(xC, 0.3, zC, xC, 0.6, zC, 1, 0, 0);
    }
}

/* disegno la luce per la retromarcia */
void drawLightingRetro(float x, float y, float z, float cos, float sin) {

    float dirX = 0.0;
    float dirZ = 0.0;
    /* in base al coseno setto la direzione Z verso cui la luce deve puntare */
    if(cos > 0.4) {
        dirZ = 1.0;
        z += 0.5;
    } else if (cos < -0.4) {
        dirZ = -1.0;
        z -= 0.5;
    }
    /* in base al coseno setto la direzione X verso cui la luce deve puntare */
    if(sin > 0.4) {
        dirX = -1.0;
    } else if (sin < -0.4) {
        dirX = 1.0;
    }

    glEnable(GL_LIGHT5);
    GLfloat ambient[] = {1.0, 0.6, 0.0, 1.0};
    glLightfv(GL_LIGHT5, GL_AMBIENT, ambient);
    GLfloat diffuse[] = {0.0, 0, 0, 1.0};
    glLightfv(GL_LIGHT5, GL_DIFFUSE, diffuse);
    /* no luce direzionale */
    GLfloat position[] = {x, y, z, 1};
    glLightfv(GL_LIGHT5, GL_POSITION, position);
    GLfloat spot_direction[] = {dirX, 0, dirZ, 0};
    glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT5, GL_SPOT_EXPONENT, 1.0);
    glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, 80.0);
    glLightf(GL_LIGHT5, GL_CONSTANT_ATTENUATION, 2);
    /* attenuazione relativamente alta per non propagare troppo la luce */
    float attenuation = 0.5;
    glLightf(GL_LIGHT5, GL_LINEAR_ATTENUATION, attenuation);
}

void Controller::drawReverseLight(float facing, float carX, float carZ, bool retroLight) {
    float angle = 360 - facing;
    float cosA = cos(angle * M_PI / 180.0);
    float sinA = sin(angle * M_PI / 180.0);
    float ZDistance = -1.3; // distanza da baricentro Z
    float XDistanceDX = 0.2; // distanza da baricentro Z
    float XDistanceSX = -0.15; // distanza da baricentro Z

    float Xdx = -XDistanceDX * cosA - ZDistance * sinA;
    float Xsx = -XDistanceSX * cosA - ZDistance * sinA;
    float Zdx = -XDistanceDX * sinA + ZDistance * cosA;
    float Zsx = -XDistanceSX * sinA + ZDistance * cosA;

    //TOP DEL TRIANGOLO
    float xPolygonDX = carX - Xdx;
    float xPolygonSX = carX - Xsx;
    float zPolygonDX = carZ - Zdx;
    float zPolygonSX = carZ - Zsx;

    glLineWidth(8);
    drawLine(xPolygonSX, 0.4, zPolygonSX, xPolygonSX, 0.45, zPolygonSX, 0, 0, 0);
    drawLine(xPolygonDX, 0.4, zPolygonDX, xPolygonDX, 0.45, zPolygonDX, 0, 0, 0);
    drawReverseLightPolygon(xPolygonSX, 0.4, zPolygonSX, facing, retroLight);
    drawReverseLightPolygon(xPolygonDX, 0.4, zPolygonDX, facing, retroLight);

    /* abilito o disabilito la luce in base a se sono in retro o meno */
    if(retroLight)
        drawLightingRetro(carX, 0.4, carZ, cosA, sinA);
    else
        glDisable(GL_LIGHT5);
}

/* disegno la luce della fiaccola */
void Controller::drawLightTorciaStatua() {
    glEnable(GL_LIGHT4);
    GLfloat ambient[] = {1, 0.0, 0.0, 1.0};
    glLightfv(GL_LIGHT4, GL_AMBIENT, ambient);
    GLfloat diffuse[] = {0.4, 0, 0, 1.0};
    glLightfv(GL_LIGHT4, GL_DIFFUSE, diffuse);
    GLfloat position[] = {Constant::POS_TORCIA_X, Constant::POS_TORCIA_Y + 10, Constant::POS_TORCIA_Z, 1};
    glLightfv(GL_LIGHT4, GL_POSITION, position);
    GLfloat spot_direction[] = {0, -1.0, 0, 0};
    glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT4, GL_SPOT_EXPONENT, 1.0);
    glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 90.0);
    glLightf(GL_LIGHT4, GL_CONSTANT_ATTENUATION, 0);
    float attenuation = 1;
    /* genero l'intermittenza in base al modulo */
    if (SDL_GetTicks() % 3 == 0)
        attenuation = 0.05;
    glLightf(GL_LIGHT4, GL_LINEAR_ATTENUATION, attenuation);
}

/* fornisce all'esterno la posizione X del target corrente */
float Controller::getTargetX() {
    return targetPoint.X();
};

/* fornisce all'esterno la posizione Z del target corrente */
float Controller::getTargetZ() {
    return targetPoint.Z();
};

/* fornisce all'esterno la posizione X del mirino */
float Controller::getMirinoX() {
    return mirinoPoint.X();
};

/* fornisce all'esterno la posizione Y del mirino */
float Controller::getMirinoY() {
    return mirinoPoint.Y();
};

/* fornisce all'esterno la posizione Z del mirino */
float Controller::getMirinoZ() {
    return mirinoPoint.Z();
};

/* booleano che decreta se il target è un Goal o meno */
bool Controller::isTargetGoal() {
    return isGoal;
};

/* booleano che decreta se il target è un TNT o meno */
bool Controller::isTargetTnt() {
    return isTnt;
};

/* booleano che decreta la fine del gioco */
bool Controller::isGameOver() {
    //    return (Controller::getSeconds() > 4);
    return gameOver;
};

/* booleano che decreta se il è scaduto il time */
bool Controller::isPlayerLoose() {
    return playerLoose;
};

/* ritorna lo score del player corrente */
int Controller::getScore() {
    return punteggio;
};

/* ritorna il numero di Goal generati */
int Controller::getGoal() {
    return goal;
};

/* ritorna il numero di Goal catturati */
int Controller::getGoalChecked() {
    return goalChecked;
};

/* ritorna il numero di TNT generati */
int Controller::getTnt() {
    return tnt;
};

/* ritorna il numero di TNT fatti esplodere */
int Controller::getTntChecked() {
    return tntChecked;
};

/* funzione che renderizza del testo in un box */
void Controller::drawText(TTF_Font *font, char fgR, char fgG, char fgB, char fgA, char bgR, char bgG, char bgB, char bgA, char *text, int x, int y) {
    SDL_Color tmpfontcolor = {fgR, fgG, fgB, fgA};
    SDL_Color tmpfontbgcolor = {bgR, bgG, bgB, bgA};
    SDL_Surface *initial;
    SDL_Surface *intermediary;
    SDL_Rect location;
    int w, h;

    initial = NULL;
    initial = TTF_RenderText_Shaded(font, text, tmpfontcolor, tmpfontbgcolor);

    w = initial->w;
    h = initial->h;

    intermediary = SDL_CreateRGBSurface(0, w, h, 32,
            0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    SDL_BlitSurface(initial, 0, intermediary, 0);

    glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_INFO_USER);
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

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_INFO_USER);
    glColor3f(1.0f, 1.0f, 1.0f);

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

    glColor3f(0.0f, 0.0f, 0.0f);

    glFinish();

    location.w = initial->w;
    location.h = initial->h;

    glDisable(GL_TEXTURE_2D);
    SDL_FreeSurface(initial);
    SDL_FreeSurface(intermediary);

}

void setNebbia() {
    GLfloat fogColor[4] = {0.6, 0.6, 0.6, 1.0};
    glFogi (GL_FOG_MODE, GL_LINEAR);
    glFogfv (GL_FOG_COLOR, fogColor);
    glFogf (GL_FOG_START, 0.0);
    glFogf (GL_FOG_END, 20.0);
    glClearColor(0.5, 0.5, 0.5, 1.0);
}

void Controller::drawNebbia(bool nebbia) {

    if(nebbia) {
        setNebbia();
        glEnable(GL_FOG);
    }
    else
        glDisable(GL_FOG);
}

/* disegna il layout di GameOver diversificando se l'utente ha perso o ha portato a termine il gioco */
void Controller::drawGameOverLayout(SDL_Window *win, TTF_Font *font, int scrH, int scrW) {

    glViewport(0, 0, scrW, scrH);
    /* sfondo nero */
    glClearColor(0, 0, 0, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(2.0 / scrW, 2.0 / scrH, 1);

    glLineWidth(2);

    char punti[3];
    sprintf(punti, "%d", punteggio);
    char str_game_over[] = "GAME OVER!!";
    char player_looser[] = "YOU LOOSE!!  TIME'S UP!!";
    char continuare[] = "Premi << ESC >> per uscire ...";
    
    TTF_Font *fontTitle;
    fontTitle = TTF_OpenFont("./ttf/amatic.ttf", 100);
    if (font == NULL) {
        fprintf(stderr, "Impossibile caricare il font.\n");
    }
    
    char tnt[20], goal[20], point[20], time[20];
    if(!playerLoose) { sprintf(tnt, "TNT:  %d / %d   ", Controller::getTntChecked(), Controller::getTnt()); }
    if(!playerLoose) { sprintf(goal, "GOAL:  %d / %d   ", Controller::getGoalChecked(), Controller::getGoal()); }
    if(!playerLoose) { sprintf(point, "SCORE:  %d   ", Controller::getScore()); }
    if(!playerLoose) { sprintf(time, "TIME:  %f ", endTime); }

    Controller::drawText(fontTitle, 0, 0, 0, 0, 210, 210, 210, 255, str_game_over, scrW / 2 - 120, scrH  - 120);
    if(!playerLoose) { 
        Controller::drawText(font, 0, 0, 0, 0, 210, 210, 210, 255, point, 50, scrH / 2 + 100);
        Controller::drawText(font, 0, 0, 0, 0, 210, 210, 210, 255, goal, 50, scrH / 2 + 30);
        Controller::drawText(font, 0, 0, 0, 0, 210, 210, 210, 255, tnt, 50, scrH / 2 -40);
        Controller::drawText(font, 0, 0, 0, 0, 210, 210, 210, 255, time, 50, scrH / 2 - 110);
    } else {
        Controller::drawText(fontTitle, 0, 0, 0, 0, 210, 210, 210, 255, player_looser, scrW / 2 - 200, scrH / 2 + 100);
    }
    Controller::drawText(font, 0, 0, 0, 0, 210, 210, 210, 255, continuare, scrW / 2 - 120, scrH / 2 - 180);
    glFinish();

    SDL_GL_SwapWindow(win);
}