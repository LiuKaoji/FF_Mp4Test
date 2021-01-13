//
//  ViewController.m
//  Mp4Test
//
//  Created by kaoji on 2021/1/12.
//

#import "ViewController.h"
#import <DLMediaUtils/DLMediaUtils.h>
#import <AVKit/AVKit.h>

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UITextView *outputLog;
@property (copy, nonatomic) NSString *muxVideoPath;
@property (weak, nonatomic) IBOutlet UITextView *textView;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    ///监听buffer输出
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(recievedLog:) name:@"LOG_THREAD" object:nil];
    ///关闭log动画
    _textView.layoutManager.allowsNonContiguousLayout = NO;
}

#pragma mark - mux
- (IBAction)muxAction:(id)sender {
    
    NSString *videoStream = [[NSBundle mainBundle] pathForResource:@"Apple" ofType:@"mp4"];//无声视频
    NSString *audioStream = [[NSBundle mainBundle] pathForResource:@"Apple" ofType:@"aac"];//声音
    NSString *outputPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"Apple_Mux.mp4"];//有声视频
    /// 将无声mp4与aac音轨合并组成有声视频
    /// 比如youtube很多音视频就是需要分轨下载 需要对视频和音频合并才得到完整文件
    [MediaUtils muxMp4WithVideo:videoStream Audio:audioStream to:outputPath Complete:^(NSInteger code) {
       
        if (code != 0) {
            [self insertLog:@"文件合并失败"];
            return;
        }
        self.muxVideoPath = outputPath;
        [self playWithPath:outputPath];
        [self insertLog:[NSString stringWithFormat:@"\n视频合并成功%@\n",outputPath]];
    }];
}

#pragma mark - demux
- (IBAction)demuxAction:(id)sender {
    
    if(_muxVideoPath == nil){
        [self insertLog:@"请先合并"];
        return;
    }
    
    NSString *outputPath = [NSTemporaryDirectory() stringByAppendingPathComponent:@"Apple_demux.aac"];
    
    /// 将音频分离 并在AAC加adts头
    /// https://blog.csdn.net/tantion/article/details/82743942
    [MediaUtils demuxAACFromMp4:self.muxVideoPath to:outputPath Complete:^(NSInteger code) {
        
        if (code != 0) {
            [self insertLog:@"音频分离失败"];
            return;
        }
        [self playWithPath:outputPath];
        [self insertLog:[NSString stringWithFormat:@"\n音频导出成功%@\n",outputPath]];
    }];
}

#pragma mark - reset
- (IBAction)resetAction:(id)sender {
    
    _muxVideoPath = nil;
    self.outputLog.text = @"Mp4Test:";
}


#pragma mark - preview
-(void)playWithPath:(NSString *)path{
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        AVPlayerViewController *playerVC = [[AVPlayerViewController alloc] init];
        playerVC.modalPresentationStyle = UIModalPresentationFullScreen;
        playerVC.player= [[AVPlayer alloc]initWithURL:[NSURL fileURLWithPath:path]];
        playerVC.allowsPictureInPicturePlayback = YES;
        [[playerVC player] play];
        [self presentViewController:playerVC animated:YES completion:nil];
    });
}

#pragma mark - log
-(void)insertLog:(NSString *)text{
    
    [self.outputLog insertText:[NSString stringWithFormat:@"\n%@",text]];

    [_textView scrollRangeToVisible:NSMakeRange(_textView.text.length - 1, 1)];
}

-(void)recievedLog:(NSNotification *)notify{
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.outputLog insertText:[NSString stringWithFormat:@"%@",notify.object]];
    });
    
}

@end
