#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "car.h"

#define CAMERA_BACK_CAR 0
#define CAMERA_TOP_FIXED 1
#define CAMERA_TOP_CAR 2
#define CAMERA_PILOT 3
#define CAMERA_MOUSE 4
#define CAMERA_TYPE_MAX 5

float viewAlpha = 20, viewBeta = 40; // angoli che definiscono la vista
float eyeDist = 5.0; // distanza dell'occhio dall'origine
int scrH = 750, scrW = 750; // altezza e larghezza viewport (in pixels)
bool useWireframe = false;
bool useEnvmap = true;
bool useHeadlight = false;
bool useShadow = true;
int cameraType = 0;

Car car; // la nostra macchina
int nstep = 0; // numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP = 10; // numero di millisec che un passo di fisica simula

// Frames Per Seconds
const int fpsSampling = 3000; // lunghezza intervallo di calcolo fps
float fps = 0; // valore di fps dell'intervallo precedente
int fpsNow = 0; // quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
Uint32 timeLastInterval = 0; // quando e' cominciato l'ultimo intervallo

extern void drawPista();
extern void drawPoke();

// setta le matrici di trasformazione in modo
// che le coordinate in spazio oggetto siano le coord 
// del pixel sullo schemo

void SetCoordToPixel() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1, -1, 0);
    glScalef(2.0 / scrW, 2.0 / scrH, 1);
}

