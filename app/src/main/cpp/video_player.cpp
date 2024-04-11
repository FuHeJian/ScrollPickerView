//
// Created by 111 on 2024/1/23.
//

#include "video_player.h"
#include "jni.h"
#include "android/native_window_jni.h"
#include "android/native_window.h"
#include "string"
#include <thread>
#include <math.h>
#include <android/log.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>

extern "C" {
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
}

struct ThreadContext {
    JavaVM *jvm = 0;
    jobject thiz = NULL;;
    jobject surface = NULL;
    std::string *jPath = 0;
    bool end = false;
};

struct VideoPlayInfo {

    AVFormatContext *avformatcontext = 0;

    SwsContext *scaleContext = 0;

    uint8_t *rgbaData = 0;
    AVPacket *decodePacket = 0;
    AVFrame *decodeFrame = 0;
    AVFrame *decodeRgbaFrame = 0;

    AVCodecContext *avCodecContext = 0;
    AVCodecContext *avCodecContextAudio = 0;

    ThreadContext *threadContext = 0;

};

void resetViewPlayInfo(VideoPlayInfo *videoInfo) {
    if (videoInfo == NULL)return;
    videoInfo->avformatcontext = 0;
    videoInfo->scaleContext = 0;
    videoInfo->rgbaData = 0;
    videoInfo->decodePacket = 0;
    videoInfo->decodeFrame = 0;
    videoInfo->decodeRgbaFrame = 0;
    videoInfo->avCodecContext = 0;
    videoInfo->threadContext = 0;
}

void clearViewPlayInfo(JNIEnv *env, VideoPlayInfo *videoInfo) {
    if (videoInfo == NULL)return;
    if (videoInfo->avformatcontext != NULL) {
        avformat_close_input(&videoInfo->avformatcontext);
    }
    if (videoInfo->scaleContext != NULL) {
        sws_freeContext(videoInfo->scaleContext);
    }
    if (videoInfo->rgbaData != 0) {
        free(videoInfo->rgbaData);
    }
    if (videoInfo->decodePacket != NULL) {
        av_packet_free(&videoInfo->decodePacket);
    }
    if (videoInfo->decodeFrame != NULL) {
        av_frame_free(&videoInfo->decodeFrame);
    }
    if (videoInfo->decodeRgbaFrame != NULL) {
        av_frame_free(&videoInfo->decodeRgbaFrame);
    }
    if (videoInfo->avCodecContext != NULL) {
        avcodec_free_context(&videoInfo->avCodecContext);
    }
    if (videoInfo->threadContext != 0) {
        if (videoInfo->threadContext->jPath != 0) {
            free(videoInfo->threadContext->jPath);
        }
        if (videoInfo->threadContext->surface != NULL) {
            env->DeleteGlobalRef(videoInfo->threadContext->surface);
        }
        if (videoInfo->threadContext->thiz != NULL) {
            env->DeleteGlobalRef(videoInfo->threadContext->thiz);
        }
        free(videoInfo->threadContext);
    }
    resetViewPlayInfo(videoInfo);
}

struct VideoControlFields {
    jfieldID seek = NULL;
    jfieldID playThread = NULL;
    jfieldID videoPlayInfo = NULL;
} videoControlFields;

void stopPlaying(JNIEnv *env, jobject thiz, jobject surface) {


}

void startPlay(JNIEnv *env, jobject thiz, jobject surface, char *path) {


}

pthread_mutex_t mutex;

void init(JNIEnv *env, jobject thiz) {

    jclass cla = env->FindClass("com/fhj/mvi_demo/VideoControler");
    videoControlFields.seek = env->GetFieldID(cla, "seek", "I");
    videoControlFields.playThread = env->GetFieldID(cla, "playThread", "J");
    videoControlFields.videoPlayInfo = env->GetFieldID(cla, "videoPlayInfo", "J");
    pthread_mutex_init(&mutex, NULL);

}

#define DIRECTION_PORTRAIT  (0)
#define DIRECTION_LANDSCAPE  (1)

struct Audio {
    void *data;
    SLuint32 size;
};

void AndroidSimpleBufferQueueCallback(
        SLAndroidSimpleBufferQueueItf caller,
        void *pContext
) {
    if (pContext) {
        Audio audio = *(Audio *) (pContext);
        (*caller)->Enqueue(caller, audio.data, audio.size);
    }
}

