#include <nds.h>
#include <filesystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NOTE_COUNT 180
#define BPM 111
#define HIT_WINDOW 125
#define TRAVEL_MS 1800
#define GAME_LENGTH_MS 56000
#define BEAT_RATE 8000

typedef struct { u32 timeMs; u8 lane; u8 player; u8 judged; } Note;
static Note chart[NOTE_COUNT];
static u8 *beatData = NULL;
static u32 beatSize = 0;
static int soundChannel = -1;
static int score, misses, combo, bestCombo;
static bool paused;
static const char *arrows[4] = {"<", "v", "^", ">"};

static void makeChart(void) {
    static const u8 pattern[16] = {0,2,1,3,0,1,2,3,1,0,3,2,0,2,3,1};
    for (int i=0;i<NOTE_COUNT;i++) {
        chart[i].timeMs = 900u + (u32)i * 310u;
        chart[i].lane = pattern[i % 16];
        chart[i].player = (i % 4 != 0 && i % 4 != 3);
        chart[i].judged = 0;
    }
}

static void loadBeat(void) {
    FILE *f = fopen("nitro:/beat.pcm", "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
    if (n > 0 && n <= 20000) {
        beatData = (u8*)malloc((u32)n);
        if (beatData) beatSize = (u32)fread(beatData, 1, (u32)n, f);
    }
    fclose(f);
}

static void startBeat(void) {
    soundChannel = soundPlaySample(gudBeatData, SoundFormat_8Bit, GUD_BEAT_DATA_SIZE, BEAT_RATE, 100, 64, true, 0);
}
static void stopBeat(void) {
    if (soundChannel >= 0) { soundKill(soundChannel); soundChannel = -1; }
}
static void waitRelease(void) {
    while (1) { scanKeys(); if (!(keysHeld() & (KEY_A|KEY_B|KEY_X|KEY_Y|KEY_TOUCH|KEY_START))) break; swiWaitForVBlank(); }
}
static void menuScreen(int selected) {
    consoleClear();
    iprintf("\x1b[1;4H================================");
    iprintf("\x1b[2;7H GUD FRIDAY NIGHT FUNKIN'");
    iprintf("\x1b[3;4H================================");
    iprintf("\x1b[5;8H MID-FIGHT MASSES");
    iprintf("\x1b[7;4H PRIMEIRO BUILD JOGAVEL - DS");
    iprintf("\x1b[10;8H%s STORY MODE",selected==0?">":" ");
    iprintf("\x1b[12;8H%s FREEPLAY",selected==1?">":" ");
    iprintf("\x1b[14;8H%s CREDITOS",selected==2?">":" ");
    iprintf("\x1b[18;3HSetas UP/DOWN + A para escolher");
    iprintf("\x1b[20;3HA/B/X/Y ou toque para jogar");
    iprintf("\x1b[23;7Hby ivtguioliveira8");
}
static int readLane(u16 k, touchPosition t) {
    if(k&KEY_LEFT || k&KEY_A) return 0;
    if(k&KEY_DOWN || k&KEY_B) return 1;
    if(k&KEY_UP || k&KEY_X) return 2;
    if(k&KEY_RIGHT || k&KEY_Y) return 3;
    if(k&KEY_TOUCH) return (t.px*4)/256;
    return -1;
}
static void hitLane(int lane,u32 now) {
    int best=-1; u32 bestDiff=HIT_WINDOW+1;
    for(int i=0;i<NOTE_COUNT;i++) {
        if(chart[i].judged || !chart[i].player || chart[i].lane!=lane) continue;
        u32 d=chart[i].timeMs>now?chart[i].timeMs-now:now-chart[i].timeMs;
        if(d<=HIT_WINDOW && d<bestDiff){best=i;bestDiff=d;}
    }
    if(best>=0){chart[best].judged=1;score+=350-(int)bestDiff;combo++;if(combo>bestCombo)bestCombo=combo;}
    else {misses++;combo=0;}
}
static void drawGame(u32 now) {
    consoleClear();
    iprintf("GUD FNF  |  PARISH - RHYTHM STAGE");
    iprintf("\x1b[2;1HScore:%d  Combo:%d  Miss:%d",score,combo,misses);
    iprintf("\x1b[4;1HOPONENTE                 BOYFRIEND");
    iprintf("\x1b[6;1H  L  D  U  R               L  D  U  R");
    for(int i=0;i<NOTE_COUNT;i++) {
        Note *n=&chart[i]; if(n->judged) continue;
        int d=(int)n->timeMs-(int)now;
        if(d>TRAVEL_MS || d< -HIT_WINDOW) continue;
        int row=17-(TRAVEL_MS-d)/165; if(row<7)row=7; if(row>17)row=17;
        int col=n->player?20+n->lane*3:2+n->lane*3;
        iprintf("\x1b[%d;%dH%s",row,col,arrows[n->lane]);
    }
    iprintf("\x1b[19;1H  L   D   U   R       L   D   U   R");
    iprintf("\x1b[21;1HSetas / A B X Y / toque em uma das 4 faixas");
    if(paused) iprintf("\x1b[12;13HPAUSADO");
}
static void resultScreen(void) {
    consoleClear();
    iprintf("\x1b[3;9HGUD FNF - RESULTADO");
    iprintf("\x1b[7;7HScore: %d",score);
    iprintf("\x1b[9;7HMisses: %d",misses);
    iprintf("\x1b[11;7HMaior combo: %d",bestCombo);
    iprintf("\x1b[15;3HAperte A para voltar ao menu");
}
static void playGame(void) {
    score=misses=combo=bestCombo=0; paused=false; makeChart();
    consoleClear(); iprintf("\x1b[9;10HREADY?"); for(int i=0;i<35;i++)swiWaitForVBlank();
    iprintf("\x1b[9;10HSET!"); for(int i=0;i<35;i++)swiWaitForVBlank();
    iprintf("\x1b[9;10HGO!"); startBeat();
    u32 frames=0; bool done=false;
    while(!done) {
        scanKeys(); u16 k=keysDown(); touchPosition t; touchRead(&t);
        if(k&KEY_START){paused=!paused;stopBeat();if(!paused)startBeat();}
        if(k&KEY_SELECT) break;
        if(!paused) {
            u32 now=(frames*1000u)/60u; int lane=readLane(k,t); if(lane>=0)hitLane(lane,now);
            for(int i=0;i<NOTE_COUNT;i++) {
                if(!chart[i].judged && chart[i].player && now>chart[i].timeMs+HIT_WINDOW){chart[i].judged=1;misses++;combo=0;}
                if(!chart[i].judged && !chart[i].player && now>=chart[i].timeMs)chart[i].judged=1;
            }
            drawGame(now); if(now>=GAME_LENGTH_MS)done=true; frames++;
        } else drawGame((frames*1000u)/60u);
        swiWaitForVBlank();
    }
    stopBeat(); resultScreen(); waitRelease();
    while(1){scanKeys();if(keysDown()&(KEY_A|KEY_B|KEY_START))break;swiWaitForVBlank();}
}
static void creditsScreen(void) {
    consoleClear(); iprintf("\x1b[3;10HCREDITOS");
    iprintf("\x1b[7;2HFriday Night Funkin' - equipe original");
    iprintf("\x1b[9;2HMid-Fight Masses - Dokki.doodlez");
    iprintf("\x1b[11;2HProjeto de fã nao oficial");
    iprintf("\x1b[14;2HCriador desta versao: ivtguioliveira8");
    iprintf("\x1b[19;2HAperte B para voltar");
    while(1){scanKeys();if(keysDown()&(KEY_B|KEY_A))break;swiWaitForVBlank();}
}
int main(void) {
    lcdMainOnTop(); consoleDemoInit(); soundEnable();
    int selected=0;
    while(1){
        menuScreen(selected); scanKeys(); u16 k=keysDown();
        if(k&KEY_UP)selected=(selected+2)%3;
        if(k&KEY_DOWN)selected=(selected+1)%3;
        if(k&KEY_A){waitRelease();if(selected==2)creditsScreen();else playGame();}
        if(k&KEY_TOUCH){waitRelease();playGame();}
        swiWaitForVBlank();
    }
    return 0;
}
