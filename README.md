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
## MP4-> AAC流 + ADTS头 = AAC
```
MP4文件中的音频数据通常以AAC格式存储，并且在没有特殊处理的情况下，无法直接解析和播放。
为了正确地解析和播放AAC音频数据，需要对其进行分离并添加ADTS头。
ADTS（Audio Data Transport Stream）头是一种用于AAC音频流的头部格式。
它包含了关键的音频参数，例如采样率、声道配置和帧长度，这些信息对于解码和播放AAC音频数据非常重要。
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