SLAndroidSimpleBufferQueueItf
playAudio(SLObjectItf *itf, AVChannelLayout channels, SLuint32 sampleRate, SLuint32 bitsPerSample) {
    SLObjectItf engine = NULL;
    SLObjectItf audioPlayerEngine = NULL;
    SLObjectItf environmentalReverbItf = NULL;
    SLEngineItf engineInterface = NULL;
    SLInterfaceID engIds[] = {SL_IID_ENGINE};
    SLboolean engReq[] = {SL_BOOLEAN_TRUE};
    auto ret = slCreateEngine(&engine, 0, 0, 1, engIds, engReq);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频SLObjectItf创建失败");
        return nullptr;
    }

    ret = (*engine)->Realize(engine, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频Realize创建失败");
        (*engine)->Destroy(engine);
        return nullptr;
    }

    ret = (*engine)->GetInterface(engine, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频SLEngineItf创建失败");
        (*engine)->Destroy(engine);
        return nullptr;
    }


    SLuint32 mask = 0;
    if (channels.u.mask & AV_CHAN_FRONT_LEFT) {
        mask |= SL_SPEAKER_FRONT_LEFT;
    }
    if (channels.u.mask & AV_CHAN_FRONT_RIGHT) {
        mask |= SL_SPEAKER_FRONT_RIGHT;
    }
    if (channels.u.mask & AV_CHAN_FRONT_CENTER) {
        mask |= SL_SPEAKER_FRONT_CENTER;
    }


    SLuint32 numSupportedInterfaces = 0;
    ret = (*engineInterface)->QueryNumSupportedInterfaces(engineInterface, SL_OBJECTID_OUTPUTMIX,
                                                          &numSupportedInterfaces);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频QueryNumSupportedInterfaces查询失败");
        (*engine)->Destroy(engine);
        return nullptr;
    }

    for (int i = 0; i < numSupportedInterfaces; i++) {
        SLInterfaceID interfaceID;
        ret = (*engineInterface)->QuerySupportedInterfaces(engineInterface, SL_OBJECTID_OUTPUTMIX,
                                                           i,
                                                           &interfaceID);
        if (SL_RESULT_SUCCESS != ret) {
            av_log(0, 1, "音频QuerySupportedInterfaces查询失败");
            (*engine)->Destroy(engine);
            return nullptr;
        }
    }


    SLInterfaceID outIds[] = {SL_IID_ENVIRONMENTALREVERB};
    SLboolean outB[] = {SL_BOOLEAN_FALSE};
    ret = (*engineInterface)->CreateOutputMix(engineInterface, &environmentalReverbItf,
                                              sizeof(outIds) / sizeof(*outIds), outIds,
                                              outB);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频CreateOutputMix创建失败");
        (*engine)->Destroy(engine);
        return nullptr;
    }
    ret = (*environmentalReverbItf)->Realize(environmentalReverbItf, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频environmentalReverbItf实现创建失败");
        (*engine)->Destroy(engine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    ret = (*environmentalReverbItf)->GetInterface(environmentalReverbItf,
                                                  SL_IID_ENVIRONMENTALREVERB,
                                                  &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频outputMixEnvironmentalReverb创建失败");
        (*engine)->Destroy(engine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }
    const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_UNDERWATER;
    ret = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
            outputMixEnvironmentalReverb, &reverbSettings);
/*    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频SetEnvironmentalReverbProperties创建失败");
        (*engine)->Destroy(engine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }*/

    SLDataLocator_OutputMix outAndroidLoctor = {SL_DATALOCATOR_OUTPUTMIX, environmentalReverbItf};
    SLDataSink dataSink = {
            &outAndroidLoctor, 0
    };

    SLDataFormat_PCM input_format = {
            SL_DATAFORMAT_PCM,                              // <<< 输入的音频格式,PCM
            (SLuint32) channels.nb_channels,                                              // <<< 输入的声道数，2(立体声)
            sampleRate * 1000,                           // <<< 输入的采样率，44100hz
            bitsPerSample,                    // <<< 输入的采样位数，16bit
            bitsPerSample,                    // <<< 容器大小，同上
            (SLuint32) channels.u.mask,// <<< 声道标记，这里使用左前声道和右前声道
            SL_BYTEORDER_LITTLEENDIAN                       // <<< 输入的字节序,小端
    };

    SLDataLocator_AndroidSimpleBufferQueue androidLoctor = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            255};
    SLDataSource dataSource = {
            &androidLoctor, &input_format
    };
/*    SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    static const SLboolean req2[] = {SL_BOOLEAN_FALSE};*/
    SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE, SL_IID_PLAY, SL_IID_VOLUME, SL_IID_MUTESOLO};
    static const SLboolean req2[] = {SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE, SL_BOOLEAN_FALSE,
                                     SL_BOOLEAN_FALSE};
    ret = (*engineInterface)->CreateAudioPlayer(engineInterface, &audioPlayerEngine, &dataSource,
                                                &dataSink,
                                                sizeof(ids) / sizeof(*ids),
                                                ids, req2);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频AudioPlayer创建失败");
        (*engine)->Destroy(engine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }

    ret = (*audioPlayerEngine)->Realize(audioPlayerEngine, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频AudioPlayer Realize创建失败");
        (*engine)->Destroy(engine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }

    SLPlayItf playItf = NULL;
    ret = (*audioPlayerEngine)->GetInterface(audioPlayerEngine, SL_IID_PLAY, &playItf);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频Interface创建失败");
        (*engine)->Destroy(engine);
        (*audioPlayerEngine)->Destroy(audioPlayerEngine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }

    //5.1  获取音频播放的buffer接口 SLAndroidSimpleBufferQueueItf
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;
    ret = (*audioPlayerEngine)->GetInterface(audioPlayerEngine, SL_IID_BUFFERQUEUE,
                                             &pcmBufferQueue);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频SLAndroidSimpleBufferQueueItf创建失败");

        (*engine)->Destroy(engine);

        (*audioPlayerEngine)->Destroy(audioPlayerEngine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }
    SLVolumeItf volume = NULL;
    ret = (*audioPlayerEngine)->GetInterface(audioPlayerEngine, SL_IID_VOLUME,
                                             &volume);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频SLAndroidSimpleBufferQueueItf创建失败");
        (*engine)->Destroy(engine);
        (*audioPlayerEngine)->Destroy(audioPlayerEngine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }
    (*volume)->SetVolumeLevel(volume, 70 * -20);
    (*volume)->SetMute(volume, SL_BOOLEAN_FALSE);
    SLMuteSoloItf pcmChannelModePlay;
    ret = (*audioPlayerEngine)->GetInterface(audioPlayerEngine, SL_IID_MUTESOLO,
                                             &pcmChannelModePlay);
    if (SL_RESULT_SUCCESS != ret) {
        av_log(0, 1, "音频SL_IID_MUTESOLO创建失败");
        (*engine)->Destroy(engine);
        (*audioPlayerEngine)->Destroy(audioPlayerEngine);
        (*environmentalReverbItf)->Destroy(environmentalReverbItf);
        return nullptr;
    }
    (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 1, SL_BOOLEAN_FALSE);
    (*pcmChannelModePlay)->SetChannelMute(pcmChannelModePlay, 0, SL_BOOLEAN_FALSE);
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    *itf = engine;
    return pcmBufferQueue;
}


