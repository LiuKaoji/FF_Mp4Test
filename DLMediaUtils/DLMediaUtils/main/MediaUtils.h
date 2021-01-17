//
//  MediaUtils.h
//  DLMediaUtils
//
//  Created by kaoji on 2020/10/15.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^MediaUtilsBlock)(NSInteger code);

@interface MediaUtils : NSObject

/**
 *  将视频轨道与音频轨道混合成影片
 *
 *  @param vPath     本地视频轨道
 *  @param aPath     音频轨道
 *  @param outPath 影片输出路径
 */
+(void)muxMp4WithVideo:(NSString *)vPath Audio:(NSString *)aPath to:(NSString *)outPath Complete:(MediaUtilsBlock)complete;

/**
 *  将音频轨道从MP4中分离出来 并打AAC头
 *
 *  @param vPath      本地MP4影片
 *  @param outPath  aac音频输出路径
 */
+(void)demuxAACFromMp4:(NSString *)vPath to:(NSString *)outPath Complete:(MediaUtilsBlock)complete;

+(void)runLogThread;

@end

NS_ASSUME_NONNULL_END
