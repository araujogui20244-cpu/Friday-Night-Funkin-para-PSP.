package com.gudstudios.fnf;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Color;
import android.graphics.Typeface;
import android.view.MotionEvent;
import android.view.View;
import android.media.AudioManager;
import android.media.ToneGenerator;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

public class MainActivity extends Activity {
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        );
        setContentView(new GameView());
    }

    @Override public void onBackPressed() {
        setContentView(new GameView());
    }

    private class GameView extends View {
        private final Paint p = new Paint(3);
        private final ArrayList<Note> notes = new ArrayList<>();
        private final Random random = new Random(7);
        private final ToneGenerator tone = new ToneGenerator(AudioManager.STREAM_MUSIC, 45);
        private final int[] colors = { Color.rgb(65,190,255), Color.rgb(80,235,130), Color.rgb(255,90,110), Color.rgb(190,110,255) };
        private boolean playing = false;
        private long lastFrame = System.currentTimeMillis();
        private long lastSpawn = 0;
        private long startTime = 0;
        private int score = 0, combo = 0, misses = 0;
        private int selectedSong = 0;
        private final String[] songs = {"Parish - Training", "Worship - Training", "Zavodila - Training", "Gospel - Training"};

        GameView() { super(MainActivity.this); setFocusable(true); }

        private void text(Canvas c, String s, float x, float y, float size, int color, boolean center) {
            p.setColor(color); p.setTextSize(size); p.setTypeface(Typeface.create("sans-serif-black", Typeface.BOLD));
            p.setTextAlign(center ? Paint.Align.CENTER : Paint.Align.LEFT); c.drawText(s, x, y, p);
        }
        private void rect(Canvas c, float l,float t,float r,float b,int color,float radius) {
            p.setColor(color); c.drawRoundRect(l,t,r,b,radius,radius,p);
        }

        @Override protected void onDraw(Canvas raw) {
            super.onDraw(raw);
            Canvas c = raw;
            float sx = getWidth()/800f, sy = getHeight()/450f;
            c.save(); c.scale(sx,sy);
            c.drawColor(Color.rgb(13,13,27));
            // stage background
            p.setShader(new android.graphics.LinearGradient(0,0,800,450,Color.rgb(30,20,55),Color.rgb(7,9,20),android.graphics.Shader.TileMode.CLAMP));
            c.drawRect(0,0,800,450,p); p.setShader(null);
            for(int i=0;i<18;i++) {
                p.setColor(Color.argb(35,150,120,255));
                c.drawCircle((i*83)%800,(i*47)%450,2+(i%3),p);
            }
            if(!playing) {
                text(c,"GUD FNF",400,105,58,Color.WHITE,true);
                text(c,"FAN GAME • ANDROID",400,143,18,Color.rgb(170,160,210),true);
                text(c,"SELECT SONG",400,190,19,Color.rgb(110,220,255),true);
                for(int i=0;i<songs.length;i++) {
                    int y=215+i*43;
                    rect(c,210,y-27,590,y+8,i==selectedSong?Color.rgb(92,45,135):Color.rgb(35,34,58),12);
                    text(c,songs[i],400,y,20,Color.WHITE,true);
                }
                rect(c,310,405,490,445,Color.rgb(0,190,130),12);
                text(c,"PLAY",400,433,22,Color.WHITE,true);
                text(c,"Primeira versão de treino • toque para selecionar",400,386,13,Color.LTGRAY,true);
            } else {
                text(c,songs[selectedSong],400,32,20,Color.WHITE,true);
                text(c,"SCORE  "+score+"     COMBO  "+combo+"     MISS  "+misses,400,62,17,Color.rgb(255,225,120),true);
                float left=190, laneW=105, receptorY=365;
                for(int i=0;i<4;i++) {
                    float x=left+i*laneW;
                    rect(c,x,80,x+91,430,Color.argb(95,30,35,60),8);
                    rect(c,x+12,receptorY-10,x+79,receptorY+10,colors[i],8);
                    text(c,new String[]{"←","↓","↑","→"}[i],x+45,receptorY+7,23,Color.WHITE,true);
                }
                long now=System.currentTimeMillis();
                if(now-lastSpawn>720) {
                    notes.add(new Note(random.nextInt(4),95,now));
                    lastSpawn=now;
                }
                Iterator<Note> it=notes.iterator();
                while(it.hasNext()) {
                    Note n=it.next();
                    n.y += (now-lastFrame)*0.22f;
                    float x=left+n.lane*laneW+45;
                    rect(c,x-28,n.y-17,x+28,n.y+17,colors[n.lane],12);
                    text(c,new String[]{"←","↓","↑","→"}[n.lane],x,n.y+8,24,Color.WHITE,true);
                    if(n.y>440) { it.remove(); misses++; combo=0; }
                }
                lastFrame=now;
                text(c,"TOQUE NA PISTA QUANDO A NOTA CHEGAR",400,438,13,Color.LTGRAY,true);
                postInvalidateDelayed(16);
            }
            c.restore();
        }

        @Override public boolean onTouchEvent(MotionEvent e) {
            if(e.getAction()!=MotionEvent.ACTION_DOWN) return true;
            float x=e.getX()*800f/getWidth(), y=e.getY()*450f/getHeight();
            if(!playing) {
                if(y>188 && y<385) {
                    int pick=(int)((y-188)/43f);
                    if(pick>=0 && pick<4) selectedSong=pick;
                }
                if(y>390) { playing=true; score=combo=misses=0; notes.clear(); lastSpawn=System.currentTimeMillis(); lastFrame=lastSpawn; }
                invalidate(); return true;
            }
            int lane=(int)((x-190)/105f);
            if(lane>=0 && lane<4) {
                Note best=null; float distance=9999;
                for(Note n:notes) if(n.lane==lane && Math.abs(n.y-365)<distance) { best=n; distance=Math.abs(n.y-365); }
                if(best!=null && distance<95) {
                    notes.remove(best); score+= distance<35?350:200; combo++;
                    tone.startTone(ToneGenerator.TONE_PROP_BEEP2,70);
                } else { misses++; combo=0; tone.startTone(ToneGenerator.TONE_PROP_NACK,70); }
            } else if(y<55) { playing=false; }
            invalidate(); return true;
        }
        @Override protected void onDetachedFromWindow() {
            tone.release(); super.onDetachedFromWindow();
        }
    }

    private static class Note {
        int lane; float y;
        Note(int lane,float y,long time){this.lane=lane;this.y=y;}
    }
}
