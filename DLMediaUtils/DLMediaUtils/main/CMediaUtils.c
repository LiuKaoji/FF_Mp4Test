//
//  CMediaUtils.c
//  TubeD
//
//  Created by kaoji on 2018/8/22.
//  Copyright © 2018年 kaoji. All rights reserved.
//

#include "CMediaUtils.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/put_bits.h>
#include <pthread/pthread.h>

#pragma mark - ~~~~~~~~~~~~~~~~~~混流音频视频~~~~~~~~~~~~~~~~~~
#pragma mark - FFMPEG
/*
 FIX: H.264 in some container format (FLV, MP4, MKV etc.) need
 "h264_mp4toannexb" bitstream filter (BSF)
 *Add SPS,PPS in front of IDR frame
 *Add start code ("0,0,0,1") in front of NALU
 H.264 in some container (MPEG2TS) don't need this BSF.
 */
//'1': Use H.264 Bitstream Filter
#define USE_H264BSF 0

/*
 FIX:AAC in some container format (FLV, MP4, MKV etc.) need
 "aac_adtstoasc" bitstream filter (BSF)
 */
//'1': Use AAC Bitstream Filter
#define USE_AACBSF 0

parameter* logParam = NULL;

void* callback_thread(void *p1)//此处用的是一个线程
{
    //do something
    parameter* p = (parameter*)p1 ;
    logParam = p;
    printf("GetCallBack print! \n");
    p->callback(p->str);//函数指针执行函数
    return  0;
}

//留给应用层的接口函数
int SetCallBackFun(char *logText, pcb callback)
{
    printf("SetCallBackFun print! \n");
    parameter *p = malloc(sizeof(parameter)) ;
    p->str  = logText;
    p->callback = callback;

    //创建线程
    pthread_t logThread;
    pthread_create(&logThread,NULL,callback_thread,(void *) p);
    pthread_join(logThread,NULL);
    return 0;
}



int MuxVideo(const char *vPath,const char *aPath,const char *outPath)
{
    
    AVOutputFormat *ofmt = NULL;
    //Input AVFormatContext and Output AVFormatContext
    AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL,*ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex_v=-1,videoindex_out=-1;
    int audioindex_a=-1,audioindex_out=-1;
    int frame_index=0;
    int64_t cur_pts_v=0,cur_pts_a=0;
    
    const char *in_filename_v = vPath;
    const char *in_filename_a = aPath;
    
    const char *out_filename = outPath;//Output file URL
    av_register_all();
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
        printf( "Could not open input file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
        printf( "Failed to retrieve input stream information");
        goto end;
    }
    
    if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
        printf( "Could not open input file.");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
        printf( "Failed to retrieve input stream information");
        goto end;
    }
    printf("===========Input Information==========\n");
    av_dump_format(ifmt_ctx_v, 0, in_filename_v, 0);
    av_dump_format(ifmt_ctx_a, 0, in_filename_a, 0);
    printf("======================================\n");
    //Output
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        printf( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    
    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        //Create output AVStream according to input AVStream
        if(ifmt_ctx_v->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            AVStream *in_stream = ifmt_ctx_v->streams[i];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
            videoindex_v=i;
            if (!out_stream) {
                printf( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            videoindex_out=out_stream->index;
            if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                printf( "Failed to copy context from input to output stream codec context\n");
                goto end;
            }
            out_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            break;
        }
    }
    
    for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        //Create output AVStream according to input AVStream
        if(ifmt_ctx_a->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO){
            AVStream *in_stream = ifmt_ctx_a->streams[i];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
            audioindex_a=i;
            if (!out_stream) {
                printf( "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }
            audioindex_out=out_stream->index;
            //Copy the settings of AVCodecContext
            if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                printf( "Failed to copy context from input to output stream codec context\n");
                goto end;
            }
            out_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            
            break;
        }
    }
    
    printf("==========Output Information==========\n");
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    printf("======================================\n");
    //Open output file
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
            printf( "Could not open output file '%s'", out_filename);
            goto end;
        }
    }
    //Write file header
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        printf( "Error occurred when opening output file\n");
        goto end;
    }
    
    
    //FIX
#if USE_H264BSF
    AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
    AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");
