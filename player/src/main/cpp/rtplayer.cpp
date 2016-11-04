//
// Created by qq on 1/11/2016.
//

#include <jni.h>
#include <string>
#include <Android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#define LOG_TAG "JNI_LOG"
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, __VA_ARGS__)

static bool initPlayerWithUrl(const char url[]);
static bool setUpVideo(void);
static void initView(void);
static bool stepFrame(void);
static void display(void);

static AVFormatContext *iFormatContext = NULL;
static AVCodecContext *iVideoCodecContext = NULL;
static AVCodec *iVideoCodec = NULL;
static AVFrame *iVideoFrame = NULL;
static AVFrame *iVideoFrameRGBA = NULL;
static AVPacket avPacket;
static struct SwsContext *sws_ctx = NULL;

static int videoStream = -1;

static int videoWidth = 0;
static int videoHeight = 0;

static ANativeWindow *nativeWindow = NULL;
static ANativeWindow_Buffer windowBuffer;

int frameFinished = 0;

void Java_io_github_gaomjun_player_RTPlayer_uninit(JNIEnv *) {
    avformat_close_input(&iFormatContext);

    if (iFormatContext) avformat_close_input(&iFormatContext);
    if (iVideoCodecContext) avcodec_close(iVideoCodecContext);
    if (iVideoFrame) av_frame_free(&iVideoFrame);
    if (iVideoFrameRGBA) av_frame_free(&iVideoFrameRGBA);
    if (sws_ctx) sws_freeContext(sws_ctx);
    if (nativeWindow) ANativeWindow_release(nativeWindow);

    iFormatContext = NULL;
    iVideoCodecContext = NULL;
    iVideoCodec = NULL;
    iVideoFrame = NULL;
    iVideoFrameRGBA = NULL;
    sws_ctx = NULL;
    nativeWindow = NULL;

    videoStream = -1;
    videoWidth = 0;
    videoHeight = 0;
    frameFinished = 0;
}

void Java_io_github_gaomjun_player_RTPlayer_updateDisplay(JNIEnv *) {
    display();
}

jboolean Java_io_github_gaomjun_player_RTPlayer_init(JNIEnv *env,
                                                              jclass,
                                                              jobject surface,
                                                              jstring urlString) {

    nativeWindow = ANativeWindow_fromSurface(env, surface);

    const char *cUrl = env->GetStringUTFChars(urlString, JNI_FALSE);

    if (!initPlayerWithUrl(cUrl)) return JNI_FALSE;

    if (!setUpVideo()) return JNI_FALSE;

    initView();

    env->ReleaseStringUTFChars(urlString, cUrl);

    return JNI_TRUE;
}

static bool initPlayerWithUrl(const char url[]) {
    avcodec_register_all();
    av_register_all();
    avformat_network_init();

    AVDictionary *opts = 0;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "probesize", "2048", 0);
    av_dict_set(&opts, "fflags", "nobuffer", 0);
//    av_dict_set(&opts, "analyzeduration", "2000000", 0);
    av_dict_set(&opts, "max_delay", "250000", 0);

    if (avformat_open_input(&iFormatContext, url, NULL, &opts) != 0) {
        ALOGD("avformat_open_input() failed \n");

        return false;
    } else {

        ALOGD("avformat_open_input() %s \n", url);
    }

    if (avformat_find_stream_info(iFormatContext, NULL) < 0) {
        ALOGD("avformat_find_stream_info() failed \n");

        return false;
    } else {
        ALOGD("avformat_find_stream_info() \n");
    }

    return true;
}

static bool setUpVideo(void) {
    for (int i = 0; i < iFormatContext->nb_streams; i++) {
        if (iFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {

            ALOGD("found video stream %d \n", i);
            videoStream = i;

            break;
        }
    }

    if (videoStream < 0) {
        ALOGD("no video stream \n");

        return false;
    }

    iVideoCodecContext = iFormatContext->streams[videoStream]->codec;
    iVideoCodec = avcodec_find_decoder(iVideoCodecContext->codec_id);
    if (iVideoCodec == NULL) {
        ALOGD("unsupported video codec \n");

        return false;
    } else {
        ALOGD("avcodec_find_decoder %s\n", iVideoCodec->name);
    }

    if (avcodec_open2(iVideoCodecContext, iVideoCodec, NULL) != 0) {
        ALOGD("Cannot open video decoder \n");

        return false;
    } else {
        ALOGD("avcodec_open2 %s\n", iVideoCodec->name);
    }

    videoWidth = iVideoCodecContext->width;
    videoHeight = iVideoCodecContext->height;

    iVideoFrame = av_frame_alloc();
    iVideoFrameRGBA = av_frame_alloc();

    return true;
}

static void initView(void) {

    ANativeWindow_setBuffersGeometry(nativeWindow,
                                     videoWidth,
                                     videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                            iVideoCodecContext->width,
                                            iVideoCodecContext->height,
                                            1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(iVideoFrameRGBA->data,
                         iVideoFrameRGBA->linesize,
                         buffer,
                         AV_PIX_FMT_RGBA,
                         iVideoCodecContext->width,
                         iVideoCodecContext->height,
                         1);

    sws_ctx = sws_getContext(iVideoCodecContext->width,
                             iVideoCodecContext->height,
                             iVideoCodecContext->pix_fmt,
                             iVideoCodecContext->width,
                             iVideoCodecContext->height,
                             AV_PIX_FMT_RGBA,
                             SWS_BILINEAR,
                             NULL, NULL, NULL);
}

static bool stepFrame(void) {
    frameFinished = 0;

    while (!frameFinished) {

        if (av_read_frame(iFormatContext, &avPacket) != 0) {
            continue;
        }

        if (avPacket.stream_index == videoStream) {
            avcodec_decode_video2(iVideoCodecContext, iVideoFrame, &frameFinished, &avPacket);
            ALOGD("pict_type: %d", iVideoFrame->pict_type);
        }
        av_packet_unref(&avPacket);
    }

    return frameFinished != 0;
}

static void display(void) {
    if (stepFrame()) {
        if (iVideoFrame->pict_type != AV_PICTURE_TYPE_NONE) {

            ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

            sws_scale(sws_ctx,
                      (uint8_t const *const *) iVideoFrame->data,
                      iVideoFrame->linesize,
                      0,
                      iVideoCodecContext->height,
                      iVideoFrameRGBA->data,
                      iVideoFrameRGBA->linesize);

            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            int dstStride = windowBuffer.stride * 4;
            uint8_t *src = (iVideoFrameRGBA->data[0]);
            int srcStride = iVideoFrameRGBA->linesize[0];

            for (int h = 0; h < videoHeight; h++) {
                memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
            }

            ANativeWindow_unlockAndPost(nativeWindow);
        }
    }
}
#ifdef __cplusplus
}
#endif