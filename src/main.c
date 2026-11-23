#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include "gud_chart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

PSP_MODULE_INFO("GUD FNF PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define W 480
#define H 272
#define MAX_NOTES 2048
#define HIT_MS 145
#define TRAVEL_MS 1750
#define PLAYER_X 260
#define OPP_X 22
#define LANE_W 43
#define GAP 5
#define STRIKE_Y 210

typedef struct { int time, lane, sustain, player, judged, hit; } Note;
static Note notes[MAX_NOTES];
static int note_count=0, score=0, misses=0, combo=0, best=0, health=50;
static int game_state=0, paused=0, finished=0;
static Uint32 song_start=0, pause_tick=0;
static SDL_Surface *screen_surface;
static SDL_Surface *menu_bg,*logo,*stage_bg,*stage_pillars,*stage_floor,*bf,*sarvente,*note_atlas;
static Mix_Music *music=NULL;
static SDL_Rect bf_idle={505,244,241,244};
static SDL_Rect sarvente_idle={1422,1986,441,760};
static SDL_Rect arrows[4]={{0,398,154,157},{0,240,158,154},{162,240,157,154},{647,397,154,157}};

static const unsigned char glyphs[36][5]={
{0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},{0x7f,0x41,0x41,0x22,0x1c},
{0x7f,0x49,0x49,0x49,0x41},{0x7f,0x09,0x09,0x09,0x01},{0x3e,0x41,0x49,0x49,0x7a},{0x7f,0x08,0x08,0x08,0x7f},
{0x00,0x41,0x7f,0x41,0x00},{0x20,0x40,0x41,0x3f,0x01},{0x7f,0x08,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
{0x7f,0x02,0x0c,0x02,0x7f},{0x7f,0x04,0x08,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},{0x7f,0x09,0x09,0x09,0x06},
{0x3e,0x41,0x51,0x21,0x5e},{0x7f,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7f,0x01,0x01},
{0x3f,0x40,0x40,0x40,0x3f},{0x1f,0x20,0x40,0x20,0x1f},{0x7f,0x20,0x18,0x20,0x7f},{0x63,0x14,0x08,0x14,0x63},
{0x03,0x04,0x78,0x04,0x03},{0x61,0x51,0x49,0x45,0x43},
{0x3e,0x51,0x49,0x45,0x3e},{0x00,0x42,0x7f,0x40,0x00},{0x62,0x51,0x49,0x49,0x46},{0x22,0x41,0x49,0x49,0x36},
{0x18,0x14,0x12,0x7f,0x10},{0x2f,0x49,0x49,0x49,0x31},{0x3e,0x49,0x49,0x49,0x32},{0x01,0x71,0x09,0x05,0x03},
{0x36,0x49,0x49,0x49,0x36},{0x26,0x49,0x49,0x49,0x3e}
};
static void text(const char *s,int x,int y,int size,SDL_Color color){
    int scale=size/8; if(scale<1)scale=1; if(scale>3)scale=3;
    
    for(;*s;s++,x+=6*scale){
        char ch=*s; int idx=-1;
        if(ch>='A'&&ch<='Z')idx=ch-'A';
        else if(ch>='a'&&ch<='z')idx=ch-'a';
        else if(ch>='0'&&ch<='9')idx=26+ch-'0';
        if(idx>=0){
            for(int cx=0;cx<5;cx++)for(int cy=0;cy<7;cy++)
                if(glyphs[idx][cx]&(1<<cy)){
                    SDL_Rect q={x+cx*scale,y+cy*scale,scale,scale};
                    SDL_FillRect(screen_surface,&q,SDL_MapRGB(screen_surface->format,color.r,color.g,color.b));
                }
        }else if(ch==':'){
            SDL_Rect q1={x+2*scale,y+2*scale,scale,scale},q2={x+2*scale,y+5*scale,scale,scale};
            SDL_FillRect(screen_surface,&q1,SDL_MapRGB(screen_surface->format,color.r,color.g,color.b));SDL_FillRect(screen_surface,&q2,SDL_MapRGB(screen_surface->format,color.r,color.g,color.b));
        }else if(ch=='-'){
            SDL_Rect q={x+1*scale,y+3*scale,4*scale,scale};SDL_FillRect(screen_surface,&q,SDL_MapRGB(screen_surface->format,color.r,color.g,color.b));
        }
    }
}
static SDL_Surface *load(const char *p){return IMG_Load(p);}
static void image(SDL_Surface *t,int x,int y,int w,int h){
    if(!t)return; SDL_Rect d={x,y,w,h}; SDL_BlitSurface(t,NULL,screen_surface,&d);
}
static void crop(SDL_Surface *t,SDL_Rect src,int x,int y,int w,int h){
    if(!t)return; SDL_Rect d={x,y,w,h}; SDL_BlitSurface(t,&src,screen_surface,&d);
}
static void fill(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b,Uint8 a){
    SDL_Rect q={x,y,w,h}; SDL_FillRect(screen_surface,&q,SDL_MapRGB(screen_surface->format,r,g,b));
}
static int load_chart(const char *path){
    (void)path;
    note_count=(int)GUD_TUTORIAL_NOTE_COUNT;
    if(note_count>MAX_NOTES)note_count=MAX_NOTES;
    for(int i=0;i<note_count;i++){
        notes[i].time=(int)GUD_TUTORIAL_CHART[i].time_ms;
        notes[i].lane=GUD_TUTORIAL_CHART[i].lane;
        notes[i].sustain=GUD_TUTORIAL_CHART[i].sustain_ms;
        notes[i].player=GUD_TUTORIAL_CHART[i].player;
        notes[i].judged=0;
        notes[i].hit=0;
    }
    return note_count;
}
static void reset_game(void){
    score=misses=combo=best=0;health=50;paused=finished=0;
    for(int i=0;i<note_count;i++)notes[i].judged=notes[i].hit=0;
    song_start=SDL_GetTicks();
}
static Uint32 song_time(void){return paused?pause_tick-song_start:SDL_GetTicks()-song_start;}
static void start_song(void){
    if(music){Mix_HaltMusic();Mix_FreeMusic(music);music=NULL;}
    music=Mix_LoadMUS("assets/tutorial-remix/Inst.ogg");
    if(music)Mix_PlayMusic(music,0);
    reset_game();game_state=1;
}
static void judge(int lane){
    int t=(int)song_time(), found=-1, bestdiff=999999;
    for(int i=0;i<note_count;i++){
        Note *n=&notes[i]; if(!n->player||n->judged||n->lane!=lane)continue;
        int d=abs(n->time-t); if(d<bestdiff){bestdiff=d;found=i;}
    }
    if(found>=0&&bestdiff<=HIT_MS){
        notes[found].judged=notes[found].hit=1;
        score+=bestdiff<45?350:bestdiff<90?200:100;
        combo++; if(combo>best)best=combo; if(health<100)health+=1;
    }else{combo=0;misses++;health-=5;}
}
static void draw_lanes(int x,int player){
    for(int l=0;l<4;l++){
        int px=x+l*(LANE_W+GAP);
        fill(px,57,LANE_W,178,24,23,39,255);
        crop(note_atlas,arrows[l],px+2,STRIKE_Y-17,LANE_W-4,34);

    }
}
static void draw_game(void){
    image(stage_bg,0,0,W,H);
    image(stage_pillars,0,0,W,H);
    image(stage_floor,0,170,W,102);
    crop(sarvente,sarvente_idle,45,66,105,155);
    crop(bf,bf_idle,348,100,95,105);
    draw_lanes(OPP_X,0); draw_lanes(PLAYER_X,1);
    Uint32 t=song_time();
    for(int i=0;i<note_count;i++){
        Note *n=&notes[i]; if(n->judged&&n->player)continue;
        int dt=n->time-(int)t;
        if(dt>TRAVEL_MS||dt< -HIT_MS)continue;
        int x=(n->player?PLAYER_X:OPP_X)+n->lane*(LANE_W+GAP)+5;
        int y=STRIKE_Y-(dt*150)/TRAVEL_MS;
        if(n->sustain>0){
            int tail=(n->sustain*150)/TRAVEL_MS;
            fill(x+12,y-tail,14,tail,190,190,255,180);
        }
        crop(note_atlas,arrows[n->lane],x,y,LANE_W-10,32);
    }
    fill(15,12,450,8,50,48,67,255); fill(15,12,health*4,8,95,220,145,255);
    char s[128]; snprintf(s,sizeof(s),"SCORE %d   COMBO %d   MISSES %d",score,combo,misses);
    text(s,16,25,12,(SDL_Color){255,255,255,255});
    text("START: PAUSE   SELECT: MENU",155,254,10,(SDL_Color){220,220,230,255});
}
static void draw_menu(void){
    image(menu_bg,0,0,W,H);
    fill(0,0,W,H,10,8,20,110);
    image(logo,120,20,240,92);
    text("GUD FNF PSP",164,112,23,(SDL_Color){255,255,255,255});
    text("FREEPLAY",190,157,20,(SDL_Color){255,255,255,255});
    text("TUTORIAL REMIX",160,188,16,(SDL_Color){120,235,255,255});
    text("X / CROSS: JOGAR",168,226,13,(SDL_Color){255,255,255,255});
    text("D-PAD: NOTAS    START: PAUSA",110,250,11,(SDL_Color){230,230,240,255});
}
static void draw_end(void){
    fill(45,58,390,150,10,10,25,235);
    text("MUSICA FINALIZADA",115,78,22,(SDL_Color){255,255,255,255});
    char s[100];snprintf(s,sizeof(s),"SCORE %d   MISSES %d   MELHOR COMBO %d",score,misses,best);
    text(s,85,122,14,(SDL_Color){170,235,255,255});
    text("CROSS: JOGAR DE NOVO   CIRCLE: MENU",70,165,12,(SDL_Color){255,255,255,255});
}
int main(int argc, char *argv[]){
    (void)argc; (void)argv;
    sceCtrlSetSamplingCycle(0); sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER)<0)return 1;
    Mix_OpenAudio(44100,AUDIO_S16SYS,2,2048);
    screen_surface=SDL_SetVideoMode(W,H,32,SDL_HWSURFACE|SDL_DOUBLEBUF);
    if(!screen_surface)return 2;
    menu_bg=load("assets/menuBG.png"); logo=load("assets/logo.png");
    stage_bg=load("assets/churchlight/bg.png");stage_pillars=load("assets/churchlight/pillars.png");stage_floor=load("assets/churchlight/floor.png");
    bf=load("assets/bf/Bf.png");sarvente=load("assets/classiccharacters/sarvente_sheet.png");note_atlas=load("assets/NOTE_assets.png");
    load_chart("assets/tutorial-remix/tutorial-remix.json");
    SceCtrlData pad,old; memset(&old,0,sizeof(old));
    int running=1;
    while(running){
        sceCtrlReadBufferPositive(&pad,1);
        unsigned int p=pad.Buttons&~old.Buttons;
        if(p&PSP_CTRL_SELECT){if(game_state==1){Mix_HaltMusic();game_state=0;}else running=0;}
        if(game_state==0){if(p&PSP_CTRL_CROSS)start_song();}
        else if(game_state==1){
            if(p&PSP_CTRL_START){if(!paused){paused=1;pause_tick=SDL_GetTicks();Mix_PauseMusic();}else{song_start+=SDL_GetTicks()-pause_tick;paused=0;Mix_ResumeMusic();}}
            if(!paused){
                if(p&PSP_CTRL_LEFT)judge(0); if(p&PSP_CTRL_DOWN)judge(1);
                if(p&PSP_CTRL_UP)judge(2); if(p&PSP_CTRL_RIGHT)judge(3);
                Uint32 t=song_time();
                for(int i=0;i<note_count;i++){Note *n=&notes[i];if(!n->player&&!n->judged&&t>(Uint32)(n->time+HIT_MS)){n->judged=1;}}
                for(int i=0;i<note_count;i++){Note *n=&notes[i];if(n->player&&!n->judged&&t>(Uint32)(n->time+HIT_MS)){n->judged=1;misses++;combo=0;health-=4;}}
                if(health<=0||t>(Uint32)(notes[note_count-1].time+3000)){finished=1;Mix_HaltMusic();}
            }
        }
        SDL_FillRect(screen_surface,NULL,SDL_MapRGB(screen_surface->format,0,0,0));
        if(game_state==0)draw_menu();else{draw_game();if(paused){fill(120,95,240,70,0,0,0,210);text("PAUSADO",190,112,22,(SDL_Color){255,255,255,255});}if(finished)draw_end();}
        SDL_Flip(screen_surface);
        if(game_state==1&&finished&&(p&PSP_CTRL_CROSS)){start_song();}
        if(screen==1&&finished&&(p&PSP_CTRL_CIRCLE)){Mix_HaltMusic();game_state=0;}
        old=pad; SDL_Delay(16);
    }
    Mix_HaltMusic();if(music)Mix_FreeMusic(music);
    Mix_CloseAudio();Mix_Quit();IMG_Quit();SDL_Quit();
    sceKernelExitGame();return 0;
}
