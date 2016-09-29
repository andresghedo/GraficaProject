#include <math.h>
#include <time.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "controller.h"
#include "constants.h"
#include "car.h"

#define CAMERA_BACK_CAR 0
#define CAMERA_TOP_FIXED 1
#define CAMERA_TOP_CAR 2
#define CAMERA_PILOT 3
#define CAMERA_MOUSE 4
#define CAMERA_MIRINO 5
#define CAMERA_TYPE_MAX 6
#define PI 3.14159265

/* angoli che definiscono la vista */
float viewAlpha = 20, viewBeta = 40;
/* distanza dell'occhio dall'origine */
float eyeDist = 5.0;
/* altezza e larghezza viewport (in pixels) */
int scrH = 750, scrW = 750;
bool useWireframe = false;
bool useEnvmap = true;
bool useHeadlight = false;
bool useShadow = true;
int cameraType = 0;
/* l'entità Car */
Car car;
/* l'entità Controller */
Controller controller;
/* numero di passi di FISICA fatti fin'ora */
int nstep = 0;
/* numero di millisec che un passo di fisica simula */
const int PHYS_SAMPLING_STEP = 10;
/* lunghezza intervallo di calcolo fps */
const int fpsSampling = 3000;
/* valore di fps dell'intervallo precedente */
float fps = 0;
/* quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale */
int fpsNow = 0;
/* quando e' cominciato l'ultimo intervallo */
Uint32 timeLastInterval = 0;
/* extern functions */
extern void drawExtremeSX();
extern void drawMiddleLine();
extern void drawExtremeDX();
extern void drawStatua();

/* setta le matrici di trasformazione in modo
   che le coordinate in spazio oggetto siano le coord 
   del pixel sullo schemo */
void SetCoordToPixel() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(2.0 / scrW, 2.0 / scrH, 1);
}

/* carimento di una texture in path filename */
bool LoadTexture(int textbind, char *filename) {

    /* carica l'immagine tramite una chiamata SDL */
    SDL_Surface *s = IMG_Load(filename);
    if (!s) return false;

    glBindTexture(GL_TEXTURE_2D, textbind);
    gluBuild2DMipmaps(
            GL_TEXTURE_2D,
            GL_RGB,
            s->w, s->h,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            s->pixels
            );
    glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);
    glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
    return true;
}

/* disegna gli assi nel sistema di riferimento */
void drawAxis() {
    const float K = 0.10;
    glColor3f(0, 1, 0);
    /* disegno linee */
    glBegin(GL_LINES);
    glVertex3f(-2, 0, 0);
    glVertex3f(+2, 0, 0);

    glColor3f(1, 0, 0);
    glVertex3f(0, -2, 0);
    glVertex3f(0, +2, 0);

    glColor3f(0, 0, 1);
    glVertex3f(0, 0, -2);
    glVertex3f(0, 0, +2);
    glEnd();

    /* disegno i triangolini per le frecce */
    glBegin(GL_TRIANGLES);
    glVertex3f(0, +2, 0);
    glVertex3f(K, +2 - K, 0);
    glVertex3f(-K, +2 - K, 0);

    glVertex3f(+2, 0, 0);
    glVertex3f(+2 - K, +K, 0);
    glVertex3f(+2 - K, -K, 0);

    glVertex3f(0, 0, +2);
    glVertex3f(0, +K, +2 - K);
    glVertex3f(0, -K, +2 - K);
    glEnd();

}

