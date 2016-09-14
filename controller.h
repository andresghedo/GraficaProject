/* 
 * File:   controller.h
 * Author: andrea
 *
 * Created on 5 settembre 2016, 15.47
 */

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
    void checkVisibilityTarget(float carX, float carY, float carZ);
    void drawTriangleForTarget(float facing, float carX, float carZ);
    void drawLightTorciaStatua();
    void drawRecinzione();

    Controller() {
        Init();
    } // costruttore
};

