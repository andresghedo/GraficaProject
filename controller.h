/*
 *  CLASSE Controller
 * 
 *  Espone i metodi della classe Controller
 * 
 */
#include <SDL2/SDL_ttf.h>

class Controller {
public:
    /* enumerato che mappa gli input utente */
    enum { LEFT = 0, RIGHT = 1, ACC = 2, DEC = 3, NKEYS = 4 };
    /* array di booleani che indicano i pulsanti premuti dall'utente */
    bool key[NKEYS];
    /* inizializzazione */
    void Init();
    /* memorizzazione del tasto premuto dal player */
    void EatKey(int keycode, int* keymap, bool pressed_or_released);
    /* gestione joystick */
    void Joy(int keymap, bool pressed_or_released);
    /* disegno di un cubo goal o tnt */
    void drawTargetCube();
    /* gestione dei vincli di gioco(cattura goal/tnt, time, fine gioco etc...) */
    void checkConstraintsGame(float carZ);
    /* disegno del mirino */
    void drawMirino(float facing, float carX, float carZ, bool draw);
    /* disegno della luce di retromarcia/freno */
    void drawReverseLight(float facing, float carX, float carZ, bool retroLight);
    /* disegno della luca della statua intermittente */
    void drawLightTorciaStatua();
    /* fornisce all'esterno la posizione X del target corrente */
    float getTargetX();
    /* fornisce all'esterno la posizione Z del target corrente */
    float getTargetZ();
    /* fornisce all'esterno la posizione X del mirino corrente */
    float getMirinoX();
    /* fornisce all'esterno la posizione Z del mirino corrente */
    float getMirinoZ();
    /* fornisce all'esterno la posizione Y del mirino corrente */
    float getMirinoY();
    /* booleano che decreta se il target è un Goal o meno */
    bool isTargetGoal();
    /* booleano che decreta se il target è un TNT o meno */
    bool isTargetTnt();
    /* booleano che decreta la fine del gioco */
    bool isGameOver();
    /* booleano che decreta se il è scaduto il time */
    bool isPlayerLoose();
    /* ritorna lo score del player corrente */
    int getScore();
    /* ritorna il numero di Goal generati */
    int getGoal();
    /* ritorna il numero di Goal catturati */
    int getGoalChecked();
    /* ritorna il numero di TNT generati */
    int getTnt();
    /* ritorna il numero di TNT fatti esplodere */
    int getTntChecked();
    /* funzione che renderizza del testo in un box */
    void drawText(TTF_Font *font, char fgR, char fgG, char fgB, char fgA, char bgR, char bgG, char bgB, char bgA, char *text, int x, int y);
    /* disegna il layout di GameOver */
    void drawGameOverLayout(SDL_Window *win, TTF_Font *font, int scrH, int scrW);
    /* torna il numero di secondi di gioco correnti */
    double getSeconds();

    /* costruttore */
    Controller() {
        Init();
    }
};

