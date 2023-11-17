#include "iostream"
#include "functions.h"
#include "android/log.h"
#include <unistd.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <any>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <thread>


extern "C" {
#include "libavutil/avstring.h"
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
}

using namespace std;

extern "C" const AVInputFormat ff_android_camera_demuxer;

const JNINativeMethod nativeMethods[] = {
        {"op", "(Ljava/lang/String;Landroid/view/Surface;)I", (void *) native_op}
};

jint native_op(JNIEnv *env, jobject thiz, jstring p, jobject surface) {
    auto path = (char *) env->GetStringUTFChars(p, NULL);
    // 获取 ANativeWindow
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    return openVideo(path, nativeWindow);
}

void log_callback(void *obj, int status, const char *info, va_list val) {
    __android_log_vprint(ANDROID_LOG_INFO, "ffmpeg日志",
                         info, val);
}


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        if (registerNativeMethods(env) == JNI_OK) {
            result = JNI_VERSION_1_6;
        }
    }

    av_log_set_callback(log_callback);

    return result;
}


static int registerNativeMethods(JNIEnv *env) {
    int result = -1;
    jclass class_hello = env->FindClass("com/fhj/mvi_demo/BaseActivity");
    if (env->RegisterNatives(class_hello, nativeMethods,
                             sizeof(nativeMethods) / sizeof(nativeMethods[0])) == JNI_OK) {
        result = 0;
    }
    return result;
}


AVPixelFormat getFormat(struct AVCodecContext *s, const enum AVPixelFormat *fmt) {
    __android_log_write(ANDROID_LOG_INFO, "日志调用getFormat",
                        "日志调用getFormat");

    return fmt[0];
}

int getVALUE() {
    return 1;
}

int getVALUE2();

int getVALUE2() {
    return 1;
}