void *loadVideoThread(void *threadContext);

/**
 *
 * @param env
 * @param thiz
 * @param surface
 * @param path
 * @return 0 == SUCCESS
 */
jint loadVideo(JNIEnv *env, jobject thiz, jobject surface, jstring jPath) {

    pthread_t thread;
    ThreadContext *para = new ThreadContext();
    env->GetJavaVM(&para->jvm);
    para->thiz = env->NewGlobalRef(thiz);
    para->surface = env->NewGlobalRef(surface);
    const char *path = env->GetStringUTFChars(jPath, nullptr);
    para->jPath = new std::string(path);
    env->ReleaseStringUTFChars(jPath, path);
    para->end = false;

    jlong videoInfoPtr = env->GetLongField(thiz, videoControlFields.videoPlayInfo);
    VideoPlayInfo *videoInfo = 0;
    if (videoInfoPtr != 0) {
        videoInfo = (VideoPlayInfo *) videoInfoPtr;
        if (videoInfo->threadContext != 0) {
            videoInfo->threadContext->end = true;
        }
    }
    pthread_create(&thread, NULL, loadVideoThread, (void *) para);
    pthread_detach(thread);
/*      void* re = 0;
    pthread_join(thread,&re);
    return (uintptr_t)re;*/
    return 0;
}