/* disegna una sfera(usata nel disegno dello sky) */
void drawSphere(double r, int lats, int longs) {
    int i, j;
    for (i = 0; i <= lats; i++) {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0 = sin(lat0);
        double zr0 = cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (j = 0; j <= longs; j++) {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            //le normali servono per l'EnvMap
            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
    }
}

/* disegno la pista texturata */
void drawPistaTexture() {
    
    if(!useWireframe) {
        const float S = 500;
        const float H = 0;
        const int K = 750;

        if(useEnvmap) {
            /* disegno il terreno ripetendo una texture su di esso */
            glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_ASFALTO);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

        if(! useEnvmap) { glColor3ub(210, 210, 210); };
        /* disegna KxK quads */
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        for (int x = 0; x < K; x++)
            for (int z = 0; z < K; z++) {
                float x0 = -S + 2 * (x + 0) * S / K;
                float x1 = -S + 2 * (x + 1) * S / K;
                float z0 = -S + 2 * (z + 0) * S / K;
                float z1 = -S + 2 * (z + 1) * S / K;
                /* disegno solo i quadrati relativi alla strada */
                if ((x0 <= 4) && (x0 >= -6) && ((z0 < -430) || (z0 > -428))) {
                    if(useEnvmap) { glTexCoord2f(0.0, 0.0); }
                    glVertex3d(x0, H, z0);
                    if(useEnvmap) { glTexCoord2f(1.0, 0.0); }
                    glVertex3d(x1, H, z0);
                    if(useEnvmap) { glTexCoord2f(1.0, 1.0); }
                    glVertex3d(x1, H, z1);
                    if(useEnvmap) { glTexCoord2f(0.0, 1.0); }
                    glVertex3d(x0, H, z1);
                }
            }
        glEnd();
        if(useEnvmap) { glDisable(GL_TEXTURE_2D); }
    }
    return;
}

/* disegno l'asfalto in prossimità dell'arrivo con una scacchiera texture */
void drawArrivoTexture() {
    const float S = 500;
    const float H = 0;
    const int K = 750;

    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_RACING_FLAG);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    for (int x = 371; x <= 378; x++)
        for (int z = 53; z <= 54; z++) {
            float x0 = -S + 2 * (x + 0) * S / K;
            float x1 = -S + 2 * (x + 1) * S / K;
            float z0 = -S + 2 * (z + 0) * S / K;
            float z1 = -S + 2 * (z + 1) * S / K;
            /* sono in prossimità di fine pista texturo con la bandiera a scacchi */
            if ((x0 <= 4) && (x0 >= -6) && (z0 >= -430) && (z0 <= -428)) {
                glTexCoord2f(0.0, 0.0);
                glVertex3d(x0, H, z0);
                glTexCoord2f(1.0, 0.0);
                glVertex3d(x1, H, z0);
                glTexCoord2f(1.0, 1.0);
                glVertex3d(x1, H, z1);
                glTexCoord2f(0.0, 1.0);
                glVertex3d(x0, H, z1);
            }
        }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    return;
}

/* disegno erba texturata dove non ho strada */
void drawFloorTexture() {
    if(!useWireframe) {
        const float S = 500;
        const float H = 0;
        const int K = 750;

        if(useEnvmap) {
            glBindTexture(GL_TEXTURE_2D, Constant::TEXTURE_ID_ERBA);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }

        if(!useEnvmap) { glColor3ub(0, 179, 60); }
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        for (int x = 0; x < K; x++)
            for (int z = 0; z < K; z++) {
                float x0 = -S + 2 * (x + 0) * S / K;
                float x1 = -S + 2 * (x + 1) * S / K;
                float z0 = -S + 2 * (z + 0) * S / K;
                float z1 = -S + 2 * (z + 1) * S / K;
                if ((x0 > 4) || (x0<-6)) {
                    if(useEnvmap) { glTexCoord2f(0.0, 0.0); }
                    glVertex3d(x0, H, z0);
                    if(useEnvmap) { glTexCoord2f(1.0, 0.0); }
                    glVertex3d(x1, H, z0);
                    if(useEnvmap) { glTexCoord2f(1.0, 1.0); }
                    glVertex3d(x1, H, z1);
                    if(useEnvmap) { glTexCoord2f(0.0, 1.0); }
                    glVertex3d(x0, H, z1);
                }
            }
        glEnd();
        if(useEnvmap) { glDisable(GL_TEXTURE_2D); }
    }
}

/* disegno di un cerchio pieno(utilizzato per segnalare player e target nel radar) */
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for (int ii = 0; ii < num_segments; ii++) {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments); //get the current angle 
        float x = r * cos(theta); //calculate the x component 
        float y = r * sin(theta); //calculate the y component 
        glVertex2f(x + cx, y + cy); //output vertex 
    }
    glEnd();
}

