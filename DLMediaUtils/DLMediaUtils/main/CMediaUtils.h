//
//  CMediaUtils.h
//  TubeD
//
//  Created by kaoji on 2018/8/22.
//  Copyright © 2018年 kaoji. All rights reserved.
//

#ifndef CMediaUtils_h
#define CMediaUtils_h

#include <stdio.h>

typedef void (*pcb)(char* str); //函数指针定义，后面可以直接使用pcb，方便
typedef struct parameter{
    char *str;
    pcb callback;
}parameter;


/**
 *  将视频轨道与音频轨道混合成影片
 *
 *  @param vPath     本地视频轨道
 *  @param aPath     音频轨道
 *  @param outPath 影片输出路径
 */
int MuxVideo(const char *vPath,const char *aPath,const char *outPath);

/**
 *  将音频轨道从MP4中分离出来 并打AAC头
 *
 *  @param vPath      本地MP4影片
 *  @param outPath  aac音频输出路径
 */
int DemuxAAC(const char *vPath,const char *outPath);


int SetCallBackFun(char *logText, pcb callback);
#endif /* CMediaUtils_h */