void *loadVideoThread(void *tc) {

    JavaVM *_jvm;
    JNIEnv *_env;

    pthread_mutex_lock(&mutex);
    ThreadContext *threadContext = reinterpret_cast<ThreadContext *>(tc);
    JavaVM *jvm = threadContext->jvm;
    JNIEnv *env = 0;
    jvm->AttachCurrentThread(&env, 0);
    if (env == NULL) {
        pthread_exit((void *) -1);
    }
    jobject thiz = threadContext->thiz;
    jobject surface = threadContext->surface;
    const char *jPath = threadContext->jPath->c_str();

    jboolean isCopy;
    jlong videoInfoPtr = env->GetLongField(thiz, videoControlFields.videoPlayInfo);
    VideoPlayInfo *videoInfo = 0;
    if (videoInfoPtr != 0) {
        videoInfo = (VideoPlayInfo *) videoInfoPtr;
        videoInfo->threadContext = threadContext;
    } else {
        videoInfo = new VideoPlayInfo();
        videoInfo->threadContext = threadContext;
        env->SetLongField(thiz, videoControlFields.videoPlayInfo, (jlong) videoInfo);
    }
    const char *path = jPath;
//  char *path = "/storage/emulated/0/DCIM/Camera/VID_20240107_110844.mp4";
    AVFormatContext *avformatcontext = 0;
    jint re = avformat_open_input(&avformatcontext, path, NULL, NULL);
    jint re_audio = 0;
    __android_log_print(ANDROID_LOG_INFO, "日志", "avformatcontext地址%lx", avformatcontext);
    if (re >= 0) {//初始化成功
        re = avformat_find_stream_info(avformatcontext, NULL);
        if (re < 0) {
            avformat_close_input(&avformatcontext);
            clearViewPlayInfo(env, videoInfo);
            videoInfo->threadContext = NULL;
            jvm->DetachCurrentThread();
            pthread_mutex_unlock(&mutex);
            pthread_exit((void *) -1);
            return NULL;
        }
        videoInfo->avformatcontext = avformatcontext;
        int seekStreamIndex = -1;
        int audioStreamIndex = -1;

        for (int i = 0; i < avformatcontext->nb_streams; i++) {

            if (avformatcontext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                seekStreamIndex = i;
            } else if (avformatcontext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioStreamIndex = i;
            }

        }

        if (seekStreamIndex >= 0 && audioStreamIndex >= 0) {

            const AVCodec *avCodecVideo = avcodec_find_decoder(
                    avformatcontext->streams[seekStreamIndex]->codecpar->codec_id);

            const AVCodec *avCodecAudio = avcodec_find_decoder(
                    avformatcontext->streams[audioStreamIndex]->codecpar->codec_id);

            if (avCodecVideo == nullptr || avCodecAudio == nullptr) {
                avformat_close_input(&avformatcontext);
                resetViewPlayInfo(videoInfo);
                videoInfo->threadContext = threadContext;
                clearViewPlayInfo(env, videoInfo);
                videoInfo->threadContext = NULL;
                jvm->DetachCurrentThread();
                pthread_mutex_unlock(&mutex);
                pthread_exit((void *) -1);
                return NULL;//没有成功加载AVCodec
            }

            AVCodecContext *av_codec_video = avcodec_alloc_context3(avCodecVideo);
            AVCodecContext *av_codec_audio = avcodec_alloc_context3(avCodecAudio);
            videoInfo->avCodecContext = av_codec_video;
            videoInfo->avCodecContextAudio = av_codec_audio;
            re = avcodec_parameters_to_context(av_codec_video,
                                               avformatcontext->streams[seekStreamIndex]->codecpar);
            re_audio = avcodec_parameters_to_context(av_codec_audio,
                                                     avformatcontext->streams[audioStreamIndex]->codecpar);
            av_codec_video->thread_count = 30;
            av_codec_video->thread_type = FF_THREAD_FRAME;
            av_codec_audio->thread_count = 30;
            av_codec_audio->thread_type = FF_THREAD_FRAME;
            if (re < 0 || re_audio < 0) {
                avformat_close_input(&avformatcontext);
                avcodec_free_context(&av_codec_video);
                avcodec_free_context(&av_codec_audio);
                resetViewPlayInfo(videoInfo);
                videoInfo->threadContext = threadContext;
                clearViewPlayInfo(env, videoInfo);
                videoInfo->threadContext = NULL;
                jvm->DetachCurrentThread();
                pthread_mutex_unlock(&mutex);
                pthread_exit((void *) -1);
                return NULL;
            }
            re = avcodec_open2(av_codec_video, avCodecVideo, NULL);
            re_audio = avcodec_open2(av_codec_audio, avCodecAudio, NULL);
            if (re < 0 || re_audio < 0) {
                avformat_close_input(&avformatcontext);
                avcodec_free_context(&av_codec_video);
                avcodec_free_context(&av_codec_audio);
                resetViewPlayInfo(videoInfo);
                videoInfo->threadContext = threadContext;
                clearViewPlayInfo(env, videoInfo);
                videoInfo->threadContext = NULL;
                jvm->DetachCurrentThread();
                pthread_mutex_unlock(&mutex);
                pthread_exit((void *) -1);
                return NULL;
            }

            ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
            ANativeWindow_Buffer buffer;

            int surfaceHeight = ANativeWindow_getHeight(window);
            int surfaceWidth = ANativeWindow_getWidth(window);
            ANativeWindow_setBuffersGeometry(window, surfaceWidth, surfaceHeight,
                                             WINDOW_FORMAT_RGBA_8888);
            AVPacket *readPacket = av_packet_alloc();
            AVFrame *decodeFrame = av_frame_alloc();
            AVFrame *decodeRGBAFrame = av_frame_alloc();

            videoInfo->decodePacket = readPacket;
            videoInfo->decodeFrame = decodeFrame;
            videoInfo->decodeRgbaFrame = decodeRGBAFrame;

            float scale = std::min<float>(
                    surfaceWidth / (av_codec_video->width * 1.0f),
                    surfaceHeight / (av_codec_video->height * 1.0f));

            int oldWidth = av_codec_video->width;
            int oldHeight = av_codec_video->height;

            int dstWidth = scale * av_codec_video->width;
            int dstHeight = scale * av_codec_video->height;

            SwsContext *scaleContext = sws_getContext(
                    av_codec_video->width, av_codec_video->height,
                    (AVPixelFormat) av_codec_video->pix_fmt, dstWidth,
                    dstHeight, AV_PIX_FMT_RGBA, 0,
                    nullptr, nullptr, nullptr);

            videoInfo->scaleContext = scaleContext;

            int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                       dstWidth,
                                                       dstHeight,
                                                       1);

            __android_log_print(ANDROID_LOG_INFO, "日志", "原始的宽%d,高%d", oldWidth, oldHeight);
            __android_log_print(ANDROID_LOG_INFO, "日志", "转换的宽%d,高%d", dstWidth, dstHeight);

            __android_log_print(ANDROID_LOG_INFO, "日志", "解析出来的格式%s",
                                av_get_pix_fmt_name((AVPixelFormat) av_codec_video->pix_fmt));

            uint8_t *ragb_data = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));

            videoInfo->rgbaData = ragb_data;

            av_image_fill_arrays(decodeRGBAFrame->data, decodeRGBAFrame->linesize, ragb_data,
                                 AV_PIX_FMT_RGBA, dstWidth, dstHeight, 1);

            int direction = DIRECTION_PORTRAIT;
            long remainTime = 0;
            int64_t time = 0;//秒
            double frameTime = 0;
            long skipFrame = 0;
            int64_t startTime = 0;
            int64_t playTime = 0;
            int64_t currentTime = 0;

            int everyFrameTime =
                    1 / av_q2d(avformatcontext->streams[seekStreamIndex]->r_frame_rate) * 1000000;
            SLObjectItf itf = NULL;
            SL_SAMPLINGRATE_48;
            SLAndroidSimpleBufferQueueItf audioQueue = playAudio(&itf,
                                                                 av_codec_audio->ch_layout,
                                                                 av_codec_audio->sample_rate,
                                                                 av_codec_audio->bits_per_coded_sample);
            int bits = av_codec_audio->bits_per_coded_sample;
            AVSampleFormat outSampleormat;
            if (bits == 16) {
                outSampleormat = AV_SAMPLE_FMT_S16;
            } else {
                outSampleormat = AV_SAMPLE_FMT_S32;
            }
            SwrContext *swrContext = swr_alloc();
            AVChannelLayout *out_channel_layout;
            av_channel_layout_default(out_channel_layout, av_codec_audio->ch_layout.nb_channels);
            swr_alloc_set_opts2(&swrContext, out_channel_layout, outSampleormat,
                                av_codec_audio->sample_rate,
                                &av_codec_audio->ch_layout, av_codec_audio->sample_fmt,
                                av_codec_audio->sample_rate,
                                0, 0);
            swr_init(swrContext);
            //2. 申请输出 Buffer
            auto m_nbSamples = av_codec_audio->sample_rate / av_codec_audio->ch_layout.nb_channels;