/* disegno la bandiera a scacchi nel radar */
void drawFinishFlagRadar() {

    float color = 1;
    for(int x=45; x<=90; x+=5) {
        glColor3f(color, color, color);
        glBegin(GL_POLYGON);
            glVertex2d(x, scrH - 5 - 20);
            glVertex2d(x, scrH - 0 - 20);
            glVertex2d(x+5, scrH - 0 - 20);
            glVertex2d(x+5, scrH - 5 - 20);
        glEnd();
        if(color == 0)
            color = 1;
        else
            color = 0;
    }

    color = 0;
    for(int x=45; x<=90; x+=5) {
        glColor3f(color, color, color);
        glBegin(GL_POLYGON);
            glVertex2d(x, scrH - 10 - 20);
            glVertex2d(x, scrH - 5 - 20);
            glVertex2d(x+5, scrH - 5 - 20);
            glVertex2d(x+5, scrH - 10 - 20);
        glEnd();
        if(color == 0)
            color = 1;
        else
            color = 0;
    }

}

/* disegno il Radar in alto a SX */
void drawRadar(int scrH) {

    glDisable(GL_LIGHTING);
    float radar_car_x, radar_target_x;
    float radar_car_z, radar_target_z;
    /* posizione della macchina nel Radar proporzionata */
    radar_car_x = 70 + (200 * car.px / 40);
    radar_car_z = scrH - Constant::RADAR_LENGTH + (-1 * (Constant::RADAR_LENGTH * car.pz / 960)) + 110;
    /* posizione del target corrente nel Radar proporzionata */
    radar_target_x = 70 + (200 * controller.getTargetX() / 40);
    radar_target_z = scrH - Constant::RADAR_LENGTH + (-1 * (Constant::RADAR_LENGTH * controller.getTargetZ() / 960)) + 110;

    /* se è fuori dalla mappa riporto all'estremo la posizione della macchina */
    if (radar_car_x < 20)
        radar_car_x = 20;
    else if (radar_car_x > 120)
        radar_car_x = 120;

    /* se è fuori dalla mappa riporto all'estremo la posizione del target corrente */
    if (radar_target_x < 20)
        radar_target_x = 20;
    else if (radar_target_x > 120)
        radar_target_x = 120;

    /* disegno il cerchiolino della macchina nella posizione radar */
    glColor3ub(255, 0, 0);
    drawCircle(radar_car_x, radar_car_z, 3, 300);

    /* disegno il cerchiolino del target nella posizione radar */
    if (controller.isTargetGoal()) /* colore verde se è un goal */
        glColor3ub(0, 153, 51);
    else
        glColor3ub(0, 0, 0); /* colore nero se è un TNT */

    /* se la posizione X è -900 vuol dire che il giocatore è arrivato alla fine e non disegno il target */
    if (controller.getTargetX() != -900)
        drawCircle(radar_target_x, radar_target_z, 5, 300);

    /* colore bianco per linee stradali */
    glColor3f(1, 1, 1);

    glBegin(GL_POLYGON);
    /* disegno della linea SX stradale */
    glVertex2d(45.0, scrH - Constant::RADAR_LENGTH - 20);
    glVertex2d(45.0, scrH - 20);
    glVertex2d(47.0, scrH - 20);
    glVertex2d(47.0, scrH - Constant::RADAR_LENGTH - 20);
    glEnd();

    glBegin(GL_POLYGON);
    /* disegno della linea DX stradale */
    glVertex2d(95.0, scrH - Constant::RADAR_LENGTH - 20);
    glVertex2d(95.0, scrH - 20);
    glVertex2d(93.0, scrH - 20);
    glVertex2d(93.0, scrH - Constant::RADAR_LENGTH - 20);
    glEnd();
    
    /* disegno della linea tratteggiata stradale */
    for (int i = 0; i < Constant::RADAR_LENGTH - 10; i += 4) {
        glBegin(GL_POLYGON);
        glVertex2d(69, scrH - Constant::RADAR_LENGTH - 20 + i);
        glVertex2d(69, scrH - Constant::RADAR_LENGTH - 20 + i + 2);
        glVertex2d(71, scrH - Constant::RADAR_LENGTH - 20 + i + 2);
        glVertex2d(71, scrH - Constant::RADAR_LENGTH - 20 + i);
        glEnd();
    }

    /* colore nero per il bordo del radar */
    glColor3ub(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2d(20, scrH - 20 - Constant::RADAR_LENGTH);
    glVertex2d(20, scrH - 20);
    glVertex2d(120, scrH - 20);
    glVertex2d(120, scrH - 20 - Constant::RADAR_LENGTH);
    glEnd();

    /* disegno la scacchiera di arrivo */
    drawFinishFlagRadar();

    /* disegno il grigio della strada */
    glColor3ub(210, 210, 210);
    glBegin(GL_POLYGON);
    glVertex2d(45, scrH - 20 - Constant::RADAR_LENGTH);
    glVertex2d(45, scrH - 20);
    glVertex2d(95, scrH - 20);
    glVertex2d(95, scrH - 20 - Constant::RADAR_LENGTH);
    glEnd();

    /* disegno il verde del fuori pista */
    glColor3ub(0, 179, 60);
    glBegin(GL_POLYGON);
    glVertex2d(20, scrH - 20 - Constant::RADAR_LENGTH);
    glVertex2d(20, scrH - 20);
    glVertex2d(120, scrH - 20);
    glVertex2d(120, scrH - 20 - Constant::RADAR_LENGTH);
    glEnd();
    glEnable(GL_LIGHTING);
}

/* setto la posizione della camera */
void setCamera() {

    double px = car.px;
    double py = car.py;
    double pz = car.pz;
    double angle = car.facing;
    double cosf = cos(angle * M_PI / 180.0);
    double sinf = sin(angle * M_PI / 180.0);
    double camd, camh, ex, ey, ez, cx, cy, cz;
    double cosff, sinff;

    /* controllo la posizione della camera a seconda dell'opzione selezionata */
    switch (cameraType) {
        case CAMERA_BACK_CAR:
            camd = 2.5;
            camh = 1.0;
            /* PUNTO DI AZIONE */
            ex = px + camd*sinf;
            ey = py + camh;
            ez = pz + camd*cosf;
            /* PUNTO VERSO CUI GUARDO */
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_MIRINO:
            camd = 0.0;
            camh = 0.0;
            /* PUNTO DI AZIONE */
            ex = controller.getMirinoX() + camd*sinf;
            ey = controller.getMirinoY() + camh;
            ez = controller.getMirinoZ() + camd*cosf;
            /* PUNTO VERSO CUI GUARDO */
            cx = controller.getMirinoX() - 100 * sinf;
            cy = controller.getMirinoY() + camh;
            cz = controller.getMirinoZ() - 100 * cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_TOP_FIXED:
            camd = 0.5;
            camh = 0.55;
            angle = car.facing + 40.0;
            cosff = cos(angle * M_PI / 180.0);
            sinff = sin(angle * M_PI / 180.0);
            ex = px + camd*sinff;
            ey = py + camh;
            ez = pz + camd*cosff;
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_TOP_CAR:
            camd = 2.5;
            camh = 1.0;
            ex = px + camd*sinf;
            ey = py + camh;
            ez = pz + camd*cosf;
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            gluLookAt(ex, ey + 5, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_PILOT:
            camd = 0.2;
            camh = 0.55;
            ex = px + camd*sinf;
            ey = py + camh;
            ez = pz + camd*cosf;
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_MOUSE:
            glTranslatef(0, 0, -eyeDist);
            glRotatef(viewBeta, 1, 0, 0);
            glRotatef(viewAlpha, 0, 1, 0);
            break;
    }
}

/* disegna il cielo */
void drawSky() {

    /* se voglio mesh in bianco nero */
    if (useWireframe) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0, 0, 0);
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawSphere(500.0, 20, 20); // OLD 100
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(1, 1, 1);
        glEnable(GL_LIGHTING);
    } else {
        if(useEnvmap) { glBindTexture(GL_TEXTURE_2D, 2); }
        if(useEnvmap) { glEnable(GL_TEXTURE_2D); }
        if(useEnvmap) { glEnable(GL_TEXTURE_GEN_S); }
        if(useEnvmap) { glEnable(GL_TEXTURE_GEN_T); }
        if(useEnvmap) { glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); } // Env map 
        if(useEnvmap) { glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); }
        /* se non uso texture uso il colore BLU */
        if(useEnvmap) { glColor3f(1, 1, 1); } else { glColor3f(0, 0, 1); }
        glDisable(GL_LIGHTING); 

        drawSphere(500.0, 20, 20);

        if(useEnvmap) { glDisable(GL_TEXTURE_GEN_S); }
        if(useEnvmap) { glDisable(GL_TEXTURE_GEN_T); }
        if(useEnvmap) { glDisable(GL_TEXTURE_2D); }
        glEnable(GL_LIGHTING);
    }

}

