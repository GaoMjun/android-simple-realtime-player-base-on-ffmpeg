package io.github.gaomjun.videoplayer;

import android.app.Activity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;

import io.github.gaomjun.player.RTPlayer;

public class MainActivity extends Activity/* implements SurfaceHolder.Callback */{
    SurfaceHolder mSurfaceHolder;
    Button mPlayButton;
    Button mPauseButton;
    Button mStopButton;

    RTPlayer mRTPlayer = new RTPlayer();

    boolean initSuccess = false;

    PLAYSTATUS playstatus = PLAYSTATUS.STOPPED;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.sf_view);
        mSurfaceHolder = surfaceView.getHolder();

        mPlayButton = (Button) findViewById(R.id.playButton);
        mPauseButton = (Button) findViewById(R.id.pauseButton);
        mStopButton = (Button) findViewById(R.id.stopButton);

        mPlayButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (initSuccess) {
                    if (playstatus == PLAYSTATUS.STOPPED) {
                        mRTPlayer.play();
                        playstatus = PLAYSTATUS.PLAYING;
                    } else if (playstatus == PLAYSTATUS.PAUSE) {
                        mRTPlayer.resume();
                        playstatus = PLAYSTATUS.PAUSE;
                    }
                } else {
                    if (mRTPlayer.initPlayer(mSurfaceHolder.getSurface(), "rtsp://admin:admin@192.168.1.108")) {
                        initSuccess = true;
                        mRTPlayer.play();
                        playstatus = PLAYSTATUS.PLAYING;
                    }
                }
            }
        });

        mPauseButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (initSuccess) {
                    if (playstatus == PLAYSTATUS.PLAYING) {
                        mRTPlayer.pause();
                        playstatus = PLAYSTATUS.PAUSE;

                    } else if (playstatus == PLAYSTATUS.PAUSE) {
                        mRTPlayer.resume();
                        playstatus = PLAYSTATUS.PLAYING;
                    }
                }
            }
        });

        mStopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (initSuccess)
                    if (playstatus != PLAYSTATUS.STOPPED) {
                        mRTPlayer.stop();
                        playstatus = PLAYSTATUS.STOPPED;
                        initSuccess = false;
                    }
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();
        mRTPlayer.stop();
    }

    public enum PLAYSTATUS {
        PLAYING, PAUSE, STOPPED;
    }
}