//            m_nbSamples = (int)av_rescale_rnd(av_codec_audio->sample_rate, 1, av_codec_audio->ch_layout.nb_channels, AV_ROUND_UP);
            int64_t audioDelay = swr_get_delay(swrContext, 1000);
//            int dst_sample = av_codec_audio->frame_size + audioDelay / av_codec_audio->ch_layout.nb_channels;
//            int dst_sample = av_codec_audio->sample_rate / av_codec_audio->ch_layout.nb_channels;
            m_nbSamples = avformatcontext->streams[audioStreamIndex]->codecpar->frame_size;
            auto m_BufferSize = av_samples_get_buffer_size(NULL,
                                                           av_codec_audio->ch_layout.nb_channels,
                                                           m_nbSamples,
                                                           outSampleormat, 1);


            auto m_AudioOutBuffer = (uint8_t *) malloc(m_BufferSize);

/*          re = av_seek_frame(avformatcontext, seekStreamIndex,
                               10000000, AVSEEK_FLAG_FRAME);
            if (re < 0) {
                return re;
            }*/
            bool rere = true;
            
            while (!av_read_frame(avformatcontext, readPacket) && !videoInfo->threadContext->end) {

                __android_log_print(ANDROID_LOG_INFO, "日志", "当前解析类型%d",
                                    (int) avformatcontext->streams[readPacket->stream_index]->codecpar->codec_type);
                if (remainTime > 0) {
                    av_usleep(remainTime);
                }
                time = av_gettime_relative();//av_gettime_relative获取的是开机后的微妙时间
                if (readPacket->stream_index != seekStreamIndex) {
                    //解析音频
                    if (audioQueue) {
                        re = avcodec_send_packet(av_codec_audio, readPacket);
                        if (!re) {
                            for (;;) {
                                re = avcodec_receive_frame(av_codec_audio, decodeFrame);
                                if (!re) {
                                    swr_convert(swrContext, &m_AudioOutBuffer,
                                                decodeFrame->nb_samples,
                                                (const uint8_t **) decodeFrame->data,
                                                decodeFrame->nb_samples);
                                    (*audioQueue)->Enqueue(audioQueue, m_AudioOutBuffer,
                                                           m_BufferSize);
                                  /*  if (rere)

                                    else {
                                        (*audioQueue)->Clear(audioQueue);
                                        rere = false;
                                    }*/
                                } else {
                                    break;
                                }
                                av_frame_unref(decodeFrame);
                            }
                        }
                        av_packet_unref(readPacket);
                    }
                    continue;
                }
/*                if (skipFrame > 0) {
                    skipFrame--;
                    continue;
                }*/
                readPacket->time_base = avformatcontext->streams[seekStreamIndex]->time_base;

                re = ANativeWindow_lock(window, &buffer, NULL);
                if (re != 0) {
                    continue;
                }

                re = avcodec_send_packet(av_codec_video, readPacket);

                if (!re) {
                    for (;;) {
                        re = avcodec_receive_frame(av_codec_video, decodeFrame);
                        if (!re) {

                            if (av_codec_video->width != oldWidth ||
                                av_codec_video->height !=
                                oldHeight) {//解析出来的帧大小，可能会和通过头文件解析出来的宽高有区别，这里需要更新一下

                                oldWidth = av_codec_video->width;
                                oldHeight = av_codec_video->height;
                                scale = std::min<float>(
                                        surfaceWidth / (av_codec_video->width * 1.0f),
                                        surfaceHeight / (av_codec_video->height * 1.0f));

                                dstWidth = scale * av_codec_video->width;
                                dstHeight = scale * av_codec_video->height;
                                sws_freeContext(scaleContext);

                                free(ragb_data);

                                buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                                       dstWidth,
                                                                       dstHeight,
                                                                       1);

                                ragb_data = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));

                                av_image_fill_arrays(decodeRGBAFrame->data,
                                                     decodeRGBAFrame->linesize,
                                                     ragb_data,
                                                     AV_PIX_FMT_RGBA, dstWidth, dstHeight, 1);

                                scaleContext = sws_getContext(
                                        av_codec_video->width, av_codec_video->height,
                                        (AVPixelFormat) av_codec_video->pix_fmt, dstWidth,
                                        dstHeight, AV_PIX_FMT_RGBA, 0,
                                        nullptr, nullptr, nullptr);

                                videoInfo->scaleContext = scaleContext;

                            }

                            if (direction == DIRECTION_PORTRAIT) {

                                decodeRGBAFrame->width = dstWidth;
                                decodeRGBAFrame->height = dstHeight;
                                sws_scale(scaleContext, decodeFrame->data,
                                          decodeFrame->linesize, 0, av_codec_video->height,
                                          decodeRGBAFrame->data, decodeRGBAFrame->linesize);


                                uint8_t *surfaceBufferData = (uint8_t *) buffer.bits;
                                int j = std::max(buffer.height / 2 - dstHeight / 2, 0);
                                if (j > 0) {
                                    memset(surfaceBufferData, 0, buffer.stride * 4 * buffer.height);
                                }
                                int y = 0;

                                for (int i = j; i < (dstHeight + j) && y < dstHeight; i++, y++) {
                                    std::memcpy(surfaceBufferData + i * buffer.stride * 4,
                                                decodeRGBAFrame->data[0] +
                                                //这里要写decodeRGBAFrame->data[0] 不要写decodeRGBAFrame->data 因为decodeRGBAFrame->data是一个指针数据 每一个数组的值才是表示指向平面的指针 对于RGBA只有一个平面，所以使用decodeRGBAFrame->data[0]
                                                y * decodeRGBAFrame->linesize[0],
                                                decodeRGBAFrame->linesize[0]);
                                }

                            } else {
                                if (decodeFrame->height < decodeFrame->width) {//横屏播放

                                } else {
                                    if (decodeFrame->height > surfaceHeight ||
                                        decodeFrame->width > surfaceWidth) {//约束视频宽高，以显示完全

                                        sws_scale(scaleContext, decodeFrame->data,
                                                  decodeFrame->linesize, 0, av_codec_video->height,
                                                  decodeRGBAFrame->data, decodeRGBAFrame->linesize);

                                    }
                                }
                            }
                        } else {
                            break;
                        }
                        av_frame_unref(decodeFrame);
                    }
                    av_packet_unref(readPacket);
                }

                ANativeWindow_unlockAndPost(window);
                currentTime = av_gettime_relative();
                if (startTime == 0) {
                    startTime = currentTime;
                } else {
                    playTime = currentTime - startTime;
                }
                time = currentTime - time;
                frameTime = av_q2d(readPacket->time_base) * readPacket->pts * 1000000;