#endif
    
    while (1) {
        AVFormatContext *ifmt_ctx;
        int stream_index=0;
        AVStream *in_stream, *out_stream;
        
        //Get an AVPacket
        if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0){
            ifmt_ctx=ifmt_ctx_v;
            stream_index=videoindex_out;
            
            if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                do{
                    in_stream  = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];
                    
                    if(pkt.stream_index==videoindex_v){
                        //Simple Write PTS
                        if(pkt.pts==AV_NOPTS_VALUE){
                            //Write PTS
                            AVRational time_base1=in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts=pkt.pts;
                            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }
                        
                        cur_pts_v=pkt.pts;
                        break;
                    }
                }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else{
                break;
            }
        }else{
            ifmt_ctx=ifmt_ctx_a;
            stream_index=audioindex_out;
            if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                do{
                    in_stream  = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];
                    
                    if(pkt.stream_index==audioindex_a){
                        
                        //Simple Write PTS
                        if(pkt.pts==AV_NOPTS_VALUE){
                            //Write PTS
                            AVRational time_base1=in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
                            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            pkt.dts=pkt.pts;
                            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_a=pkt.pts;
                        
                        break;
                    }
                }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
            }else{
                break;
            }
            
        }
        
        //FIX:Bitstream Filter
#if USE_H264BSF
        av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
        av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
        
        
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index=stream_index;
        
        char buffer[80];
        sprintf(buffer, "Write 1 Packet. size:%5d\tpts:%lld\n",pkt.size,pkt.pts);
        puts(buffer);
        logParam->str = buffer;
        callback_thread(logParam);
 
        //Write
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            printf( "Error muxing packet\n");
            break;
        }
        av_free_packet(&pkt);
        
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);
    
#if USE_H264BSF
    av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
    av_bitstream_filter_close(aacbsfc);
#endif
    
end:
    avformat_close_input(&ifmt_ctx_v);
    avformat_close_input(&ifmt_ctx_a);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf( "Error occurred.\n");
        return -1;
    }
    return 0;
}


#pragma mark - ~~~~~~~~~~~~~~~~~~分离音频~~~~~~~~~~~~~~~~~~

typedef struct ADTSContext
{
    int write_adts;
    int objecttype;
    int sample_rate_index;
    int channel_conf;
} *tt;

#define ADTS_HEADER_SIZE 7/*ADTS 头中相对有用的信息 采样率、声道数、帧长度。想想也是，我要是解码器的话，你给我一堆得AAC音频ES流我也解不出来。每一个带ADTS头信息的AAC流会清晰的告送解码器他需要的这些信息。*/

int ff_adts_write_frame_header(struct ADTSContext *ctx,
                               uint8_t *buf, int size, int pce_size)
{
    PutBitContext pb;
    
    init_put_bits(&pb, buf, ADTS_HEADER_SIZE);
    
    /* adts_fixed_header */
    put_bits(&pb, 12, 0xfff);   /* syncword */
    put_bits(&pb, 1, 0);        /* ID */
    put_bits(&pb, 2, 0);        /* layer */
    put_bits(&pb, 1, 1);        /* protection_absent */
    put_bits(&pb, 2, ctx->objecttype); /* profile_objecttype */
    put_bits(&pb, 4, ctx->sample_rate_index);
    put_bits(&pb, 1, 0);        /* private_bit */
    put_bits(&pb, 3, ctx->channel_conf); /* channel_configuration */
    put_bits(&pb, 1, 0);        /* original_copy */
    put_bits(&pb, 1, 0);        /* home */
    
    /* adts_variable_header */
    put_bits(&pb, 1, 0);        /* copyright_identification_bit */
    put_bits(&pb, 1, 0);        /* copyright_identification_start */
    put_bits(&pb, 13, ADTS_HEADER_SIZE + size + pce_size); /* aac_frame_length */
    put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness 0x7ff说明时可变码率 */
    put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */
    
    flush_put_bits(&pb);
    
    return 0;
}

