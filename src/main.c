#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO("GUD FNF PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define LANES 4
#define MAX_NOTES 32
#define HIT_Y 220
#define NOTE_SPEED 2.7f

typedef struct {
    int lane;
    float y;
    int active;
    int judged;
} Note;

static Note notes[MAX_NOTES];
static int score = 0, misses = 0, combo = 0;
static unsigned int frame_count = 0;
static int running = 1;
static const char *lane_names[LANES] = {"LEFT", "DOWN", "UP", "RIGHT"};

static void reset_game(void) {
    memset(notes, 0, sizeof(notes));
    score = 0;
    misses = 0;
    combo = 0;
    frame_count = 0;
}

static void spawn_note(void) {
    int i;
    for (i = 0; i < MAX_NOTES; ++i) {
        if (!notes[i].active) {
            notes[i].active = 1;
            notes[i].judged = 0;
            notes[i].lane = rand() % LANES;
            notes[i].y = 30.0f;
            return;
        }
    }
}

static void hit_lane(int lane) {
    int i, best = -1;
    float best_dist = 9999.0f;
    for (i = 0; i < MAX_NOTES; ++i) {
        if (notes[i].active && !notes[i].judged && notes[i].lane == lane) {
            float d = notes[i].y - (float)HIT_Y;
            if (d < 0) d = -d;
            if (d < best_dist) {
                best_dist = d;
                best = i;
            }
        }
    }
    if (best >= 0 && best_dist <= 34.0f) {
        notes[best].judged = 1;
        notes[best].active = 0;
        score += 350;
        combo++;
    }
}

static void update_game(void) {
    int i;
    frame_count++;
    if (frame_count % 48 == 0) spawn_note();

    for (i = 0; i < MAX_NOTES; ++i) {
        if (!notes[i].active) continue;
        notes[i].y += NOTE_SPEED;
        if (notes[i].y > 255.0f) {
            notes[i].active = 0;
            misses++;
            combo = 0;
        }
    }
}

static void draw_game(void) {
    int i;
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenPrintf("GUD FNF PSP - PROTOTYPE\n");
    pspDebugScreenPrintf("SCORE: %d   COMBO: %d   MISSES: %d\n\n", score, combo, misses);
    pspDebugScreenPrintf("      LEFT     DOWN      UP    RIGHT\n");
    for (i = 0; i < LANES; ++i) {
        pspDebugScreenPrintf("      [ %s ]", lane_names[i]);
    }
    pspDebugScreenPrintf("\n\n");
    for (i = 0; i < MAX_NOTES; ++i) {
        if (notes[i].active) {
            pspDebugScreenPrintf("NOTE lane=%d  y=%d\n", notes[i].lane, (int)notes[i].y);
        }
    }
    pspDebugScreenPrintf("\nD-PAD: hit notes | START: restart | SELECT: exit\n");
}

int main(void) {
    SceCtrlData pad, old_pad;
    memset(&old_pad, 0, sizeof(old_pad));
    pspDebugScreenInit();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    srand(12345);
    reset_game();

    while (running) {
        sceCtrlReadBufferPositive(&pad, 1);
        unsigned int pressed = pad.Buttons & ~old_pad.Buttons;

        if (pressed & PSP_CTRL_SELECT) running = 0;
        if (pressed & PSP_CTRL_START) reset_game();
        if (pressed & PSP_CTRL_LEFT) hit_lane(0);
        if (pressed & PSP_CTRL_DOWN) hit_lane(1);
        if (pressed & PSP_CTRL_UP) hit_lane(2);
        if (pressed & PSP_CTRL_RIGHT) hit_lane(3);

        update_game();
        pspDebugScreenClear();
        draw_game();
        sceDisplayWaitVblankStart();
        old_pad = pad;
    }
    sceKernelExitGame();
    return 0;
}
