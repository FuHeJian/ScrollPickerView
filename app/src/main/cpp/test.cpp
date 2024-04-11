#include "iostream"
#include "android/log.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <any>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <media/NdkImage.h>
#include <SLES/OpenSLES_Android.h>
#include <filesystem>

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <elf.h>
#include "dlfcn.h"
#include <thread>
#include "hook.h"

#include "set"
#include "map"

#include "video_player.h"

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
#include "libavutil/dict.h"
#include "fcntl.h"
}

#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC

using namespace std;

extern "C" const AVInputFormat ff_android_camera_demuxer;

HOOK_INFO moc;
HOOK_INFO fopenMoc;
HOOK_INFO openMoc;

jint native_op(JNIEnv *env, jobject thiz, jstring p, jobject surface);

int my_open(const char *const pathname, int flags, mode_t modes);

FILE *_Nullable my_fopen(const char *_Nonnull __path, const char *_Nonnull __mode);

const JNINativeMethod nativeMethods[] = {
        {"op", "(Ljava/lang/String;Landroid/view/Surface;)I", (void *) native_op},
        {"op", "(Ljava/lang/String;Landroid/view/Surface;)I", (void *) native_op}
};

void log_callback(void *obj, int status, const char *info, va_list val) {
    __android_log_vprint(ANDROID_LOG_INFO, "ffmpeg日志",
                         info, val);
}


jbyteArray getByteAddress(JNIEnv *env, jobject thiz, jobject objBuffer){
    void * add = env->GetDirectBufferAddress(objBuffer);
}

JNINativeMethod VideoControlerNativeMethod[] = {
        {"init",           "()V",                                         (void *) init},
        {"loadVideo",      "(Landroid/view/Surface;Ljava/lang/String;)I", (void *) loadVideo},
        {"parseErrorCode", "(I)Ljava/lang/String;",                       (void *) parseErrorCode},
};


static int registerNativeMethods(JNIEnv *env) {
    int result = -1;
    jbyteArray arr = env->NewByteArray(2);
    jclass class_hello = env->FindClass("com/fhj/mvi_demo/BaseActivity");
    if (env->RegisterNatives(class_hello, nativeMethods,
                             sizeof(nativeMethods) / sizeof(nativeMethods[0])) == JNI_OK) {
        result = 0;
    }

    class_hello = env->FindClass("com/fhj/mvi_demo/VideoControler");
    if (env->RegisterNatives(class_hello, VideoControlerNativeMethod,
                             sizeof(VideoControlerNativeMethod) /
                             sizeof(VideoControlerNativeMethod[0])) == JNI_OK) {
        result = 0;
    }
    return result;
}

AVPixelFormat getFormat(struct AVCodecContext *s, const enum AVPixelFormat *fmt) {
    const AVInputFormat *put = av_find_input_format("dfafd");
    __android_log_write(ANDROID_LOG_INFO, "日志调用getFormat",
                        "日志调用getFormat");
    __android_log_write(ANDROID_LOG_INFO, "日志调用getFormat",
                        put->name);
    return fmt[0];
}

static void add_stream(AVStream **ost, AVFormatContext *oc,
                       const AVCodec **codec,
                       enum AVCodecID codec_id) {

    *codec = avcodec_find_encoder(codec_id);

    *ost = avformat_new_stream(oc, *codec);

}

