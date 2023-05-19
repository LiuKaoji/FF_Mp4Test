# FF_Mp4Test
在iOS上利用ffmpeg进行mp4混流,分离aac,patch adts

![platform](https://img.shields.io/badge/platform-ios-lightgrey.svg)  

![preview](https://raw.githubusercontent.com/LiuKaoji/FF_Mp4Test/main/preview.gif)


## H264 + AAC = MP4
```mermaid
sequenceDiagram
    participant A as 打开输入文件
    participant B as 获取流信息
    participant C as 识别音频流
    participant D as 识别视频流
    participant E as 创建输出文件
    participant F as 初始化音频编码器
    participant G as 初始化视频编码器
    participant H as 写入文件头
    participant I as 循环读取音频数据
    participant J as 循环读取视频数据
    participant K as 添加ADTS头部
    participant L as 写入音频数据
    participant M as 写入视频数据
    participant N as 释放数据包资源
    participant O as 判断是否读完所有音频数据包
    participant P as 判断是否读完所有视频数据包
    participant Q as 写入文件尾
    participant R as 关闭输入/输出文件

    A->>B: avformat_open_input
    B->>C: avformat_find_stream_info (音频)
    B->>D: avformat_find_stream_info (视频)
    C->>E: avformat_alloc_output_context2
    D-->>E: avformat_alloc_output_context2
    E->>F: avcodec_find_encoder (音频)
    E->>G: avcodec_find_encoder (视频)
    F->>H: avformat_write_header
    I->>E: av_read_frame (音频)
    J->>E: av_read_frame (视频)
    I->>K: 添加ADTS头
    J->>L: 写入音频数据
    J->>M: 写入视频数据
    K->>L: 写入音频数据及其ADTS头
    L->>N: 写入音频数据
    M->>N: 写入视频数据
    N->>O: av_packet_unref (音频)
    N->>P: av_packet_unref (视频)
```
## MP4-> AAC流 + ADTS头 = AAC
```
MP4文件中的音频数据通常以AAC格式存储。为了正确解析和播放AAC音频数据，需要对其进行分离并添加ADTS头。
ADTS（Audio Data Transport Stream）头是AAC音频流的一种头部格式，其中包含关键的音频参数，如采样率、声道配置和帧长度。

分离MP4文件中的AAC音频并添加ADTS头的步骤如下：
1.打开输入文件。
2.获取流信息。
3.识别音频流。
4.创建输出文件。
5.循环读取音频数据包。
6.创建ADTS头并添加到音频数据包。
7.将带有ADTS头的音频数据写入输出文件。
8.释放数据包资源。
9.判断是否读取完所有数据包。
10.关闭输入/输出文件。
```
```mermaid  
sequenceDiagram
    participant A as 打开输入文件
    participant B as 获取流信息
    participant C as 识别音频流
    participant D as 创建输出文件
    participant E as 循环读取数据包
    participant F as 创建ADTS头并添加
    participant G as 写入音频数据
    participant H as 释放数据包资源
    participant I as 判断是否读完所有数据包
    participant J as 关闭输入/输出文件

    A->>B: avformat_open_input
    B->>C: avformat_find_stream_info
    C->>D: fopen (音频输出文件)
    D->>E: av_read_frame
    E->>F: 如果数据包属于音频流
    F->>G: 写入AAC数据及其ADTS头
    G->>H: 写入音频数据
    H->>I: av_packet_unref
    I->>E: 如果未读完所有数据包
    I->>J: 如果已读完所有数据包
    J-->>A: avformat_close_input, fclose
```