jlong openVideo(char *path, ANativeWindow *nativeWindow) {
    AVFormatContext *av_context = avformat_alloc_context();
    AVFormatContext *av_context_File = avformat_alloc_context();
//   path = "/storage/emulated/0/DCIM/Camera/VID_20230930_164921.mp4";

    void *i = 0;
    //初始化av_context信息，包括流的数量，媒体格式
    void *op = 0;
    const AVInputFormat *androidFormat = &ff_android_camera_demuxer;
    auto success = avformat_open_input(&av_context, NULL, androidFormat, NULL);
    success |= avformat_open_input(&av_context_File, path, NULL, NULL);

    if (success == 0) {

        //初始化各个流的参数
        int streamIndex = avformat_find_stream_info(av_context, NULL);
        int streamIndex_File = avformat_find_stream_info(av_context_File, NULL);

        success = 1;

        __android_log_write(ANDROID_LOG_INFO, "日志iformat变化前", av_context->iformat->name);

        auto findRe = streamIndex | streamIndex_File;

        if (findRe < 0) {
            return 10;
        }

        int audioIndex = -1;

        int sCount = av_context->nb_streams;

        for (int i = 0; i < sCount; i++) {

            if (av_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                audioIndex = i;
                __android_log_write(ANDROID_LOG_INFO, "日志找到的codec类型",
                                    av_get_media_type_string(
                                            av_context->streams[i]->codecpar->codec_type));
            }

        }
        int audioIndex_File = -1;

        sCount = av_context_File->nb_streams;

        for (int i = 0; i < sCount; i++) {

            if (av_context_File->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                audioIndex_File = i;
                __android_log_write(ANDROID_LOG_INFO, "日志找到的codec类型",
                                    av_get_media_type_string(
                                            av_context_File->streams[i]->codecpar->codec_type));
            }

        }

        if (audioIndex != -1 && audioIndex_File != -1) {

            __android_log_write(ANDROID_LOG_INFO, "日志1时长",
                                to_string(av_context->streams[audioIndex]->duration *
                                          av_q2d(av_context->streams[audioIndex]->time_base)).c_str());
            __android_log_write(ANDROID_LOG_INFO, "日志2时长",
                                to_string(av_context->streams[audioIndex]->duration *
                                          AV_TIME_BASE).c_str());

            /*       AVCodec *codec = avcodec_find_decoder_by_name("h264_mediacodec");
                     if (!codec) {
                         codec = avcodec_find_decoder(av_context->streams[audioIndex]->codecpar->codec_id);
                     }*/

            const AVCodec *codec = avcodec_find_decoder(
                    av_context->streams[audioIndex]->codecpar->codec_id);
            const AVCodec *codec_File = avcodec_find_decoder(
                    av_context_File->streams[audioIndex_File]->codecpar->codec_id);

            if (codec->capabilities & AV_CODEC_CAP_HARDWARE && av_codec_is_decoder(codec)) {
                __android_log_write(ANDROID_LOG_INFO, "日志硬件解码器：",
                                    codec->name);
            }

            void *next = 0;
            const AVCodec *NV = 0;
/*            while ((NV = av_codec_iterate(&next))) {
                if (av_codec_is_decoder(NV)) {
                    __android_log_write(ANDROID_LOG_INFO, "日志支持的解码器：",
                                        NV->name);

                }
            }*/

            AVCodecContext *av_codec = avcodec_alloc_context3(codec);
            AVCodecContext *av_codec_File = avcodec_alloc_context3(codec_File);


            avcodec_parameters_to_context(av_codec, av_context->streams[audioIndex]->codecpar);
            avcodec_parameters_to_context(av_codec_File,
                                          av_context_File->streams[audioIndex_File]->codecpar);
            __android_log_write(ANDROID_LOG_INFO, "日志iformat变化后", av_context->iformat->name);
            av_opt_set_int(av_codec, "refcounted_frames", 1, 0);
            av_opt_set_int(av_codec_File, "refcounted_frames", 1, 0);
/*            av_codec->get_format = getFormat;
            auto type = av_hwdevice_iterate_types(AV_HWDEVICE_TYPE_NONE);

            AVBufferRef *hw_ref = NULL;

            av_codec->thread_count = 20;

            av_hwdevice_ctx_create(&hw_ref, type, NULL, NULL, 0);

            av_codec->hw_device_ctx = av_buffer_ref(hw_ref);

            AVHWFramesContext *frames_ctx = 0;
            AVBufferRef *hw_frames_ref = av_hwframe_ctx_alloc(av_codec->hw_device_ctx);
            frames_ctx = (AVHWFramesContext *) (hw_frames_ref->data);
            frames_ctx->format = AV_PIX_FMT_MEDIACODEC; // 或其他硬件相关的像素格式
            frames_ctx->sw_format = AV_PIX_FMT_YUV420P; // 硬件表面的软件格式
            frames_ctx->width = av_codec->width;
            frames_ctx->height = av_codec->height;
            frames_ctx->initial_pool_size = 20; // 初始缓冲池大小，根据需要调整

            if (av_hwframe_ctx_init(hw_frames_ref) < 0) {
                // 初始化失败处理
            }

            av_codec->hw_frames_ctx = hw_frames_ref;*/

            avcodec_open2(av_codec, codec, NULL);
            avcodec_open2(av_codec_File, codec_File, NULL);

            for (int i = 0;; i++) {
                const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
                if (!config) {
                    break;
                }
            }


            AVPacket *packet = av_packet_alloc();//存储编码后的数据
            AVPacket *packet_File = av_packet_alloc();//存储编码后的数据

            AVFrame *frame = av_frame_alloc();//存储解码后的数据
            AVFrame *frame_File = av_frame_alloc();//存储解码后的数据
//            AVFrame *outFrame = av_frame_alloc();//存储解码后的数据

            AVDictionaryEntry *tag = NULL;

            ANativeWindow_setBuffersGeometry(nativeWindow, ANativeWindow_getWidth(nativeWindow),
                                             ANativeWindow_getHeight(nativeWindow),
                                             WINDOW_FORMAT_RGBA_8888);


            AVFilterContext *filterContext_in = nullptr;
            AVFilterContext *filterContext_out = nullptr;
            AVFilter *filter = nullptr;
            AVFilterGraph *graph = avfilter_graph_alloc();
            char args[512];
//            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            snprintf(args, sizeof(args),
                     "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                     av_codec->width, av_codec->height, av_codec->pix_fmt,
                     av_context->streams[audioIndex]->time_base.num,
                     av_context->streams[audioIndex]->time_base.den,
                     av_codec->sample_aspect_ratio.num,
                     av_codec->sample_aspect_ratio.den);

/*            snprintf(args, sizeof(args),
                     "video_size=1270x780:pix_fmt=%d:time_base=1/90000:pixel_aspect=2/1",
                     av_codec->pix_fmt);*/


            const AVFilter *bufferFilter = avfilter_get_by_name("buffer");
            const AVFilter *bufferSinkFilter = avfilter_get_by_name("buffersink");

            int createRe = avfilter_graph_create_filter(&filterContext_in, bufferFilter, "in", args,
                                                        NULL, graph);

            __android_log_write(ANDROID_LOG_INFO, "日志创建filter错误",
                                strcat(av_err2str(createRe), args));

            createRe = avfilter_graph_create_filter(&filterContext_out, bufferSinkFilter, "out",
                                                    NULL,
                                                    NULL, graph);

            uint8_t *opt = NULL;

            const AVOption *option = NULL;

            bool re = (long) filterContext_out == (long) (&(filterContext_out->av_class));

            string a = to_string((long) filterContext_out);
            string b = to_string((long) (&(filterContext_out->av_class)));

            option = av_opt_find(filterContext_out, "pix_fmts", NULL, 0, AV_OPT_SEARCH_CHILDREN);

            av_opt_get(filterContext_out, "pix_fmts", AV_OPT_SEARCH_CHILDREN, &opt);

            __android_log_write(ANDROID_LOG_INFO, "日志获取pix_fmts",
                                strcat((char *) opt, ""));

            AVFilterInOut *input = avfilter_inout_alloc();
            AVFilterInOut *output = avfilter_inout_alloc();

            input->filter_ctx = filterContext_out;
            input->name = av_strdup("out");
            input->next = nullptr;
            input->pad_idx = 0;

            output->filter_ctx = filterContext_in;
            output->name = av_strdup("in");
            output->next = nullptr;
            output->pad_idx = 0;
//            setpts=2*PTS
            avfilter_graph_parse_ptr(graph, "setpts=1*PTS,transpose=dir=1", &input, &output,
                                     nullptr);

            avfilter_graph_config(graph, NULL);

            FILE *out_file = fopen("/storage/emulated/0/DCIM/Camera/ffmpeg_out.mp4", "wb");
            ANativeWindow_Buffer windowBuffer;

            //导出
            const AVCodec *encode = avcodec_find_encoder(AV_CODEC_ID_H264);
            AVCodecContext *encodeCtx = avcodec_alloc_context3(encode);
            av_opt_set(encodeCtx->priv_data, "preset", "slow", 0);
            encodeCtx->bit_rate = 400000;
            encodeCtx->framerate = {25,1};
            encodeCtx->time_base = {1,25};
            encodeCtx->height = max(av_context->streams[audioIndex]->codecpar->height, av_context_File->streams[audioIndex_File]->codecpar->height);
            encodeCtx->width = max(av_codec->width, av_codec_File->width);
            encodeCtx->pix_fmt = AV_PIX_FMT_YUV420P;

            avcodec_open2(encodeCtx, encode, NULL);

            auto writeFrame = av_frame_alloc();
            auto writePacket = av_packet_alloc();

            av_frame_make_writable(writeFrame);
            writeFrame->height = encodeCtx->height;
            writeFrame->width = encodeCtx->width;
            writeFrame->format = encodeCtx->pix_fmt;

            while (av_read_frame(av_context, packet) == 0) {

                if (packet->stream_index == audioIndex) {
                    int ret = 0;
                    while (!(ret = av_read_frame(av_context_File, packet_File))) {
                        if (packet_File->stream_index == audioIndex_File) {
                            break;
                        }
                    }
                    if (ret) {
                        break;
                    }

                    // 锁定 ANativeWindow 的缓冲区
                    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                    ret = avcodec_send_packet(av_codec, packet);
                    ret |= avcodec_send_packet(av_codec_File, packet_File);

                    auto halfWidth = ANativeWindow_getWidth(nativeWindow) / 2;

                    if (ret < 0) {
                        __android_log_write(ANDROID_LOG_INFO, "日志send_packet错误",
                                            strcat(av_err2str(ret), ""));
                        break;
                    }

                    ret = avcodec_receive_frame(av_codec, frame);
                    ret |= avcodec_receive_frame(av_codec_File, frame_File);

                    if (0) {
                        __android_log_write(ANDROID_LOG_INFO, "日志pix_fmt类型",
                                            av_get_pix_fmt_name((AVPixelFormat) frame->format));
                        if (ret < 0) {
                            __android_log_write(ANDROID_LOG_INFO, "日志receive_frame",
                                                strcat(av_err2str(ret), ""));
                        }

                        if (ret != 0) {
                            ANativeWindow_unlockAndPost(nativeWindow);
                            break;
                        }

                        int size = av_image_get_buffer_size((AVPixelFormat) frame->format,
                                                            frame->width,
                                                            frame->height, 1);

                        AVFrame *temp_frame = av_frame_alloc();

                        ret = av_hwframe_transfer_data(temp_frame, frame, 0);

                        if (ret != 0) {
                            __android_log_write(ANDROID_LOG_INFO, "日志transfer_data",
                                                strcat(av_err2str(ret), ""));
                            av_frame_free(&temp_frame);
                            break;
                        }

                        uint8_t *buffer = (uint8_t *) av_malloc(size);

                        av_image_copy_to_buffer(buffer, size, (const uint8_t *const *) frame->data,
                                                frame->linesize, (AVPixelFormat) frame->format,
                                                frame->width, frame->height, 1);

                        av_freep(buffer);
                        av_frame_ref(frame, temp_frame);
                        av_frame_free(&temp_frame);
                    }

                    int addRet = av_buffersrc_add_frame(filterContext_in, frame);

                    if (addRet < 0) {
                        __android_log_write(ANDROID_LOG_INFO, "日志add_frame",
                                            strcat(av_err2str(addRet), ""));
                        ANativeWindow_unlockAndPost(nativeWindow);
                        continue;
                    }

                    addRet = av_buffersink_get_frame(filterContext_out, frame);
                    if (addRet < 0) {
                        __android_log_write(ANDROID_LOG_INFO, "日志get_frame",
                                            strcat(av_err2str(addRet), ""));
                        ANativeWindow_unlockAndPost(nativeWindow);
                        continue;
                    }

                    // 将 AVFrame 的数据复制到 ANativeWindow 的缓冲区
                    uint8_t *dstData = (uint8_t *) windowBuffer.bits;
//                    windowBuffer.stride *
                    int dstStride = windowBuffer.stride * 4; // RGBA 格式，每个像素占用 4 个字节

                    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                               frame->width, frame->height, 1);
                    int buffer_size_File = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                                    frame_File->width,
                                                                    frame_File->height, 1);

                    uint8_t *ragb_data = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));
                    uint8_t *ragb_data_File = (uint8_t *) av_malloc(
                            buffer_size_File * sizeof(uint8_t));

                    auto rgba_frame = av_frame_alloc();
                    auto rgba_frame_File = av_frame_alloc();
                    av_image_fill_arrays(rgba_frame->data, rgba_frame->linesize,
                                         ragb_data,
                                         AV_PIX_FMT_RGBA,
                                         frame->width, frame->height, 1);
                    av_image_fill_arrays(rgba_frame_File->data, rgba_frame_File->linesize,
                                         ragb_data_File,
                                         AV_PIX_FMT_RGBA,
                                         frame_File->width, frame_File->height, 1);

                    SwsContext *sws = sws_getContext(frame->width, frame->height,
                                                     av_codec->pix_fmt,
                                                     halfWidth, frame->height,
                                                     AV_PIX_FMT_RGBA,
                                                     0,
                                                     nullptr, nullptr, nullptr);

                    SwsContext *sws_File = sws_getContext(frame_File->width, frame_File->height,
                                                          av_codec_File->pix_fmt,
                                                          halfWidth, frame_File->height,
                                                          AV_PIX_FMT_RGBA,
                                                          0,
                                                          nullptr, nullptr, nullptr);

                    sws_scale(sws, frame->data, frame->linesize, 0, frame->height,
                              rgba_frame->data, rgba_frame->linesize);

                    sws_scale(sws_File, frame_File->data, frame_File->linesize, 0,
                              frame_File->height,
                              rgba_frame_File->data, rgba_frame_File->linesize);

                    sws_freeContext(sws);
                    sws_freeContext(sws_File);