// CARICO PRELIMINARMENTE LE TEXTURE
bool LoadTexture(int textbind, char *filename) {
    
    printf("[DEBUG]Carico l'immagine per la texture : %s \n", filename);
    // carica l'immagine tramite una chiamata SDL
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

// disegna gli assi nel sist. di riferimento

void drawAxis() {
    const float K = 0.10;
    glColor3f(0, 1, 0);// GREEN
    glBegin(GL_LINES);
    glVertex3f(-2, 0, 0);
    glVertex3f(+2, 0, 0);

    glColor3f(1, 0, 0);// RED
    glVertex3f(0, -2, 0);
    glVertex3f(0, +2, 0);

    glColor3f(0, 0, 1);// BLUE
    glVertex3f(0, 0, -2);
    glVertex3f(0, 0, +2);
    glEnd();

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

// DISEGNO DEL "CIELO"
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

// DISEGNA LA PAVIMENTAZIONE
void drawFloor() {
    const float S = 100; // size
    const float H = 0; // altezza
    const int K = 150; //disegna K x K quads

    // disegna KxK quads
    glBegin(GL_QUADS);
    glColor3f(0.6, 0.6, 0.6); // colore uguale x tutti i quads
    glNormal3f(0, 1, 0); // normale verticale uguale x tutti
    for (int x = 0; x < K; x++)
        for (int z = 0; z < K; z++) {
            float x0 = -S + 2 * (x + 0) * S / K;
            float x1 = -S + 2 * (x + 1) * S / K;
            float z0 = -S + 2 * (z + 0) * S / K;
            float z1 = -S + 2 * (z + 1) * S / K;
            glVertex3d(x0, H, z0);
            glVertex3d(x1, H, z0);
            glVertex3d(x1, H, z1);
            glVertex3d(x0, H, z1);
        }
    glEnd();
}

/****************************************/
/* function per disegnare una minimappa */
/****************************************/
void drawMinimap(int scrH, int scrW) {
    printf("scrH: %d scrW: %d\n", scrH, scrW);
  /* calcolo delle coordinate reali dell'oggetto su minimappa */
  float minimap_posx;
  float minimap_posz;
  minimap_posx = ((50*car.px)/67) + 50 + 20;
  minimap_posz = ((50*car.pz)/67) + 50 + scrH-20-100;

//  float minimap_cubex;
//  float minimap_cubez;
//  minimap_cubex = ((50*pos_x)/67) + 50 + 20;
//  minimap_cubez = ((50*pos_z)/67) + 50 + scrH-20-100;

  /* disegno del cursore */
  glColor3ub(0,0,255);
  glBegin(GL_QUADS);
    glVertex2d(minimap_posx, minimap_posz + 3);
    glVertex2d(minimap_posx + 3, minimap_posz);
    glVertex2d(minimap_posx, minimap_posz - 3);
    glVertex2d(minimap_posx - 3, minimap_posz);
  glEnd();

  /* disegno del target */
//  glColor3ub(255,0,0);
//  glBegin(GL_QUADS);
//    glVertex2d(minimap_cubex, minimap_cubez + 3);
//    glVertex2d(minimap_cubex + 3, minimap_cubez);
//    glVertex2d(minimap_cubex, minimap_cubez - 3);
//    glVertex2d(minimap_cubex - 3, minimap_cubez);
//  glEnd();

  /* disegno minimappa */
  glColor3ub(0,0,0);
  glBegin(GL_LINE_LOOP);
    glVertex2d(20,scrH -20 -100);
    glVertex2d(20,scrH -20);
    glVertex2d(120,scrH-20);
    glVertex2d(120,scrH-20-100);
  glEnd();

  glColor3ub(210,210,210);
  glBegin(GL_POLYGON);
    glVertex2d(20,scrH -20 -100);
    glVertex2d(20,scrH -20);
    glVertex2d(120,scrH -20);
    glVertex2d(120,scrH-20-100);
   glEnd();
}

// setto la posizione della camera
void setCamera() {
    //printf("[DEBUG]Setta la posizione della CAM %d...\n", cameraType);
    double px = car.px;
    double py = car.py;
    double pz = car.pz;
    double angle = car.facing;
    double cosf = cos(angle * M_PI / 180.0);
    double sinf = sin(angle * M_PI / 180.0);
    double camd, camh, ex, ey, ez, cx, cy, cz;
    double cosff, sinff;

    // controllo la posizione della camera a seconda dell'opzione selezionata
    switch (cameraType) {
        case CAMERA_BACK_CAR:
            printf("[DEBUG] Camera:  CAMERA_BACK_CAR\n");
            camd = 2.5;
            camh = 1.0;
            // PUNTO DI AZIONE
            ex = px + camd*sinf;
            ey = py + camh;
            ez = pz + camd*cosf;
            // PUNTO VERSO CUI GUARDO
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            printf("[DEBUG] CAR X: %f  Y: %f  Z:%f\n", px,py,pz);
            printf("[DEBUG] CAM EX: %f  EY: %f  EZ:%f\n", ex,ey,ez);
            printf("[DEBUG] CAM CX: %f  CY: %f  CZ:%f\n", cx,cy,cz);
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_TOP_FIXED:
            printf("[DEBUG] Camera:  CAMERA_TOP_FIXED\n");
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
            printf("[DEBUG] CAR X: %f  Y: %f  Z:%f\n", px,py,pz);
            printf("[DEBUG] CAM EX: %f  EY: %f  EZ:%f\n", ex,ey,ez);
            printf("[DEBUG] CAM CX: %f  CY: %f  CZ:%f\n", cx,cy,cz);
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_TOP_CAR:
            printf("[DEBUG] Camera:  CAMERA_TOP_CAR\n");
            camd = 2.5;
            camh = 1.0;
            ex = px + camd*sinf;
            ey = py + camh;
            ez = pz + camd*cosf;
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            printf("[DEBUG] CAR X: %f  Y: %f  Z:%f\n", px,py,pz);
            printf("[DEBUG] CAM EX: %f  EY: %f  EZ:%f\n", ex,ey,ez);
            printf("[DEBUG] CAM CX: %f  CY: %f  CZ:%f\n", cx,cy,cz);
            gluLookAt(ex, ey + 5, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_PILOT:
            printf("[DEBUG] Camera:  CAMERA_PILOT\n");
            camd = 0.2;
            camh = 0.55;
            ex = px + camd*sinf;
            ey = py + camh;
            ez = pz + camd*cosf;
            cx = px - camd*sinf;
            cy = py + camh;
            cz = pz - camd*cosf;
            printf("[DEBUG] CAR X: %f  Y: %f  Z:%f\n", px,py,pz);
            printf("[DEBUG] CAM EX: %f  EY: %f  EZ:%f\n", ex,ey,ez);
            printf("[DEBUG] CAM CX: %f  CY: %f  CZ:%f\n", cx,cy,cz);
            gluLookAt(ex, ey, ez, cx, cy, cz, 0.0, 1.0, 0.0);
            break;
        case CAMERA_MOUSE:
            printf("[DEBUG] Camera:  CAMERA_MOUSE\n");
            glTranslatef(0, 0, -eyeDist);
            glRotatef(viewBeta, 1, 0, 0);
            glRotatef(viewAlpha, 0, 1, 0);
            break;
    }
}

// disegna il cielo
void drawSky() {
    int H = 100;

    if (useWireframe) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0, 0, 0);
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        drawSphere(100.0, 20, 20);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(1, 1, 1);
        glEnable(GL_LIGHTING);
    } else {
        glBindTexture(GL_TEXTURE_2D, 2);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Env map
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glColor3f(1, 1, 1);
        glDisable(GL_LIGHTING);

        //   drawCubeFill();
        drawSphere(100.0, 20, 20);

        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
    }

}

/* Esegue il Rendering della scena */
void rendering(SDL_Window *win) {

    // un frame in piu'!!!
    fpsNow++;

    glLineWidth(3); // larchezza linee che vengono disegnate in scena

    // settiamo il viewport
    glViewport(0, 0, scrW, scrH);

    // colore sfondo = bianco
    glClearColor(1, 1, 1, 1);


    // settiamo la matrice di proiezione
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, //fovy,
            ((float) scrW) / scrH, //aspect Y/X,
            0.2, //distanza del NEAR CLIPPING PLANE in coordinate vista
            1000 //distanza del FAR CLIPPING PLANE in coordinate vista
            );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // riempe tutto lo screen buffer di pixel color sfondo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //drawAxis(); // disegna assi frame VISTA

    // setto la posizione luce
    float tmpv[4] = {0, 1, 2, 0}; // ultima comp=0 => luce direzionale
    glLightfv(GL_LIGHT0, GL_POSITION, tmpv);

    setCamera(); // QUI SPOSTO LA VISUALE!!!!


    drawAxis(); // disegna assi frame MONDO

    static float tmpcol[4] = {1, 1, 1, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);

    glEnable(GL_LIGHTING);

    // settiamo matrice di modellazione
    drawAxis(); // disegna assi frame OGGETTO
    //drawCubeWire();

    drawSky();      // DISEGNO CIELO
    drawFloor();    // DISEGNO SUOLO
    //drawPista();    // DISEGNO PISTA
    drawPoke();     // DISEGNO POKEBALL
    //drawMinimap(scrH, scrW);
    car.controller.drawTarget();

    car.Render();   // DISEGNA LA MACCHINA--> SENZA QUESTO LA MACCHINA NON SI VEDE

    // attendiamo la fine della rasterizzazione di 
    // tutte le primitive mandate 

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // disegnamo i fps (frame x sec) come una barra a sinistra.
    // (vuota = 0 fps, piena = 100 fps)
    
    //DISEGNA LA BARRA A SX
    SetCoordToPixel();

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
    // FINE DISEGNO BARRA A SX

    glFinish();
    // ho finito: buffer di lavoro diventa visibile
    SDL_GL_SwapWindow(win);
}