/* Esegue il Rendering della scena */
void rendering(SDL_Window *win, TTF_Font *font) {

    /* incremento dei frame */
    fpsNow++;
    /* larghezza linee */
    glLineWidth(3);
    /* setting viewport */
    glViewport(0, 0, scrW, scrH);
    /* colore sfondo bianco */
    glClearColor(1, 1, 1, 1);
    /* matrice di proiezione */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70,
        ((float) scrW) / scrH, /* aspect Y/X, */
        0.2, /* distanza del NEAR CLIPPING PLANE in coordinate vista */
        1000 /* distanza del FAR CLIPPING PLANE in coordinate vista */
        );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* riempe tutto lo screen buffer di pixel color sfondo */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setCamera();

    /* luce principale del mondo */
    float tmpv[4] = {0, 1, 2, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, tmpv);
    /* disegno la luce della statua */
    controller.drawLightTorciaStatua();
    /* disegno assi XYZ del MONDO(Per debug) */
    drawAxis();
    /* disegno il mirino */
    bool mirino = (cameraType != CAMERA_MIRINO);
    controller.drawMirino(car.facing, car.px, car.pz, mirino);
    /* disegno le luci posteriori dell'auto */
    controller.drawReverseLight(car.facing, car.px, car.pz, controller.key[Controller::DEC]);
    static float tmpcol[4] = {1, 1, 1, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);

    glEnable(GL_LIGHTING);
    /* disegno del cielo */
    drawSky();
    /* disegno del suolo texturato(erba) */
    drawFloorTexture();
    /* disegno della pista texturata */
    drawPistaTexture();
    /* disegno dell'arrivo texturato con la bandiera a scacchi */
    drawArrivoTexture();
    /* disegno linea SX stradale */
    drawExtremeSX();
    /* disegno della linea tratteggiata */
    drawMiddleLine();
    /* disegno linea SX stradale */
    drawExtremeDX();
    /* disegno della statua della liertà */
    drawStatua();
    /* disegno il target(sia esso goal o tnt) */
    controller.drawTargetCube();
    /* render della macchina */
    car.Render();

    /* attendiamo la fine della rasterizzazione di tutte le primitive mandate */
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // disegnamo i fps (frame x sec) come una barra a sinistra.(vuota = 0 fps, piena = 100 fps) */
    SetCoordToPixel();
    
    /* disegno barra a sx */
    glBegin(GL_QUADS);
    float y = scrH * fps / 100;
    float ramp = fps / 100;
    glColor3f(1 - ramp, 0, ramp);
    glVertex2d(10, 0);
    glVertex2d(10, y);
    glVertex2d(0, y);
    glVertex2d(0, 0);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glLineWidth(2);
    /* disegno del Radar in alto a SX */
    drawRadar(scrH);
    /* mi preparo il testo da mettere nella barra di stato */
    char tnt[20];
    sprintf(tnt, "TNT:  %d / %d   ", controller.getTntChecked(), controller.getTnt());
    char goal[20];
    sprintf(goal, "GOAL:  %d / %d   ", controller.getGoalChecked(), controller.getGoal());
    char *tntAndGoal = strcat(tnt, goal);
    char point[20];
    sprintf(point, "SCORE:  %d   ", controller.getScore());
    char time[20];
    sprintf(time, "TIME:  %lf ", -Constant::GAME_LIMIT_SECONDS + controller.getSeconds());
    strcat(point, time);
    glDisable(GL_LIGHTING);
    /* disegno la barra di stato con le info utente(punti, tempo, etc...) */
    controller.drawText(font, 0, 0, 0, 0, 210, 210, 210, 255, strcat(tntAndGoal, point), scrW - 600, scrH - 100);
    glFinish();
    SDL_GL_SwapWindow(win);
}