//画左边
                    for (int j = windowBuffer.height / 2 - frame->height / 2, i = 0;
                         j < windowBuffer.height / 2 + frame->height / 2; i++, j++) {
                        memcpy(dstData + j * dstStride,
                               ragb_data + i * rgba_frame->linesize[0],
                               min(rgba_frame->linesize[0], dstStride / 2));
                    }

                    int startW = min(rgba_frame->linesize[0], dstStride / 2);

//画右边
                    for (int j = windowBuffer.height / 2 - frame_File->height / 2, i = 0;
                         j < windowBuffer.height / 2 + frame_File->height / 2; i++, j++) {

                        memcpy(dstData + j * dstStride + startW,
                               ragb_data_File + i * rgba_frame_File->linesize[0],
                               min(rgba_frame_File->linesize[0], dstStride - startW));
                    }

                    writeFrame->pts = frame_File->pts;


                    av_frame_ref(writeFrame,frame_File);

                    ret = avcodec_send_frame(encodeCtx, writeFrame);
                    if (ret >= 0) {
                        avcodec_receive_packet(encodeCtx, writePacket);
                        //写到文件

                        fwrite(writePacket->data, 1, writePacket->size, out_file);
                    }else{
                        __android_log_write(ANDROID_LOG_INFO, "日志 编码错误",
                                            strcat(av_err2str(ret), ""));
                        av_frame_unref(writeFrame);
                    }

                    // 解锁 ANativeWindow
                    ANativeWindow_unlockAndPost(nativeWindow);

                    tag = NULL;
                    while ((tag = av_dict_get(frame->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                        __android_log_write(ANDROID_LOG_INFO, "日志frame的metadata->",

                                            strcat(strcat(tag->key, ":"), tag->value));
                    av_frame_free(&rgba_frame_File);
                    av_frame_free(&rgba_frame);
                    av_free(ragb_data);
                    av_free(ragb_data_File);

                } else {
                    /*__android_log_write(ANDROID_LOG_INFO, "日志解析资源时遇到其他轨道类型",
                                        av_get_media_type_string(
                                                av_context->streams[packet->stream_index]->codecpar->codec_type));*/
                }
            }
            __android_log_write(ANDROID_LOG_INFO, "日志", "解析结束");

            // 释放 ANativeWindow
            ANativeWindow_release(nativeWindow);

            av_packet_free(&packet);
            av_packet_free(&packet_File);
            av_frame_free(&frame);
            av_frame_free(&frame_File);
            avcodec_close(av_codec);
            avcodec_close(av_codec_File);
            avformat_close_input(&av_context);
            avformat_close_input(&av_context_File);
            int ret = -1;
            ret = avcodec_send_frame(encodeCtx, writeFrame);
            if (ret >= 0) {
                ret = avcodec_receive_packet(encodeCtx, writePacket);
                if (ret >= 0) {
                    fwrite(writePacket->data, 1, writePacket->size, out_file);
                }
            }
            av_packet_free(&writePacket);
            av_frame_free(&writeFrame);
            fclose(out_file);
            avcodec_free_context(&encodeCtx);
        }
    }

    if (success < 0) {
        char *a;
        size_t b;
        av_strerror(success, a, b
        );
        __android_log_write(ANDROID_LOG_WARN,
                            "日志", a);
//        avformat_free_context(av_context);
        av_context = nullptr;
        success = 0;
    }

    return (long)
            av_context;
}








