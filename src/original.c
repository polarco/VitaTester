#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <vita2d.h>
#include "runtime.h"
#include "ui.h"
#include <psp2/display.h>

/* Font buffer */
extern unsigned int basicfont_size;
extern unsigned char basicfont[];

SceCtrlData     pad;
SceTouchData    touch;

signed char lx;
signed char ly;
signed char rx;
signed char ry;
int fxTouch;
int fyTouch;
int bxTouch;
int byTouch;

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)




#define BLACK   RGBA8(  0,   0,   0, 255)
#define WHITE   RGBA8(255, 255, 255, 255)
#define GREEN   RGBA8(  0, 255,   0, 255)
#define RED     RGBA8(255,   0,   0, 255)
#define BLUE    RGBA8(  0,   0, 255, 255)


static vita2d_texture *bg;
static vita2d_texture *cross;
static vita2d_texture *circle;
static vita2d_texture *square;
static vita2d_texture *triangle;
static vita2d_texture *select_texture;
static vita2d_texture *start;
static vita2d_texture *ltrigger;
static vita2d_texture *rtrigger;
static vita2d_texture *analog;
static vita2d_texture *dpad;
static vita2d_texture *frontTouch;
static vita2d_texture *backTouch;
void vt_original_init(void) {
    /* Setup image buffers */
    bg = vita2d_load_PNG_file("app0:/icons/background.png");
    cross = vita2d_load_PNG_file("app0:/icons/cross.png");
    circle = vita2d_load_PNG_file("app0:/icons/circle.png");
    square = vita2d_load_PNG_file("app0:/icons/square.png");
    triangle = vita2d_load_PNG_file("app0:/icons/triangle.png");
    select_texture = vita2d_load_PNG_file("app0:/icons/select.png");
    start = vita2d_load_PNG_file("app0:/icons/start.png");
    ltrigger = vita2d_load_PNG_file("app0:/icons/ltrigger.png");
    rtrigger = vita2d_load_PNG_file("app0:/icons/rtrigger.png");
    analog = vita2d_load_PNG_file("app0:/icons/analog.png");
    dpad = vita2d_load_PNG_file("app0:/icons/dpad.png");
    frontTouch = vita2d_load_PNG_file("app0:/icons/finger_gray.png");
    backTouch = vita2d_load_PNG_file("app0:/icons/finger_blue.png");

}
void vt_original_draw(vita2d_font *font,const VtSnapshot *snapshot,unsigned fps,bool stress) {
 pad=snapshot->pad;
        /* Display background */
        vita2d_draw_texture(bg, 0, 54);

        /* Display infos */


        vita2d_font_draw_textf(font, 10, 525, WHITE, 25, "Left: ( %3d, %3d )", pad.lx, pad.ly);
        vita2d_font_draw_textf(font, 780, 525, WHITE, 25, "Right: ( %3d, %3d )", pad.rx, pad.ry);

        /* Update joystick values */
        lx = (int)pad.lx - 128;
        ly = (int)pad.ly - 128;
        rx = (int)pad.rx - 128;
        ry = (int)pad.ry - 128;

        /* Draw and move left analog stick on screen */
        vita2d_draw_texture(analog, (85 + lx / 8), (285 + ly / 8));

        /* Draw and move right analog on screen */
        vita2d_draw_texture(analog, (802 + rx / 8), (285 + ry / 8));

        /* Draw the up directional button if pressed */
        if (pad.buttons & SCE_CTRL_UP) {
            vita2d_draw_texture(dpad, 59, 134);
        }

        /* Draw the down directional button if pressed */
        if (pad.buttons & SCE_CTRL_DOWN) {
            vita2d_draw_texture_rotate(dpad, 94, 231, 3.14f);
        }

        /* Draw the left directional button if pressed */
        if (pad.buttons & SCE_CTRL_LEFT) {
            vita2d_draw_texture_rotate(dpad, 65, 203, -1.57f);
        }

        /* Draw the right directional button if pressed */
        if (pad.buttons & SCE_CTRL_RIGHT) {
            vita2d_draw_texture_rotate(dpad, 123, 203, 1.57f);
        }

        /* Draw cross button if pressed */
        if (pad.buttons & SCE_CTRL_CROSS) {
            vita2d_draw_texture(cross, 830, 202);
        }

        /* Draw circle button if pressed */
        if (pad.buttons & SCE_CTRL_CIRCLE) {
            vita2d_draw_texture(circle, 869, 165);
        }

        /* Draw square button if pressed */
        if (pad.buttons & SCE_CTRL_SQUARE) {
            vita2d_draw_texture(square, 790, 165);
        }

        /* Draw triangle button if pressed */
        if (pad.buttons & SCE_CTRL_TRIANGLE) {
            vita2d_draw_texture(triangle, 830, 127);
        }

        /* Draw select button if pressed */
        if (pad.buttons & SCE_CTRL_SELECT) {
            vita2d_draw_texture(select_texture, 781, 375);
        }

        /* Draw start button if pressed */
        if (pad.buttons & SCE_CTRL_START) {
            vita2d_draw_texture(start, 841, 373);
        }

        /* Draw left trigger if pressed */
        if (pad.buttons & SCE_CTRL_LTRIGGER) {
            vita2d_draw_texture(ltrigger, 38, 40);
        }

        /* Draw right trigger if pressed */
        if (pad.buttons & SCE_CTRL_RTRIGGER) {
            vita2d_draw_texture(rtrigger, 720, 40);
        }

        /* Draw front touch on screen */
        if (stress) vt_draw_hud(font, snapshot, fps);
        else vita2d_font_draw_text(font,12,24,WHITE,19,"PS: voltar à LiveArea • Para fechar, encerre a bolha");
        touch=snapshot->front;
        for (unsigned i = 0; i < touch.reportNum; i++) {
            fxTouch = (lerp(touch.report[i].x, 1919, 960) - 50);
            fyTouch = (lerp(touch.report[i].y, 1087, 544) - 56.5);
            vita2d_draw_texture(frontTouch, fxTouch, fyTouch);
        }

        /* Draw rear touch on screen */
        touch=snapshot->back;
        for (unsigned i = 0; i < touch.reportNum; i++) {
            bxTouch = (lerp(touch.report[i].x, 1919, 960) - 50);
            byTouch = (lerp(touch.report[i].y, 1285, 855) - 113);
            vita2d_draw_texture(backTouch, bxTouch, byTouch);
        }

}
void vt_original_fini(void) {
vita2d_free_texture(bg);
vita2d_free_texture(cross);
vita2d_free_texture(circle);
vita2d_free_texture(square);
vita2d_free_texture(triangle);
vita2d_free_texture(select_texture);
vita2d_free_texture(start);
vita2d_free_texture(ltrigger);
vita2d_free_texture(rtrigger);
vita2d_free_texture(analog);
vita2d_free_texture(dpad);
vita2d_free_texture(frontTouch);
vita2d_free_texture(backTouch);
}
