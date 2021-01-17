//
//  MediaUtils.m
//  DLMediaUtils
//
//  Created by kaoji on 2020/10/15.
//

#import  "MediaUtils.h"
#include "CMediaUtils.h"
#include <stdio.h>

@implementation MediaUtils

+(void)muxMp4WithVideo:(NSString *)vPath Audio:(NSString *)aPath to:(NSString *)outPath Complete:(MediaUtilsBlock)complete{

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
       
        int ret = MuxVideo([vPath UTF8String],[aPath UTF8String],[outPath UTF8String]);
        dispatch_async(dispatch_get_main_queue(), ^{
            if(complete)complete(ret);
            
        });
    });
}

+(void)demuxAACFromMp4:(NSString *)vPath to:(NSString *)outPath Complete:(MediaUtilsBlock)complete{
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        int ret = DemuxAAC([vPath UTF8String],[outPath UTF8String]);

        dispatch_async(dispatch_get_main_queue(), ^{
            if(complete)complete(ret);
        });
    });
}

+(void)runLogThread{
    SetCallBackFun("", fCallBack);
}


void fCallBack(char *str)
{
    //do something
//    printf("a = %s\n",str);
//    printf("fCallBack print! \n");
    NSString *logStr = [NSString stringWithUTF8String:str];
    [[NSNotificationCenter defaultCenter]postNotificationName:@"LOG_THREAD" object:logStr];
}


@end
