# FF_Mp4Test
在iOS上利用ffmpeg进行mp4混流,分离aac,patch adts


![platform](https://img.shields.io/badge/platform-ios-lightgrey.svg)  

![preview](https://raw.githubusercontent.com/LiuKaoji/FF_Mp4Test/main/preview.gif)

```
FFmpeg
|
|-> libavcodec: 提供编解码器、AVFrame、AVPacket等
|   |
|   |-> codec: 用于音视频的编解码
|   |-> AVFrame: 存储未压缩的音视频数据
|   |-> AVPacket: 存储压缩的音视频数据
|
|-> libavformat: 提供AVFormatContext、AVStream、AVIOContext等
|   |
|   |-> AVFormatContext: 管理多媒体文件的读写
|   |-> AVStream: 表示音视频文件中的一路流
|   |-> AVIOContext: 处理音视频数据的输入输出
|
|-> libavfilter: 提供AVFilter、AVFilterGraph等
|   |
|   |-> AVFilter: 实现音视频处理效果
|   |-> AVFilterGraph: 管理滤镜的连接和数据流转
|
|-> libavutil: 提供AVDictionary、AVOption、ImgUtils等
|   |
|   |-> AVDictionary: 存储键值对
|   |-> AVOption: 管理配置选项
|   |-> ImgUtils: 提供图像处理功能
|
|-> FFmpeg 命令: 用于多媒体操作的命令行工具
|
|-> 应用场景
    |
    |-> 解码音视频文件
    |   |
    |   |-> codec
    |   |-> AVFormatContext
    |   |-> AVStream
    |
    |-> 编码音视频文件
    |   |
    |   |-> codec
    |   |-> AVFrame
    |   |-> AVFormatContext
    |   |-> AVStream
    |
    |-> 转码音视频文件
    |   |
    |   |-> codec
    |   |-> AVFrame
    |   |-> AVPacket
    |   |-> AVFormatContext
    |   |-> AVStream
    |
    |-> 音视频流媒体传输
    |   |
    |   |-> AVIOContext
    |   |-> AVFormatContext
    |   |-> AVStream
    |
    |-> 音视频滤镜处理
        |
        |-> AVFilter
        |-> AVFilterGraph
        |-> AVFrame
```
