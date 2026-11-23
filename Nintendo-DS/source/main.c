#include <nds.h>
#include <stdio.h>
#include <stdbool.h>

#define NOTE_COUNT 24
#define HIT_WINDOW 8
#define LANE_COUNT 4

typedef struct {
    int frame;
    int lane;
    bool opponent;
    bool judged;
} Note;

static Note chart[NOTE_COUNT] = {
    {90,0,0,0},{108,2,0,0},{126,1,1,0},{144,3,0,0},
    {162,0,0,0},{180,1,0,0},{198,2,1,0},{216,3,0,0},
    {234,2,0,0},{252,0,1,0},{270,1,0,0},{288,3,0,0},
    {306,0,0,0},{324,2,0,0},{342,1,1,0},{360,3,0,0},
    {378,1,0,0},{396,0,0,0},{414,3,1,0},{432,2,0,0},
    {450,0,0,0},{468,1,0,0},{486,2,0,0},{504,3,0,0}
};

static int score = 0;
static int misses = 0;
static int combo = 0;
static int frameCount = 0;
static int judgedCount = 0;
static bool paused = false;

static const char *laneName[LANE_COUNT] = {"LEFT","DOWN","UP","RIGHT"};

static void resetGame(void) {
    int i;
    score = 0;
    misses = 0;
    combo = 0;
    frameCount = 0;
    judgedCount = 0;
    paused = false;
    for (i = 0; i < NOTE_COUNT; i++) chart[i].judged = false;
}

static int inputLane(u16 keys, touchPosition touch) {
    if (keys & KEY_LEFT) return 0;
    if (keys & KEY_DOWN) return 1;
    if (keys & KEY_UP) return 2;
    if (keys & KEY_RIGHT) return 3;

    if (keys & KEY_TOUCH) {
        if (touch.px < 64) return 0;
        if (touch.px < 128) return 1;
        if (touch.px < 192) return 2;
        return 3;
    }
    return -1;
}

static void showMenu(void) {
    consoleClear();
    iprintf("\x1b[2;5HGUD FNF - NINTENDO DS");
    iprintf("\x1b[5;3HFRIDAY NIGHT FUNKIN FAN GAME");
    iprintf("\x1b[8;2HPressione A ou toque para jogar");
    iprintf("\x1b[11;2HSetas: acertar notas");
    iprintf("\x1b[12;2HTela touch: quatro pistas");
    iprintf("\x1b[13;2HSTART: pausar");
    iprintf("\x1b[17;2HPrototipo DS - BPM 100");
    iprintf("\x1b[19;2HCriador: ivtguioliveira8");
}

static void drawGame(void) {
    int i;
    consoleClear();
    iprintf("\x1b[0;1HGUD FNF  |  DEMO DS");
    iprintf("\x1b[2;1HScore: %d   Misses: %d", score, misses);
    iprintf("\x1b[3;1HCombo: %d", combo);
    iprintf("\x1b[5;1HOPPONENT        PLAYER");
    iprintf("\x1b[7;1H   |   |   |       |   |   |");
    for (i = 0; i < NOTE_COUNT; i++) {
        int delta;
        int row;
        if (chart[i].judged) continue;
        delta = chart[i].frame - frameCount;
        if (delta > 54 || delta < -HIT_WINDOW) continue;
        row = 14 - (delta / 6);
        if (row < 8) row = 8;
        if (row > 16) row = 16;
        if (chart[i].opponent) {
            iprintf("\x1b[%d;%dH%c", row, 2 + chart[i].lane * 4, '*');
        } else {
            iprintf("\x1b[%d;%dH%c", row, 21 + chart[i].lane * 2, '^');
        }
    }
    iprintf("\x1b[18;1H L      D      U      R");
    iprintf("\x1b[20;1HToque na faixa ou use as setas");
    if (paused) iprintf("\x1b[10;10HPAUSADO");
}

static void judgeLane(int lane) {
    int i;
    int best = -1;
    int bestDelta = HIT_WINDOW + 1;
    for (i = 0; i < NOTE_COUNT; i++) {
        int delta;
        if (chart[i].judged || chart[i].opponent || chart[i].lane != lane) continue;
        delta = chart[i].frame - frameCount;
        if (delta < 0) delta = -delta;
        if (delta <= HIT_WINDOW && delta < bestDelta) {
            best = i;
            bestDelta = delta;
        }
    }
    if (best >= 0) {
        chart[best].judged = true;
        score += 100 + combo * 5;
        combo++;
        judgedCount++;
    } else {
        misses++;
        combo = 0;
    }
}

int main(void) {
    touchPosition touch;
    consoleDemoInit();
    lcdMainOnBottom();
    showMenu();

    while (1) {
        scanKeys();
        u16 down = keysDown();
        if (down & KEY_A || down & KEY_TOUCH) {
            resetGame();
            break;
        }
        swiWaitForVBlank();
    }

    while (1) {
        scanKeys();
        u16 down = keysDown();
        touchRead(&touch);

        if (down & KEY_START) paused = !paused;
        if (!paused) {
            int lane = inputLane(down, touch);
            if (lane >= 0) judgeLane(lane);

            if (frameCount > 0) {
                int i;
                for (i = 0; i < NOTE_COUNT; i++) {
                    if (!chart[i].judged && !chart[i].opponent &&
                        frameCount > chart[i].frame + HIT_WINDOW) {
                        chart[i].judged = true;
                        misses++;
                        combo = 0;
                    }
                    if (!chart[i].judged && chart[i].opponent &&
                        frameCount >= chart[i].frame) {
                        chart[i].judged = true;
                    }
                }
            }
            frameCount++;
        }

        drawGame();
        if (judgedCount + misses >= 18 || frameCount > 620) {
            consoleClear();
            iprintf("\x1b[4;7HGUD FNF - RESULTADO");
            iprintf("\x1b[8;5HScore: %d", score);
            iprintf("\x1b[9;5HMisses: %d", misses);
            iprintf("\x1b[10;5HMaior combo: %d", combo);
            iprintf("\x1b[14;2HAperte A para voltar ao menu");
            while (1) {
                scanKeys();
                if (keysDown() & KEY_A) {
                    resetGame();
                    showMenu();
                    while (1) {
                        scanKeys();
                        if (keysDown() & (KEY_A | KEY_TOUCH)) break;
                        swiWaitForVBlank();
                    }
                    break;
                }
                swiWaitForVBlank();
            }
        }
        swiWaitForVBlank();
    }
    return 0;
}