jlong openVideo(char *path, ANativeWindow *nativeWindow) {
    AVFormatContext *av_context = 0;
    AVFormatContext *av_context_File = 0;

    void *i = 0;
//  初始化av_context信息，包括流的数量，媒体格式
    void *op = 0;
    int success = 0;
    const AVInputFormat *androidFormat = &ff_android_camera_demuxer;

    success |= avformat_open_input(&av_context_File, path, NULL, NULL);

    if (success == 0) {

        //初始化各个流的参数
        int streamIndex_File = avformat_find_stream_info(av_context_File, NULL);

        success = 1;

        auto findRe = streamIndex_File;

        if (findRe < 0) {
            return 10;
        }

        int audioIndex = -1;

        int audioIndex_File = -1;

        int sCount = av_context_File->nb_streams;

        for (int i = 0; i < sCount; i++) {

            if (av_context_File->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                audioIndex_File = i;
                __android_log_write(ANDROID_LOG_INFO, "日志找到的codec类型",
                                    av_get_media_type_string(
                                            av_context_File->streams[i]->codecpar->codec_type));
            }

        }

        AVDictionary *avDictionary = NULL;
/*      av_dict_set(&avDictionary, "framerate",
                    to_string(av_context_File->streams[audioIndex_File]->r_frame_rate.num).c_str(),
                    0);*/
        success = avformat_open_input(&av_context, NULL, androidFormat, NULL);

        int streamIndex = avformat_find_stream_info(av_context, NULL);
        findRe = streamIndex;
        if (findRe < 0) {
            return 10;
        }

        sCount = av_context->nb_streams;

        for (int i = 0; i < sCount; i++) {

            if (av_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                audioIndex = i;
            }
            __android_log_write(ANDROID_LOG_INFO, "日志找到的codec类型",
                                av_get_media_type_string(
                                        av_context->streams[i]->codecpar->codec_type));
        }


        if (audioIndex != -1 && audioIndex_File != -1) {

            __android_log_write(ANDROID_LOG_INFO, "日志1时长",
                                to_string(av_context->streams[audioIndex]->duration *
                                          av_q2d(av_context->streams[audioIndex]->time_base)).c_str());
            __android_log_write(ANDROID_LOG_INFO, "日志2时长",
                                to_string(av_context->streams[audioIndex]->duration *
                                          AV_TIME_BASE).c_str());

            /*AVCodec *codec = avcodec_find_decoder_by_name("h264_mediacodec");
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

            AVPixelFormat fmtfmt = getFormat(av_codec, codec->pix_fmts);

            __android_log_write(ANDROID_LOG_INFO, "日志iformat变化后", av_context->iformat->name);
            __android_log_write(ANDROID_LOG_INFO, to_string(fmtfmt).c_str(),
                                av_context->iformat->name);
            av_opt_set_int(av_codec, "refcounted_frames", 1, 0);
            av_opt_set_int(av_codec_File, "refcounted_frames", 1, 0);
            av_codec->get_format = getFormat;
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

            av_codec->hw_frames_ctx = hw_frames_ref;

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
//          "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
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

            string out_file = "/storage/emulated/0/DCIM/Camera/ffmpeg_out.mp4";
            ANativeWindow_Buffer windowBuffer;

            //导出
            AVFormatContext *oc;

            void *iterate = 0;
            const AVOutputFormat *outFmt;
            while ((outFmt = av_muxer_iterate(&iterate))) {
                if (out_file.c_str() && outFmt->extensions &&
                    av_match_ext(out_file.c_str(), outFmt->extensions)) {
                    break;
                }
            }
            avformat_alloc_output_context2(&oc, outFmt, NULL, out_file.c_str());

            oc->io_open(oc, &oc->pb, out_file.c_str(), AVIO_FLAG_WRITE | oc->avio_flags, NULL);

            AVStream *video_st;
            AVStream *audio_st;
            const AVCodec *video_codec;
            const AVCodec *audio_codec;
            AVCodecContext *video_codec_ctx;
            AVCodecContext *audio_codec_ctx;

            AVPacket *out_packet = av_packet_alloc();
            AVFrame *out_frame = av_frame_alloc();

            if (outFmt->video_codec != AV_CODEC_ID_NONE) {
                add_stream(&video_st, oc, &video_codec, outFmt->video_codec);
                video_codec_ctx = avcodec_alloc_context3(video_codec);
                video_st->index = 0;
                video_codec_ctx->codec_id = outFmt->video_codec;

                video_codec_ctx->bit_rate = 8 * 10240;
                /* Resolution must be a multiple of two. */
                video_codec_ctx->width = 1080;
                video_codec_ctx->height = 1920;
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                 * of which frame timestamps are represented. For fixed-fps content,
                 * timebase should be 1/framerate and timestamp increments should be
                 * identical to 1. */
//                video_st->time_base = (AVRational) {1, STREAM_FRAME_RATE};
                video_st->time_base = (AVRational) {1, 25};
                video_codec_ctx->time_base = video_st->time_base;

                video_codec_ctx->gop_size = 12; /* emit one intra frame every twelve frames at most */
                video_codec_ctx->pix_fmt = STREAM_PIX_FMT;
                if (video_codec_ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                    /* just for testing, we also add B-frames */
                    video_codec_ctx->max_b_frames = 2;
                }
                if (video_codec_ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                    /* Needed to avoid using macroblocks in which some coeffs overflow.
                     * This does not happen with normal video, it just happens here as
                     * the motion of the chroma plane does not match the luma plane. */
                }
                video_codec_ctx->mb_decision = 2;

                avcodec_parameters_from_context(video_st->codecpar, video_codec_ctx);
                avcodec_open2(video_codec_ctx, video_codec, NULL);
                out_frame->width = video_codec_ctx->width;
                out_frame->height = video_codec_ctx->height;
                out_frame->format = video_codec_ctx->pix_fmt;
                av_frame_get_buffer(out_frame, 0);
            }

/*            if (outFmt->audio_codec != AV_CODEC_ID_NONE) {
                add_stream(&audio_st, oc, &audio_codec, outFmt->audio_codec);
                audio_codec_ctx = avcodec_alloc_context3(audio_codec);
                audio_codec_ctx->bit_rate = 64000;
                audio_codec_ctx->sample_rate = 44100;
                audio_codec_ctx->sample_fmt = (audio_codec)->sample_fmts ?
                                              (audio_codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

                audio_st->time_base = (AVRational) {1, audio_codec_ctx->sample_rate};

                avcodec_parameters_from_context(audio_st->codecpar, audio_codec_ctx);
                avcodec_open2(audio_codec_ctx, audio_codec, NULL);
            }*/

            av_dump_format(oc, 0, out_file.c_str(), 1);

            avformat_write_header(oc, NULL);

            av_frame_make_writable(out_frame);
            auto frame_yuv = av_frame_alloc();

            int write_pts_time = 0;

            int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                       av_context->streams[audioIndex]->codecpar->width,
                                                       av_context->streams[audioIndex]->codecpar->height,
                                                       1);
            uint8_t *ragb_data = (uint8_t *) av_malloc(buffer_size * sizeof(uint8_t));
            auto rgba_frame = av_frame_alloc();

/*          int buffer_size_File = av_image_get_buffer_size(AV_PIX_FMT_RGBA,
                                                            av_context_File->streams[audioIndex_File]->codecpar->width, av_context_File->streams[audioIndex_File]->codecpar->height, 1);
            uint8_t *ragb_data_File = (uint8_t *) av_malloc(
                    buffer_size_File * sizeof(uint8_t));
            auto rgba_frame_File = av_frame_alloc();*/

            while (av_read_frame(av_context, packet) == 0) {

                if (packet->stream_index == audioIndex) {
                    int ret = 0;
/*                    while (!(ret = av_read_frame(av_context_File, packet_File))) {
                        if (packet_File->stream_index == audioIndex_File) {
                            break;
                        }
                    }
                    if (ret) {
                        break;
                    }*/
                    // 锁定 ANativeWindow 的缓冲区
                    ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
                    ret = avcodec_send_packet(av_codec, packet);
//                    ret |= avcodec_send_packet(av_codec_File, packet_File);

                    auto halfWidth = ANativeWindow_getWidth(nativeWindow) / 2;

                    if (ret < 0) {
                        __android_log_write(ANDROID_LOG_INFO, "日志send_packet错误",
                                            strcat(av_err2str(ret), ""));
                        break;
                    }

                    ret = avcodec_receive_frame(av_codec, frame);
//                    ret |= avcodec_receive_frame(av_codec_File, frame_File);

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

/*                    int addRet = av_buffersrc_add_frame(filterContext_in, frame);

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
                    }*/

                    // 将 AVFrame 的数据复制到 ANativeWindow 的缓冲区
                    uint8_t *dstData = (uint8_t *) windowBuffer.bits;
                    //windowBuffer.stride *
                    int dstStride = windowBuffer.stride * 4; // RGBA 格式，每个像素占用 4 个字节


                    av_image_fill_arrays(rgba_frame->data, rgba_frame->linesize,
                                         ragb_data,
                                         AV_PIX_FMT_RGBA,
                                         frame->width, frame->height, 1);
/*                    av_image_fill_arrays(rgba_frame_File->data, rgba_frame_File->linesize,
                                         ragb_data_File,
                                         AV_PIX_FMT_RGBA,
                                         frame_File->width, frame_File->height, 1);*/

                    SwsContext *sws = sws_getContext(frame->width, frame->height,
                                                     av_codec->pix_fmt,
                                                     halfWidth, frame->height,
                                                     AV_PIX_FMT_RGBA,
                                                     0,
                                                     nullptr, nullptr, nullptr);

/*                    SwsContext *sws_File = sws_getContext(frame_File->width, frame_File->height,
                                                          av_codec_File->pix_fmt,
                                                          halfWidth, frame_File->height,
                                                          AV_PIX_FMT_RGBA,
                                                          0,
                                                          nullptr, nullptr, nullptr);*/

                    sws_scale(sws, frame->data, frame->linesize, 0, frame->height,
                              rgba_frame->data, rgba_frame->linesize);

/*                    sws_scale(sws_File, frame_File->data, frame_File->linesize, 0,
                              frame_File->height,
                              rgba_frame_File->data, rgba_frame_File->linesize);*/

                    sws_freeContext(sws);
//                    sws_freeContext(sws_File);

/*                    frame_yuv->height = out_frame->height;
                    frame_yuv->width = out_frame->width;
                    frame_yuv->format = AV_PIX_FMT_YUV420P;
                    av_frame_get_buffer(frame_yuv, 0);

                    SwsContext *sws_Write_yuv = sws_getContext(frame->width, frame->height,
                                                               av_codec->pix_fmt,
                                                               halfWidth, frame->height,
                                                               (AVPixelFormat) frame_File->format,
                                                               0,
                                                               nullptr, nullptr, nullptr);

                    sws_scale(sws_Write_yuv, frame->data, frame->linesize, 0, frame->height,
                              frame_yuv->data, frame_yuv->linesize);

                    sws_freeContext(sws_Write_yuv);*/

//画左边
                    for (int j = windowBuffer.height / 2 - frame->height / 2, i = 0;
                         j < windowBuffer.height / 2 + frame->height / 2; i++, j++) {
                        memcpy(dstData + j * dstStride,
                               ragb_data + i * rgba_frame->linesize[0],
                               min(rgba_frame->linesize[0], dstStride / 2));
                    }

                    int startW = min(rgba_frame->linesize[0], dstStride / 2);

//画右边
/*                    for (int j = windowBuffer.height / 2 - frame_File->height / 2, i = 0;
                         j < windowBuffer.height / 2 + frame_File->height / 2; i++, j++) {

                        memcpy(dstData + j * dstStride + startW,
                               ragb_data_File + i * rgba_frame_File->linesize[0],
                               min(rgba_frame_File->linesize[0], dstStride - startW));

                    }*/

/*                    // ------------------写到文件-----------------------
                    int hw = out_frame->width / 2;
                    int hhw = hw / 2;

//                    __android_log_write(ANDROID_LOG_INFO, "日志准备写文件",
//                                        "");
//                    memset(out_frame->data[0],0,out_frame->height * out_frame->linesize[0]);
//                    memset(out_frame->data[1],0,out_frame->height * out_frame->linesize[1] /2);
//                    memset(out_frame->data[2],0,out_frame->height * out_frame->linesize[2]/2);
                    for (int h = 0; h < out_frame->height; h++) {
                        for (int w = 0; w < out_frame->width; w++) {
                            if (w < hw) {
                                if (frame_yuv->height <= h || w >= frame_yuv->width) {
                                    continue;
                                }
                                out_frame->data[0][h * out_frame->linesize[0] +
                                                   w] = frame_yuv->data[0][
                                        h * frame_yuv->linesize[0] + w];
                            } else {
                                if (frame_File->height <= h || w >= frame_File->width)break;
                                out_frame->data[0][h * out_frame->linesize[0] +
                                                   w] = frame_File->data[0][
                                        h * frame_File->linesize[0] + w];
                            }
                            *//*if(w<hw){
                                out_frame->data[1][h*out_frame->linesize[1] + w] = 0;
                                out_frame->data[2][h*out_frame->linesize[2] + w] = 0;
                            }*//*
                        }
                    }

                    for (int h = 0; h < out_frame->height / 2; h++) {
                        for (int w = 0; w < hw; w++) {
                            if (w >= hhw) {
                                if (frame_File->height / 2 <= h || w >= frame_File->width / 2)break;
                                out_frame->data[1][h * out_frame->linesize[1] +
                                                   w] = frame_File->data[1][
                                        h * frame_File->linesize[1] + w];
                                out_frame->data[2][h * out_frame->linesize[2] +
                                                   w] = frame_File->data[2][
                                        h * frame_File->linesize[2] + w];
                            } else {
                                if (frame_yuv->height / 2 <= h || w >= frame_yuv->width / 2)
                                    continue;
                                out_frame->data[1][h * out_frame->linesize[1] +
                                                   w] = frame_yuv->data[1][
                                        h * frame_yuv->linesize[1] + w];
                                out_frame->data[2][h * out_frame->linesize[2] +
                                                   w] = frame_yuv->data[2][
                                        h * frame_yuv->linesize[2] + w];
                            }
                        }
                    }

                    out_frame->pts = av_rescale_q(frame_File->pts,
                                                  av_context_File->streams[audioIndex_File]->time_base,
                                                  video_st->time_base);
//                    * 1/90000.0
                    __android_log_write(ANDROID_LOG_INFO, "日志当前帧的pts:",
                                        to_string(out_frame->pts).c_str());
                    __android_log_write(ANDROID_LOG_INFO, "日志当前帧的pts_time_base:",
                                        to_string(av_q2d(out_frame->time_base)).c_str());
                    __android_log_write(ANDROID_LOG_INFO, "日志当前帧的pts_codec_time_base:",
                                        to_string(av_q2d(video_codec_ctx->time_base)).c_str());
                    __android_log_write(ANDROID_LOG_INFO, "日志当前帧的pts_packet_time_base:",
                                        to_string(av_q2d(video_codec_ctx->pkt_timebase)).c_str());
                    avcodec_send_frame(video_codec_ctx, out_frame);
                    ret = avcodec_receive_packet(video_codec_ctx, out_packet);
//                    __android_log_write(ANDROID_LOG_INFO, "日志写文件",
//                                        "");
                    if (ret == 0) {
                        av_write_frame(oc, out_packet);
                    }*/

                    // ------------------写到文件-END----------------------

                    // 解锁 ANativeWindow
                    ANativeWindow_unlockAndPost(nativeWindow);
                    tag = NULL;
                    while ((tag = av_dict_get(frame->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                        __android_log_write(ANDROID_LOG_INFO, "日志frame的metadata->",
                                            strcat(strcat(tag->key, ":"), tag->value));
//                    av_frame_free(&rgba_frame_File);
//                    av_frame_free(&rgba_frame);
//                    av_free(ragb_data);
//                    av_free(ragb_data_File);

                } else {
                    /*__android_log_write(ANDROID_LOG_INFO, "日志解析资源时遇到其他轨道类型",
                                        av_get_media_type_string(
                                                av_context->streams[packet->stream_index]->codecpar->codec_type));*/
                }
            }
            __android_log_write(ANDROID_LOG_INFO, "日志", "解析结束");
            av_write_trailer(oc);
            // 释放 ANativeWindow
            ANativeWindow_release(nativeWindow);
            av_packet_free(&packet);
            av_packet_free(&packet_File);
            av_packet_free(&out_packet);
            av_frame_free(&frame);
            av_frame_free(&frame_File);
            avcodec_close(av_codec);
            avcodec_close(av_codec_File);
            avformat_close_input(&av_context);
            avformat_close_input(&av_context_File);
            av_packet_free(&out_packet);
            av_frame_free(&out_frame);
            av_frame_free(&frame_yuv);
        }
    }

    if (success < 0) {
        char *a;
        size_t b;
        __android_log_write(ANDROID_LOG_WARN,
                            "日志", a);
//        avformat_free_context(av_context);
        av_context = nullptr;
        success = 0;
    }

    return (long)
            av_context;
}

void open_elf() {
    std::vector<std::string> list = logAllSo();
    for (const auto &item: list) {
        __android_log_print(ANDROID_LOG_INFO, "日志", "so文件%s",
                            item.c_str());

    }
    Elf64_Ehdr *headerInfo = getHeader("libc.so");
    if (headerInfo == 0)return;
    __android_log_print(ANDROID_LOG_INFO, "日志", "so文件%d",
                        headerInfo->e_ident[EI_CLASS]);
    Elf64_Addr baseAddr = (Elf64_Addr) headerInfo;
    Elf64_Phdr dynamic = getDynamicPHT(headerInfo);
    ELF64INFO segmentInfo = getELF64INFO(baseAddr, dynamic);

    uint32_t gotTable = getFunctionSymbolIndex("open", segmentInfo.GNUHASHTABLE,
                                               segmentInfo.STRTAB,
                                               segmentInfo.SYMTAB);

    if (gotTable != -1 && openMoc.oldPtr == 0) {
        Elf64_Sym funSym = segmentInfo.SYMTAB[gotTable];
        Elf64_Addr *ptr = (Elf64_Addr *) (baseAddr + funSym.st_value);
        uintptr_t oldPtr = hookGotPltItem(segmentInfo, (void *) my_open, gotTable, STT_FUNC);
        openMoc.oldPtr = (void *) (oldPtr);
        __android_log_print(ANDROID_LOG_INFO, "日志", "libjavacore文件函数地址%lx", oldPtr);
        __android_log_print(ANDROID_LOG_INFO, "日志", "libjavacore2文件函数地址%llx", oldPtr - baseAddr);
    }

    gotTable = getFunctionSymbolIndex("fopen", segmentInfo.GNUHASHTABLE,
                                      segmentInfo.STRTAB,
                                      segmentInfo.SYMTAB);

    if (gotTable != -1 && fopenMoc.oldPtr == 0) {
        Elf64_Sym funSym = segmentInfo.SYMTAB[gotTable];
        Elf64_Addr *ptr = (Elf64_Addr *) (baseAddr + funSym.st_value);
        uintptr_t oldPtr = hookGotPltItem(segmentInfo, (void *) my_fopen, gotTable, STT_FUNC);
        fopenMoc.oldPtr = (void *) (oldPtr);
    }

    __android_log_print(ANDROID_LOG_INFO, "日志", "当前文件%d",
                        headerInfo->e_ident[EI_CLASS]);

    Elf64_Ehdr *selfHeaderInfo = getSelfHeader();

    Elf64_Addr selfBaseAddr = (Elf64_Addr) selfHeaderInfo;
    __android_log_print(ANDROID_LOG_INFO, "日志", "%d",
                        selfHeaderInfo->e_ident[EI_CLASS]);
    Elf64_Phdr selfDynamic = getDynamicPHT(selfHeaderInfo);
    ELF64INFO selfSegmentInfo = getELF64INFO(selfBaseAddr, selfDynamic);

    uint32_t selfGotTable = getFunctionSymbolIndex("open", selfSegmentInfo.GNUHASHTABLE,
                                                   selfSegmentInfo.STRTAB,
                                                   selfSegmentInfo.SYMTAB);

    if (selfGotTable != -1) {
        uint32_t ptr = findFuncPtr(selfSegmentInfo, selfGotTable);
        __android_log_print(ANDROID_LOG_INFO, "日志", "当前文件函数地址%lx", ptr);
    }

}

int my_open(const char *const pathname, int flags, mode_t modes) {
    __android_log_print(ANDROID_LOG_INFO, "日志", "open打开文件%s", pathname);
    return ((int (*)(const char *const, int, mode_t)) openMoc.oldPtr)(pathname, flags, modes);
}

FILE *_Nullable my_fopen(const char *_Nonnull __path, const char *_Nonnull __mode) {
    __android_log_print(ANDROID_LOG_INFO, "日志", "fopen打开文件%s", __path);
    return ((FILE *(*)(const char *, const char *)) fopenMoc.oldPtr)(__path, __mode);
}


jint native_op(JNIEnv *env, jobject thiz, jstring p, jobject surface) {

    auto path = (char *) env->GetStringUTFChars(p, NULL);
    // 获取 ANativeWindow
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    try {
        openVideo(path, nativeWindow);
    } catch (...) {
        __android_log_write(ANDROID_LOG_INFO, "日志写出到文件发生错误", "err");
    }
    return 1;

}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_1) == JNI_OK) {
        if (registerNativeMethods(env) == JNI_OK) {
            result = JNI_VERSION_1_6;
        }
    }
//    av_log_set_callback(log_callback);
//    open_elf();
    return result;
}