int DemuxAAC(const char *vPath,const char *outPath)
{
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex=-1,audioindex=-1;
    
    const char *out_filename_a = outPath;
    
    av_register_all();
    
    //Input
    if ((ret = avformat_open_input(&ifmt_ctx, vPath, 0, 0)) < 0) {
        printf( "Could not open input file.");
        return -1;
    }
    
    /* 解析H264  的配置信息 ：
     1. ffmpeg的avformat_find_stream_info函数可以取得音视频媒体多种，比如播放持续时间、音视频压缩格式、音轨信息、字幕信息、帧率、采样率等;
     2. ff_h264_decode_extradata 解析extradata；
     3. 如果音频数据是AAC流，在解码时需要ADTS(Audio Data Transport Stream)头部，不管是容器封装还是流媒体，没有这个，一般都是不能播放的。很多朋友在做AAC流播放时遇到播不出声音，很可能就是这个原因导致。
     
     ADTS所需的数据仍然是放在上面的扩展数据extradata中，我们需要先解码这个扩展数据，然后再从解码后的数据信息里面重新封装成ADTS头信息，加到每一帧AAC数据之前再送解码器，这样就可以正常解码了。*/
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf( "Failed to retrieve input stream information");
        return -1;
    }
    
    
    
    videoindex=-1;
    for(i=0; i<ifmt_ctx->nb_streams; i++) {
        if(ifmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex=i;
        }else if(ifmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){
            audioindex=i;
        }
    }
    //Dump Format------------------
    printf("\nInput Video===========================\n");
    av_dump_format(ifmt_ctx, 0, vPath, 0);
    printf("%s",ifmt_ctx);
    printf("\n======================================\n");
    
    FILE *fp_audio=fopen(out_filename_a,"wb+");
    if (NULL == fp_audio) {
        printf( "Could not open audio output file.");
        return -1;
    }
    /*
     FIX: H.264 in some container format (FLV, MP4, MKV etc.) need
     "h264_mp4toannexb" bitstream filter (BSF)
     *Add SPS,PPS in front of IDR frame
     *Add start code ("0,0,0,1") in front of NALU
     H.264 in some container (MPEG2TS) don't need this BSF.
     */
//#if TEST_H264
//    AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");//aac_adtstoasc
//#endif
    
    /* auto detect the output format from the name. default is mpeg. */
    AVOutputFormat *fmt = av_guess_format(NULL, vPath, NULL);
    if (!fmt) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        fmt = av_guess_format("mpeg", NULL, NULL);
    }
    if (!fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        exit(1);
    }
    
    while(av_read_frame(ifmt_ctx, &pkt)>=0){
        if(pkt.stream_index==videoindex){
            
            
        }else if(pkt.stream_index==audioindex){
            
            /*封装前7个字节adts到每一个packet，可以随时解析；  adif只有文件开头有信息，不可以解析每一个packet*/
            if(fmt->audio_codec == AV_CODEC_ID_AAC){
                
                struct ADTSContext *ctx = (struct ADTSContext*)malloc(sizeof(struct ADTSContext));
                AVCodecParameters *codecpar = ifmt_ctx->streams[audioindex]->codecpar;
                int sample_rate_index = 0;
                switch (codecpar->sample_rate) {
                    case 96000:
                        sample_rate_index = 0;
                        break;
                    case 88200:
                        sample_rate_index = 1;
                        break;
                    case 64000:
                        sample_rate_index = 2;
                        break;
                    case 48000:
                        sample_rate_index = 3;
                        break;
                    case 44100:
                        sample_rate_index = 4;
                        break;
                    case 32000:
                        sample_rate_index = 5;
                        break;
                    case 24000:
                        sample_rate_index = 6;
                        break;
                    case 22050:
                        sample_rate_index = 7;
                        break;
                    case 16000:
                        sample_rate_index = 8;
                        break;
                    case 12000:
                        sample_rate_index = 9;
                        break;
                    case 11025:
                        sample_rate_index = 10;
                        break;
                    case 8000:
                        sample_rate_index = 11;
                        break;
                    case 7350:
                        sample_rate_index = 12;
                        break;
                        
                    default:
                        break;
                }
                ctx->sample_rate_index = sample_rate_index;///< samples per second
                ctx->channel_conf = codecpar->channels;///< number of audio channels
                ctx->objecttype = codecpar->profile;
                uint8_t buf[ADTS_HEADER_SIZE];
                ff_adts_write_frame_header(ctx,buf, pkt.size, 0);
                fwrite(buf,1,ADTS_HEADER_SIZE,fp_audio);
                free(ctx);
                
                
            }
            char buffer[80];
            sprintf(buffer, "Write Audio Packet. size:%d\t pts:%lld\n",pkt.size,pkt.pts);
            puts(buffer);
            logParam->str = buffer;
            callback_thread(logParam);
            printf("Write Audio Packet. size:%d\t pts:%lld\n",pkt.size,pkt.pts);
            fwrite(pkt.data,1,pkt.size,fp_audio);
        }
        av_packet_unref(&pkt);
    }
    
#if TEST_H264
    av_bitstream_filter_close(h264bsfc);
#endif
    
    fclose(fp_audio);
    
    avformat_close_input(&ifmt_ctx);
    
    if (ret < 0 && ret != AVERROR_EOF) {
        printf( "Error occurred.\n");
        return -1;
    }
    return 0;
}