/* ridisegno della finestra */
void redraw() {
    SDL_Event e;
    e.type = SDL_WINDOWEVENT;
    e.window.event = SDL_WINDOWEVENT_EXPOSED;
    SDL_PushEvent(&e);
}

/* MAIN del programma */
int main(int argc, char* argv[]) {
    SDL_Window *win;
    SDL_GLContext mainContext;
    Uint32 windowID;
    SDL_Joystick *joystick;
    static int keymap[Controller::NKEYS] = {SDLK_a, SDLK_d, SDLK_w, SDLK_s};

    /* inizializzo SDL */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    /* inizializzo TTF per le scritte */
    if (TTF_Init() < 0) {
        fprintf(stderr, "Impossibile inizializzare TTF: %s\n", SDL_GetError());
        SDL_Quit();
        return (2);
    }

    /* ricavo un font dalla cartella ./ttf/ */
    TTF_Font *font;
    font = TTF_OpenFont("./ttf/amatic.ttf", 45);
    if (font == NULL) {
        fprintf(stderr, "Impossibile caricare il font.\n");
    }

    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* facciamo una finestra di scrW x scrH pixels */
    win = SDL_CreateWindow(argv[0], 0, 0, scrW, scrH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    /* ricavo il context */
    mainContext = SDL_GL_CreateContext(win);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali 
    glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_POLYGON_OFFSET_FILL); // caro openGL sposta i 
    glPolygonOffset(1, 1); // indietro di 1

    /* Caricamento preventivo delle Texture */
    if (!LoadTexture(Constant::TEXTURE_ID_LOGO_GOMME, (char *) "./img/logo_gomme.jpg")) return 0;
    if (!LoadTexture(Constant::TEXTURE_ID_CARROZZERIA, (char *) "./img/envmap_flipped.jpg")) return 0;
    if (!LoadTexture(Constant::TEXTURE_ID_SKY, (char *) "./img/sky_ok.jpg")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_TNT, (char *) "./img/tnt.png")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_ASFALTO, (char *) "./img/asfalto.png")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_ERBA, (char *) "./img/erba.jpg")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_SELFIE, (char *) "./img/selfie.jpg")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_RACING_FLAG, (char *) "./img/racing_flag.jpg")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_LIGHT_ON, (char *) "./img/light_on.png")) return -1;
    if (!LoadTexture(Constant::TEXTURE_ID_LIGHT_OFF, (char *) "./img/light_off.png")) return -1;

    /* decreta a fine del gioco */
    bool done = 0;
    /* decreta quando uscire dalla schermata di GameOver */
    bool doneGO = 0;
    while (!done) {

        SDL_Event e;

        /* controllo se c'è un evento utente*/
        if (SDL_PollEvent(&e)) {
            /* processo in base al tasto premuto */
            switch (e.type) {
                case SDL_KEYDOWN:
                    /* setto l'input per la macchina */
                    controller.EatKey(e.key.keysym.sym, keymap, true);
                    /* cambio posizione camera */
                    if (e.key.keysym.sym == SDLK_F1) cameraType = (cameraType + 1) % CAMERA_TYPE_MAX;
                    /* vedo le mesh nude bianco e nero */
                    if (e.key.keysym.sym == SDLK_F2) useWireframe = !useWireframe;
                    /* si/no texture */
                    if (e.key.keysym.sym == SDLK_F3) useEnvmap = !useEnvmap;
                    /* fanali anteriori */
                    if (e.key.keysym.sym == SDLK_F4) useHeadlight = !useHeadlight;
                    /* mostro ombra della macchina o meno */
                    if (e.key.keysym.sym == SDLK_F5) useShadow = !useShadow;
                    break;
                case SDL_KEYUP:
                    controller.EatKey(e.key.keysym.sym, keymap, false);
                    break;
                case SDL_QUIT:
                    done = 1;
                    doneGO = 1;
                    break;
                case SDL_WINDOWEVENT:
                    /* ridisegno la finestra */
                    if (e.window.event == SDL_WINDOWEVENT_EXPOSED) {
                        rendering(win, font);
                    } else {
                        windowID = SDL_GetWindowID(win);
                        if (e.window.windowID == windowID) {
                            switch (e.window.event) {
                                case SDL_WINDOWEVENT_SIZE_CHANGED:
                                {
                                    scrW = e.window.data1;
                                    scrH = e.window.data2;
                                    glViewport(0, 0, scrW, scrH);
                                    rendering(win, font);
                                    break;
                                }
                            }
                        }
                    }
                    break;

                    /* mi muovo con gli eventi mouse se la telecameta è CAMERA_MOUSE */
                case SDL_MOUSEMOTION:
                    if (e.motion.state & SDL_BUTTON(1) & cameraType == CAMERA_MOUSE) {
                        viewAlpha += e.motion.xrel;
                        viewBeta += e.motion.yrel;
                        /* per non andare sotto la macchina */
                        if (viewBeta<+5) viewBeta = +5;
                        if (viewBeta>+90) viewBeta = +90;
                    }
                    break;

                case SDL_MOUSEWHEEL:
                    if (e.wheel.y < 0) {
                        /* Zoom IN */
                        eyeDist = eyeDist * 0.9;
                        if (eyeDist < 1) eyeDist = 1;
                    };
                    if (e.wheel.y > 0) {
                        /* Zoom OUT */
                        eyeDist = eyeDist / 0.9;
                    };
                    break;

                case SDL_JOYAXISMOTION: // Handle Joystick Motion 
                    if (e.jaxis.axis == 0) {
                        if (e.jaxis.value < -3200) {
                            controller.Joy(0, true);
                            controller.Joy(1, false);
                        }
                        if (e.jaxis.value > 3200) {
                            controller.Joy(0, false);
                            controller.Joy(1, true);
                        }
                        if (e.jaxis.value >= -3200 && e.jaxis.value <= 3200) {
                            controller.Joy(0, false);
                            controller.Joy(1, false);
                        }
                        rendering(win, font);
                    }
                    break;
                case SDL_JOYBUTTONDOWN: // Handle Joystick Button Presses 
                    if (e.jbutton.button == 0) {
                        controller.Joy(2, true);
                    }
                    if (e.jbutton.button == 2) {
                        controller.Joy(3, true);
                    }
                    break;
                case SDL_JOYBUTTONUP: // Handle Joystick Button Presses 
                    controller.Joy(2, false);
                    controller.Joy(3, false);
                    break;
            }
        } else {

            Uint32 timeNow = SDL_GetTicks();

            if (timeLastInterval + fpsSampling < timeNow) {
                fps = 1000.0 * ((float) fpsNow) / (timeNow - timeLastInterval);
                fpsNow = 0;
                timeLastInterval = timeNow;
            }

            bool doneSomething = false;
            /* sicurezza da loop infiniti */
            int guardia = 0;

            /* finche' il tempo simulato e' rimasto indietro rispetto al tempo reale...*/
            while (nstep * PHYS_SAMPLING_STEP < timeNow) {
                car.DoStep(controller.key[Controller::LEFT], controller.key[Controller::RIGHT], controller.key[Controller::ACC], controller.key[Controller::DEC]);
                nstep++;
                doneSomething = true;
                timeNow = SDL_GetTicks();
                if (guardia++ > 1000) {
                    done = true;
                    break;
                }
            }
            /* controllo i vincoli di gioco se non sono in gameover*/
            if ((doneSomething)&&(!controller.isGameOver())) {
                rendering(win, font);
                controller.checkConstraintsGame(car.pz);
            } else if (controller.isGameOver()) { /* se sono in gameover finisco il ciclo di gioco */
                done = true;
            }
        }
    }

    /* ciclo per schermata game over */
    while (!doneGO) {
        SDL_Event e;

        if (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYDOWN:
                    /* esco definitivamente dal gioco con <<ESC>> */
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        doneGO = true;
                    break;
                case SDL_QUIT:
                    doneGO = true;
                    break;
                /* resize */
                case SDL_WINDOWEVENT:
                    windowID = SDL_GetWindowID(win);
                    if (e.window.windowID == windowID) {
                        switch (e.window.event) {
                            case SDL_WINDOWEVENT_SIZE_CHANGED:
                            {
                                scrW = e.window.data1;
                                scrH = e.window.data2;
                                glViewport(0, 0, scrW, scrH);
                                break;
                            }
                        }
                    }

                    break;
            }
        } else {
            /* mostro il layout finale */
            controller.drawGameOverLayout(win, font, scrH, scrW);
        }
    }
    /* destroy e fine */
    SDL_GL_DeleteContext(mainContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return (0);
}