/*                if(frameTime > ){
                    skipFrame++;
                }*/
                if (playTime > frameTime + everyFrameTime) {//解析的太慢，直接跳过去
                    skipFrame = (playTime - frameTime + everyFrameTime) / everyFrameTime;
                }
                remainTime = everyFrameTime - time;
            }

            ANativeWindow_release(window);
/*          if (audioQueue) {
                (*itf)->Destroy(itf);
            }*/
            swr_free(&swrContext);

            avformat_close_input(&avformatcontext);
            sws_freeContext(scaleContext);
            avcodec_free_context(&av_codec_video);
            avcodec_free_context(&av_codec_audio);
            av_packet_free(&readPacket);
            av_frame_free(&decodeFrame);
            av_frame_free(&decodeRGBAFrame);
            free(ragb_data);
            free(m_AudioOutBuffer);
            resetViewPlayInfo(videoInfo);

        } else {
            avformat_close_input(&avformatcontext);
            resetViewPlayInfo(videoInfo);
        }

    } else {

        clearViewPlayInfo(env, videoInfo);
        videoInfo->threadContext = NULL;
        jvm->DetachCurrentThread();
        pthread_mutex_unlock(&mutex);
        pthread_exit((void *) re);
        return 0;

    }

    videoInfo->threadContext = threadContext;
    clearViewPlayInfo(env, videoInfo);
    videoInfo->threadContext = NULL;
    jvm->DetachCurrentThread();
    pthread_mutex_unlock(&mutex);
    pthread_exit((void *) re);
    return 0;

}

jstring parseErrorCode(JNIEnv *env, jobject thiz, jint errorCode) {
    char *re = av_err2str(errorCode);
    jstring javaString = env->NewStringUTF(re);
    return javaString;
}

