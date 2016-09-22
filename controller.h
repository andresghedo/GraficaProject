/* 
 * File:   controller.h
 * Author: andrea
 *
 * Created on 5 settembre 2016, 15.47
 */
#include <SDL2/SDL_ttf.h>

class Controller {
public:

    enum {
        LEFT = 0, RIGHT = 1, ACC = 2, DEC = 3, NKEYS = 4
    };
    bool key[NKEYS];
    void Init();
    void EatKey(int keycode, int* keymap, bool pressed_or_released);
    void Joy(int keymap, bool pressed_or_released);
    void drawTargetCube(float mozzo);
    void checkVisibilityTarget(float carX, float carZ);
    void drawTriangleForTarget(float facing, float carX, float carZ);
    void drawLightTorciaStatua();
    float getTargetX();
    float getTargetZ();
    bool isTargetGoal();
    bool isTargetTnt();
    bool isGameOver();
    bool isPlayerLoose();
    int getScore();
    int getGoal();
    int getGoalChecked();
    int getTnt();
    int getTntChecked();
    void SDL_GL_DrawText(TTF_Font *font, char fgR, char fgG, char fgB, char fgA, char bgR, char bgG, char bgB, char bgA, char *text, int x, int y);
    void drawGameOverLayout(SDL_Window *win, TTF_Font *font, int scrH, int scrW);
    double getSeconds();

    Controller() {
        Init();
    } // costruttore
};

