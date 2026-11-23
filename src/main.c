#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include "gud_chart.h"

PSP_MODULE_INFO("GUD FNF PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define SCREEN_W 480
#define SCREEN_H 272
#define LANE_W 58
#define LANE_GAP 8
#define LANE_X(i) (106 + (i) * (LANE_W + LANE_GAP))
#define STRIKE_Y 218
#define NOTE_TRAVEL_MS 1800
#define HIT_WINDOW_MS 145

typedef struct { unsigned short x,y,z; unsigned int color; } Vertex;
static unsigned int __attribute__((aligned(16))) list[262144];
static int score=0, misses=0, combo=0, best=0, paused=0, finished=0;
static unsigned int judged[GUD_TUTORIAL_NOTE_COUNT];
static unsigned int start_ms=0;
static unsigned int pause_at=0;

static unsigned int now_ms(void) {
    return (unsigned int)(sceKernelGetSystemTimeWide()/1000ULL);
}
static void rect(int x,int y,int w,int h,unsigned int color) {
    Vertex *v=(Vertex*)sceGuGetMemory(2*sizeof(Vertex));
    v[0].x=x; v[0].y=y; v[0].z=0; v[0].color=color;
    v[1].x=x+w; v[1].y=y+h; v[1].z=0; v[1].color=color;
    sceGuDrawArray(GU_SPRITES,GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,v);
}
static void begin_frame(void) {
    sceGuStart(GU_DIRECT,list);
    sceGuClearColor(0xff10101c);
    sceGuClear(GU_COLOR_BUFFER_BIT);
}
static void end_frame(void) {
    sceGuFinish(); sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}
static void reset_game(void) {
    memset(judged,0,sizeof(judged));
    score=misses=combo=best=paused=finished=0;
    start_ms=now_ms();
}
static unsigned int song_time(void) {
    unsigned int n=now_ms();
    return n>=start_ms?n-start_ms:0;
}
static void judge_lane(int lane,unsigned int t) {
    unsigned int i, best_i=0, best_d=0xffffffff;
    for(i=0;i<GUD_TUTORIAL_NOTE_COUNT;i++) {
        if(judged[i] || GUD_TUTORIAL_CHART[i].lane!=lane) continue;
        int diff=(int)GUD_TUTORIAL_CHART[i].time_ms-(int)t;
        unsigned int d=(unsigned int)(diff<0?-diff:diff);
        if(d<best_d){best_d=d;best_i=i;}
    }
    if(best_d<=HIT_WINDOW_MS) {
        judged[best_i]=1; score+=350; combo++;
        if(combo>best)best=combo;
    } else { combo=0; }
}
static void draw_scene(unsigned int t) {
    int i;
    /* stage-like background and four play lanes */
    rect(0,0,480,272,0xff171522);
    rect(0,0,480,34,0xff29243a);
    rect(0,246,480,26,0xff29243a);
    for(i=0;i<4;i++) {
        unsigned int c=(i==0?0xffd65b75:i==1?0xff58b9d4:i==2?0xffa58be8:0xff70d69b);
        rect(LANE_X(i),38,LANE_W,192,0xff242334);
        rect(LANE_X(i),STRIKE_Y,LANE_W,5,c);
        rect(LANE_X(i)+18,STRIKE_Y-17,22,22,c);
    }
    for(i=0;i<(int)GUD_TUTORIAL_NOTE_COUNT;i++) {
        if(judged[i])continue;
        int dt=(int)GUD_TUTORIAL_CHART[i].time_ms-(int)t;
        if(dt>NOTE_TRAVEL_MS || dt< -180)continue;
        int y=STRIKE_Y-(dt*180)/NOTE_TRAVEL_MS;
        unsigned int c=(GUD_TUTORIAL_CHART[i].lane==0?0xffd65b75:
                        GUD_TUTORIAL_CHART[i].lane==1?0xff58b9d4:
                        GUD_TUTORIAL_CHART[i].lane==2?0xffa58be8:0xff70d69b);
        rect(LANE_X(GUD_TUTORIAL_CHART[i].lane)+7,y,LANE_W-14,17,c);
        rect(LANE_X(GUD_TUTORIAL_CHART[i].lane)+12,y+4,LANE_W-24,3,0xffffffff);
    }
    /* compact status bars */
    rect(12,12,score>0?(score/150>200?200:score/150):2,7,0xff75d99a);
    rect(12,258,misses*8>200?200:misses*8,5,0xffd65b75);
}
int main(void) {
    SceCtrlData pad,old;
    memset(&old,0,sizeof(old));
    sceCtrlSetSamplingCycle(0); sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    pspDebugScreenInit();
    sceGuInit();
    sceGuStart(GU_DIRECT,list);
    sceGuDrawBuffer(GU_PSM_8888,(void*)0,512);
    sceGuDispBuffer(SCREEN_W,SCREEN_H,(void*)0x88000,512);
    sceGuDepthBuffer((void*)0x110000,512);
    sceGuOffset(2048-(SCREEN_W/2),2048-(SCREEN_H/2));
    sceGuViewport(2048,2048,SCREEN_W,SCREEN_H);
    sceGuDepthRange(65535,0);
    sceGuScissor(0,0,SCREEN_W,SCREEN_H);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDisable(GU_DEPTH_TEST);
    sceGuDisable(GU_TEXTURE_2D);
    sceGuShadeModel(GU_FLAT);
    sceGuFinish(); sceGuSync(0,0); sceGuDisplay(GU_TRUE);
    reset_game();
    while(1) {
        sceCtrlReadBufferPositive(&pad,1);
        unsigned int pressed=pad.Buttons & ~old.Buttons;
        if(pressed & PSP_CTRL_SELECT)break;
        if(pressed & PSP_CTRL_START) {
            if(!paused){paused=1;pause_at=now_ms();}
            else {start_ms += now_ms()-pause_at;paused=0;}
        }
        if(pressed & PSP_CTRL_CROSS && finished)reset_game();
        if(!paused && !finished) {
            unsigned int t=song_time();
            if(pressed & PSP_CTRL_LEFT)judge_lane(0,t);
            if(pressed & PSP_CTRL_DOWN)judge_lane(1,t);
            if(pressed & PSP_CTRL_UP)judge_lane(2,t);
            if(pressed & PSP_CTRL_RIGHT)judge_lane(3,t);
            unsigned int i;
            for(i=0;i<GUD_TUTORIAL_NOTE_COUNT;i++)
                if(!judged[i] && t>GUD_TUTORIAL_CHART[i].time_ms+HIT_WINDOW_MS) {
                    judged[i]=1;misses++;combo=0;
                }
            if(t>GUD_TUTORIAL_CHART[GUD_TUTORIAL_NOTE_COUNT-1].time_ms+2500)finished=1;
        }
        begin_frame(); draw_scene(song_time());
        if(paused)rect(150,104,180,48,0xee000000);
        if(finished)rect(120,94,240,76,0xee111122);
        end_frame();
        old=pad;
    }
    sceGuTerm();
    sceKernelExitGame();
    return 0;
}