void redraw() {
    // ci automandiamo un messaggio che (s.o. permettendo)
    // ci fara' ridisegnare la finestra
    SDL_Event e;
    e.type = SDL_WINDOWEVENT;
    e.window.event = SDL_WINDOWEVENT_EXPOSED;
    SDL_PushEvent(&e);
}

/*Scheletro adottato nelle prime esercitazioni SDL*/
int main(int argc, char* argv[]) {
    SDL_Window *win;
    SDL_GLContext mainContext;
    Uint32 windowID;
    SDL_Joystick *joystick;
    static int keymap[Controller::NKEYS] = {SDLK_a, SDLK_d, SDLK_w, SDLK_s};

    // inizializzazione di SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    SDL_JoystickEventState(SDL_ENABLE);
    joystick = SDL_JoystickOpen(0);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // facciamo una finestra di scrW x scrH pixels
    win = SDL_CreateWindow(argv[0], 0, 0, scrW, scrH, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    //Create our opengl context and attach it to our window
    mainContext = SDL_GL_CreateContext(win);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali 
    // prima di usarle
    //glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_POLYGON_OFFSET_FILL); // caro openGL sposta i 
    // frammenti generati dalla
    // rasterizzazione poligoni
    glPolygonOffset(1, 1); // indietro di 1

    if (!LoadTexture(0, (char *) "./img/logo.jpg")) return 0;
    if (!LoadTexture(1, (char *) "./img/envmap_flipped.jpg")) return 0;
    if (!LoadTexture(2, (char *) "./img/sky_ok.jpg")) return -1;

    bool done = 0;
    while (!done) {

        SDL_Event e;

        // guardo se c'e' un evento:
        if (SDL_PollEvent(&e)) {
            // se si: processa evento
            switch (e.type) {
                case SDL_KEYDOWN:
                    // pressione di un tasto movimento macchina
                    car.controller.EatKey(e.key.keysym.sym, keymap, true);
                    // cambia camera
                    if (e.key.keysym.sym == SDLK_F1) cameraType = (cameraType + 1) % CAMERA_TYPE_MAX;
                    if (e.key.keysym.sym == SDLK_F2) useWireframe = !useWireframe;
                    if (e.key.keysym.sym == SDLK_F3) useEnvmap = !useEnvmap;
                    if (e.key.keysym.sym == SDLK_F4) useHeadlight = !useHeadlight;
                    if (e.key.keysym.sym == SDLK_F5) useShadow = !useShadow;
                    break;
                case SDL_KEYUP:
                    car.controller.EatKey(e.key.keysym.sym, keymap, false);
                    break;
                case SDL_QUIT:
                    done = 1;
                    break;
                case SDL_WINDOWEVENT:
                    // dobbiamo ridisegnare la finestra
                    if (e.window.event == SDL_WINDOWEVENT_EXPOSED) {
                        rendering(win);
                        //printf("[DEBUG] windows event if!\n");
                    } else {
                        //printf("[DEBUG] windows event else!\n");
                        windowID = SDL_GetWindowID(win);
                        if (e.window.windowID == windowID) {
                            switch (e.window.event) {
                                case SDL_WINDOWEVENT_SIZE_CHANGED:
                                {
                                    scrW = e.window.data1;
                                    scrH = e.window.data2;
                                    glViewport(0, 0, scrW, scrH);
                                    rendering(win);
                                    //redraw(); // richiedi ridisegno
                                    break;
                                }
                            }
                        }
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (e.motion.state & SDL_BUTTON(1) & cameraType == CAMERA_MOUSE) {
                        viewAlpha += e.motion.xrel;
                        viewBeta += e.motion.yrel;
                        //          if (viewBeta<-90) viewBeta=-90;
                        if (viewBeta<+5) viewBeta = +5; //per non andare sotto la macchina
                        if (viewBeta>+90) viewBeta = +90;
                        // redraw(); // richiedi un ridisegno (non c'e' bisongo: si ridisegna gia' 
                        // al ritmo delle computazioni fisiche)
                    }
                    break;

                case SDL_MOUSEWHEEL:
                    if (e.wheel.y < 0) {
                        // avvicino il punto di vista (zoom in)
                        eyeDist = eyeDist * 0.9;
                        if (eyeDist < 1) eyeDist = 1;
                    };
                    if (e.wheel.y > 0) {
                        // allontano il punto di vista (zoom out)
                        eyeDist = eyeDist / 0.9;
                    };
                    break;

                case SDL_JOYAXISMOTION: // Handle Joystick Motion 
                    if (e.jaxis.axis == 0) {
                        if (e.jaxis.value < -3200) {
                            car.controller.Joy(0, true);
                            car.controller.Joy(1, false);
                            //	      printf("%d <-3200 \n",e.jaxis.value);
                        }
                        if (e.jaxis.value > 3200) {
                            car.controller.Joy(0, false);
                            car.controller.Joy(1, true);
                            //	      printf("%d >3200 \n",e.jaxis.value);
                        }
                        if (e.jaxis.value >= -3200 && e.jaxis.value <= 3200) {
                            car.controller.Joy(0, false);
                            car.controller.Joy(1, false);
                            //	      printf("%d in [-3200,3200] \n",e.jaxis.value);
                        }
                        rendering(win);
                        //redraw();
                    }
                    break;
                case SDL_JOYBUTTONDOWN: // Handle Joystick Button Presses 
                    if (e.jbutton.button == 0) {
                        car.controller.Joy(2, true);
                        //	   printf("jbutton 0\n");
                    }
                    if (e.jbutton.button == 2) {
                        car.controller.Joy(3, true);
                        //	   printf("jbutton 2\n");
                    }
                    break;
                case SDL_JOYBUTTONUP: // Handle Joystick Button Presses 
                    car.controller.Joy(2, false);
                    car.controller.Joy(3, false);
                    break;
            }
        } else {
            //printf("[DEBUG]Else while\n");
            // nessun evento: siamo IDLE

            Uint32 timeNow = SDL_GetTicks(); // che ore sono?

            if (timeLastInterval + fpsSampling < timeNow) {
                fps = 1000.0 * ((float) fpsNow) / (timeNow - timeLastInterval);
                fpsNow = 0;
                timeLastInterval = timeNow;
            }

            bool doneSomething = false;
            int guardia = 0; // sicurezza da loop infinito

            // finche' il tempo simulato e' rimasto indietro rispetto
            // al tempo reale...
            while (nstep * PHYS_SAMPLING_STEP < timeNow) {
                car.DoStep();
                nstep++;
                doneSomething = true;
                timeNow = SDL_GetTicks();
                if (guardia++ > 1000) {
                    done = true;
                    break;
                } // siamo troppo lenti!
            }

            if (doneSomething)
                rendering(win);
                //redraw();
            else {
                // tempo libero!!!
            }
        }
    }
    SDL_GL_DeleteContext(mainContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return (0);
}

