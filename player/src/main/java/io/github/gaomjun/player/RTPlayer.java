package io.github.gaomjun.player;

import android.os.Handler;
import android.view.Surface;

/**
 * Created by qq on 1/11/2016.
 */

public class RTPlayer {
    private native boolean init(Surface surface, String url);

    private native void uninit();

    private native void updateDisplay();

    private Handler handler = new Handler();

    private Runnable runnable = new Runnable() {
        public void run () {
            updateDisplay();
            handler.postDelayed(this, 1);
        }
    };

    /**
     * must init in play first
     * @param surface
     * @param url
     * @return
     */
    public boolean initPlayer(Surface surface, String url) {
        return init(surface, url);
    }

    /**
     * if init successed, start to play
     */
    public void play() {
        handler.post(runnable);
    }

    /**
     * if playing, will pause
     */
    public void pause() {
        handler.removeCallbacks(runnable);
    }

    /**
     * if pause, will replay
     */
    public void resume() {
        play();
    }

    /**
     * stop play and uninit player
     */
    public void stop() {
        handler.removeCallbacks(runnable);
        uninit();
    }

    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avutil-55");
        System.loadLibrary("ffmpeg_codec");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");

        System.loadLibrary("rtplayer");
    }
}
