

@[TOC](目录)

# 准备

## 编译环境
	 ubuntu 16.04.3

## libpjproject 版本
	2.8
	https://www.pjsip.org/release/2.8/pjproject-2.8.tar.bz2
	

## 编译工具
必备
 - gcc
 - make
 - autoconf

可选：
 - wget
 - git


# 依赖库

## video4linux2
这个是 linux 视频设备的驱动，编译 pjsua 时 会有依赖。
apt 安装即可。
sudo apt-get install libv4l-dev

## SDL2
pjsua 本身只是协议栈，并不具备 音视频的显示播放功能，但它使用 SDL2 完成这样。
SDL2 涉及视频和音频的功能，所以编译 SDL2 的时候，要加上对应的 driver 和 codec。
需要注意的是， 没有这些，也不影响 SDL2 的编译，只不过编出来的库不支持这些 driver 和 codec，比如，用 pjsua2 preview video 时会报找不到对应的视频驱动、make Call 时会报找不到音频播放器。

 - 音频驱动 pulse
	```shell
	sudo apt-get install libasound-dev libpulse-dev
	```
 - 视频驱动  x11、opengl
	```shell
	sudo apt-get install libx11-dev libxext-dev libgl-dev
	```

以上组件安装好了以后，就可以编译源码了。
### SDL2 编译
```shell
	wget http://www.libsdl.org/release/SDL2-2.0.7.tar.gz 
	tar -zxf SDL2-2.0.7.tar.gz 
	cd SDL2-2.0.7
	./autogen.sh
	./configure  --enable-shared=no
```
静态编译，所以带了参数 --enable-shared=no。
configure 完成以后，会有 build  summary，留意一下：
```
SDL2 Configure Summary:
Building Static Libraries
Enabled modules : atomic audio video render events joystick haptic power filesystem threads timers file loadso cpuinfo assembly
Assembly Math   : mmx 3dnow sse sse2 sse3
Audio drivers   : disk dummy oss alsa(dynamic) pulse(dynamic)
Video drivers   : dummy x11(dynamic) opengl opengl_es2 vulkan
X11 libraries   : xdbe xshape xvidmode
Input drivers   : linuxev linuxkd
Using libsamplerate : NO
Using libudev       : NO
Using dbus          : NO
Using ime           : YES
Using ibus          : NO
Using fcitx         : NO
```
可以看到，上面安装的 视频驱动 x11、opengl、音频驱动 alsa、pulse 都显示出来了。

继续编译。
```shell
make
sudo make install
```
至此，sdl2 编译完成。

### SDL2  测试
sdltest.cpp:
```c
#include <SDL2/SDL.h>
#include <iostream>

using namespace std;


int main () {
	// 初始化
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        cout<<"Unable to init SDL: " << SDL_GetError()<<endl;
        return 1;
    }
    
    // 打印视频驱动
    int sdldrinumb = SDL_GetNumVideoDrivers();
    cout<<"sdl video driver number: " << sdldrinumb<<endl;
    for (int i = 0; i < sdldrinumb; i++) {
       cout<<"sdl video driver " << i << " name: "<<  SDL_GetVideoDriver(i) << endl;
    }
  
  	// 打印视频渲染驱动
    int sdlRender = SDL_GetNumRenderDrivers();
    cout<<"sdl render driver numer: " << sdldrinumb<<endl;
    for (int i = 0; i < sdldrinumb; i++) {
        SDL_RendererInfo ri;
        int ret =  SDL_GetRenderDriverInfo(i, &ri);
        if (ret == -1) {
            cout<<"get sdl render info failed:  " << i << ": "<<  ret << endl;
        } else {
            cout<<"sdl render  " << i << " name: "<<  ri.name << ", flag:"<< ri.flags <<endl;
        }
    }
    
    // 打印音频驱动
    int sdlAudioDriverNumber = SDL_GetNumAudioDrivers();
    cout<<"sdl audio driver number: " << sdlAudioDriverNumber<<endl;
    for (int i = 0; i < sdlAudioDriverNumber; i++) {
       cout<<"sdl audio driver " << i << " name: "<<  SDL_GetAudioDriver(i) << endl;
    }
    
    // 打印当前音频设备
    int audioPlayDeviceNumber = SDL_GetNumAudioDevices(0);
    cout<<"sdl audio play device number: " << audioPlayDeviceNumber<<endl;
    for (int i = 0; i < audioPlayDeviceNumber; i++) {
       cout<<"sdl audio play " << i << " name: "<<  SDL_GetAudioDeviceName(i, 0) << endl;
    }
    
    int audioCaptureDeviceNumber = SDL_GetNumAudioDevices(1);
    cout<<"sdl audio capture device number: " << audioCaptureDeviceNumber<<endl;
    for (int i = 0; i < audioCaptureDeviceNumber; i++) {
       cout<<"sdl audio capture " << i << " name: "<<  SDL_GetAudioDeviceName(i, 1) << endl;
    }
    cout<<"end "<<endl;
}
```

编译
```shell
g++ -o sdltest sdltest.cpp `sdl2-config --cflags --libs`
```
代码看起来虽然多，但其实很简单，初始化 SDL 后，打印驱动信息：
```
sdl video driver number: 2
sdl video driver 0 name: x11
sdl video driver 1 name: dummy

sdl render driver numer: 2
sdl render  0 name: opengl, flag:14
sdl render  1 name: opengles2, flag:14

sdl audio driver number: 5
sdl audio driver 0 name: pulseaudio
sdl audio driver 1 name: alsa
sdl audio driver 2 name: dsp
sdl audio driver 3 name: disk
sdl audio driver 4 name: dummy

sdl audio play device number: -1
sdl audio capture device number: -1
end
```

从这个结果应该是要可以看到我们之前安装的那些依赖。
如果没有标红的那几项，那接下来 pjsua 在使用的时候，会应该找不到对应的驱动而出错。

SDL2 编译完成。


## openh264
openh264 是思科的 h264 编码的开源实现。
使用了汇编加速，所以需要安装 nasm。

### nasm
ubuntu apt 安装即可
```shell
sudo apt-get install nasm
```
### 编译
```shell
git clone https://github.com/cisco/openh264.git
cd openh264
make libraries
sudo make install-static
```
## opus
开源音频编解码器。
```shell
wget https://archive.mozilla.org/pub/opus/opus-1.3.tar.gz
tar -xzf opus-1.3.tar.gz
cd opus-1.3
./configure --enable-shared=no
make 
sudo make install
```

## openssl
如果使用 libpjproject 进行网络通话是，需要加 tls，则需要 openssl
```shell
wget https://www.openssl.org/source/old/1.0.1/openssl-1.0.1u.tar.gz  
tar -zxf openssl-1.0.1u.tar.gz
cd openssl-1.0.1u
./config --enable-shared=no
make
make install
```
--------------------------
至此，所有依赖编译安装完成


# libpjproject 
pjsua2 其实是 pjsip 的 C++的接口。

先下载源码：
```shell
wget https://www.pjsip.org/release/2.8/pjproject-2.8.tar.bz2
tar -xjf pjproject-2.8.tar.bz2
cd pjproject-2.8
```
如果要支持视频 
```shell
cp pjlib/include/pj/config_site_sample.h pjlib/include/pj/config_site.h
echo "#define PJMEDIA_HAS_VIDEO 1" >> pjlib/include/pj/config_site.h
```
预编译:
```shell
./configure --disable-shared --disable-ffmpeg
```
如果编译了 openssl 的话， 加上 --with-ssl=[ssl path] 参数
```shell
./configure --with-ssl=/usr/local/ssl --disable-shared --disable-ffmpeg
```

继续编译：
```shell
make dep 
make
sudo make install
```
----
至此 libpjproject 静态库编译完成。

# 测试
 pjsua-video-test.cpp
```c
#include <pjsua2.hpp>
#include <SDL2/SDL.h>
#include <iostream>

using namespace pj;

// 子类 PjsuaCall，主要用来连接音频设备
class PjsuaCall : public Call {
public:
    PjsuaCall(Account &acc, int call_id = PJSUA_INVALID_ID)
    : Call(acc, call_id){
    }
    
    virtual void onCallMediaState(OnCallMediaStateParam &prm) {
        std::cout << "call media state change"<<std::endl;
        CallInfo ci = getInfo();
        for (unsigned i = 0; i < ci.media.size(); i++) {
            std::cout << "call media "<< i << " , type " << ci.media[i].type <<std::endl;
            if (ci.media[i].type == PJMEDIA_TYPE_AUDIO && getMedia(i)) {
                // 连接音频
                pj::AudioMedia *aud_med = (pj::AudioMedia *)getMedia(i);
                pj::AudDevManager &mgr = pj::Endpoint::instance().audDevManager();

                aud_med->startTransmit(mgr.getPlaybackDevMedia());
                mgr.getCaptureDevMedia().startTransmit(*aud_med);
            } 
        }
    }
};

// 测试-预览本机视频
bool testVideoPreview(Endpoint &ep) {
	pj_status_t status = pjsua_vid_preview_start(PJMEDIA_VID_DEFAULT_CAPTURE_DEV, NULL);
    if (status != PJ_SUCCESS) {
        std::cout <<"preview local video failed! "<< ep.utilStrError(status)<<std::endl;
        return false;
    }
    return true;
}

// 测试-视频通话
bool testMakeCall(Endpoint &ep, TransportId &tid) {
    // create account
    std::string localAddr = ep.transportGetInfo(tid).localName;
    std::string myAccoutID = "myname<sip:myid@" + localAddr +">";
    std::cout << "myAccoutID:" << myAccoutID;
    
    AccountConfig acfg;
    acfg.idUri = myAccoutID;
    acfg.videoConfig.defaultCaptureDevice = PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
    acfg.videoConfig.defaultRenderDevice = PJMEDIA_VID_DEFAULT_RENDER_DEV;
    acfg.videoConfig.autoShowIncoming = PJ_TRUE;
    acfg.videoConfig.autoTransmitOutgoing = PJ_TRUE;
    //// 如果配置了 turn server 
    //acfg.natConfig.iceEnabled = true;
    //acfg.natConfig.turnEnabled = true;
    //acfg.natConfig.turnServer = "turn server ip:port";
    //// turn server 一般是 udp
    //acfg.natConfig.turnConnType = PJ_TURN_TP_UDP;
    
    Account myAcc;
    try {
        myAcc.create(acfg, true);
    } catch (pj::Error &error) {
        std::cout << "create account failed:" << error.info() << std::endl;
        return false;
    }
    
    // make call
    std::string remoteid = "sip:roomid@sip server:port;transport=tcp";
    //// rtcp 使用 tcp
    //std::string remoteid = "sip:roomid@sip server:port;transport=tcp";
    std::cout << "remoteid:"<< remoteid << std::endl;
    
    PjsuaCall cl(myAcc);
    CallOpParam prm(true);
    prm.opt.videoCount = 1;
    
    try {
        cl.makeCall(remoteid, prm);
    } catch (pj::Error &error) {
        std::cout << "make call failed:" << error.info() << std::endl;
        return false;
    }
    return true;
}

// 测试-打印 libpjproject 支持的库
bool testDevices(Endpoint &ep) {
    std::cout << "********* video device info ************" << std::endl;;
    pjmedia_vid_dev_info viddevs[100] = {0};
    unsigned devcount = 100;
    
    pj_status_t ret = pjsua_vid_enum_devs(viddevs, &devcount);
    if (ret != PJ_SUCCESS) {
        std::cout << "enum video device failed: " << ep.utilStrError(ret) << std::endl;
    } else {
        std::cout << "video device count: " << devcount << std::endl;
        for (unsigned i = 0; i < devcount; i++) {
            pjmedia_vid_dev_info info = viddevs[i];
            
            std::cout << "video index: " << i << std::endl;
            std::cout << "video id: " << info.id << std::endl;
            std::cout << "video name: " << info.driver << std::endl;
            std::cout << "video caps: " << info.caps << std::endl;
            std::cout << "video fmt_cnt: " << info.fmt_cnt << std::endl;
            std::cout << "----------------" << std::endl;
        }
    }


    std::cout << "********** video codec info ************" << std::endl;
    pjsua_codec_info videoCodecs[100] = {0};
    unsigned videoCount = 100;
    
    ret = pjsua_vid_enum_codecs(videoCodecs, &videoCount);
    if (ret != PJ_SUCCESS) {
        std::cout << "enum video codecs failed: " << ep.utilStrError(ret) << std::endl;
    } else {
        std::cout << "video codecs count: " << videoCount << std::endl;
        for (unsigned i = 0; i < videoCount; i++) {
            pjsua_codec_info ci = videoCodecs[i];
            std::string idString(ci.codec_id.ptr, ci.codec_id.slen);
            std::string decString(ci.desc.ptr, ci.desc.slen);

            std::cout << "video codec index: " << i << std::endl;
            std::cout << "video codec id: " << idString << std::endl;
            std::cout << "video codec description: " << decString << std::endl;
            std::cout << "----------------" << std::endl;
        }
    }


    std::cout << "********** audio device info ************" << std::endl;
    pjmedia_aud_dev_info audioDevices[100] = {0};
    unsigned audioDeviceCount = 100;
    
    ret = pjsua_enum_aud_devs(audioDevices, &audioDeviceCount);
    if (ret != PJ_SUCCESS) {
        std::cout << "enum audio device failed: " << ep.utilStrError(ret) << std::endl;
    } else {
        std::cout << "audio device count: " << audioDeviceCount << std::endl;
        for (unsigned i = 0; i < audioDeviceCount; i++) {
            pjmedia_aud_dev_info ai = audioDevices[i];

            std::string nameString(ai.name);
            std::string driverString(ai.driver);

            std::cout << "video device index: " << i << std::endl;
            std::cout << "video device name: " << nameString << std::endl;
            std::cout << "video device driver: " << driverString << std::endl;
            std::cout << "----------------" << std::endl;
        }
    }


    std::cout << "********** audio codec info ************" << std::endl;
    pjsua_codec_info audioCodecs[100] = { 0 };
    unsigned audioCount = 100;
    
    ret = pjsua_enum_codecs(audioCodecs, &audioCount);
    if (ret != PJ_SUCCESS) {
        std::cout << "enum audio codecs failed: " << ep.utilStrError(ret) << std::endl;
    } else {
        std::cout << "audio codecs count: " << audioCount << std::endl;
        for (unsigned i = 0; i < audioCount; i++) {
            pjsua_codec_info ci = audioCodecs[i];
            std::string idString(ci.codec_id.ptr, ci.codec_id.slen);
            std::string decString(ci.desc.ptr, ci.desc.slen);
            std::cout << "audio codec index: " << i << std::endl;
            std::cout << "audio codec id: " << idString << std::endl;
            std::cout << "audio codec description: " << decString << std::endl;
            std::cout << "----------------" << std::endl;
        }
    }
    
    std::cout << "*********************************" << std::endl;
    return true;
}

int main() {
	// 初始化 SDL 库
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        std::cout << SDL_GetError() << std::endl;
        return 1;
    }
    
    Endpoint ep;
    ep.libCreate();
    
    // 初始化 Endpoint
    EpConfig ep_conf;
    ep.libInit(ep_conf);
    
    // 初始化 sip  Transport，简单处理 错误
    TransportId tid;
    TransportConfig tcfg;
    // sip server 默认端口 5060
    tcfg.port = 5060;
    try  {
        // rtcp 使用 udp
        tid = ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
        //// rtcp 使用 tcp
        //tid = ep.transportCreate(PJSIP_TRANSPORT_TCP, tcfg);
    }   catch (Error &error)   {
        std::cout << error.info() << std::endl;
        return 1;
    }
    // 启动 pjsua lib 库( 轮询 pjsip 的工作线程等)
    ep.libStart();
    std::cout << "***  pjsua2 started ***" << std::endl;

	// libpjproject 版本
    std::cout << "veision: " << pj::Endpoint::instance().libVersion().full << std::endl;
    bool ret = true;
    
    // 设备测试 
    ret = testDevices(ep);
    /////////////////////////////////////////////////////////////////////

    //// 预览测试 10 秒
    ret = testVideoPreview(ep);
    pj_thread_sleep(1000 * 10);
    /////////////////////////////////////////////////////////////////////

    //// 通话测试 1 分钟
    //ret = testMakeCall(ep, tid);
    //pj_thread_slee1000 * 60));
    //////////////////////////////////////////////////////////////////

    if (ret) {
        std::cout << "***  pjsua2 succeed ***" << std::endl;
    } else {
        std::cout << "***  pjsua2 failed ***" << std::endl;
    }        
    return 0;
}
```

编译：
```shell
g++ pjsua-video-test.cpp `pkg-config --cflags --libs libpjproject`
```

打印信息如下：
```
********* video device info ************
video device count: 3
video index: 0
video id: 0
video name: SDL
video caps: 525
video fmt_cnt: 11
----------------
video index: 1
video id: 1
video name: Colorbar
video caps: 1
video fmt_cnt: 11
----------------
video index: 2
video id: 2
video name: Colorbar
video caps: 1
video fmt_cnt: 11
----------------
********** video codec info ************
video codecs count: 1
video codec index: 0
video codec id: H264/97
video codec description: OpenH264 codec
----------------
********** audio device info ************
audio device count: 15
video device index: 0
video device name: default
video device driver: ALSA
----------------
video device index: 1
video device name: pulse
video device driver: ALSA
----------------
video device index: 2
video device name: sysdefault:CARD=AudioPCI
video device driver: ALSA
----------------
video device index: 3
video device name: front:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 4
video device name: rear:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 5
video device name: surround40:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 6
video device name: iec958:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 7
video device name: dmix:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 8
video device name: dmix:CARD=AudioPCI,DEV=1
video device driver: ALSA
----------------
video device index: 9
video device name: dsnoop:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 10
video device name: dsnoop:CARD=AudioPCI,DEV=1
video device driver: ALSA
----------------
video device index: 11
video device name: hw:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 12
video device name: hw:CARD=AudioPCI,DEV=1
video device driver: ALSA
----------------
video device index: 13
video device name: plughw:CARD=AudioPCI,DEV=0
video device driver: ALSA
----------------
video device index: 14
video device name: plughw:CARD=AudioPCI,DEV=1
video device driver: ALSA
----------------
********** audio codec info ************
audio codecs count: 11
audio codec index: 0
audio codec id: speex/16000/1
audio codec description: 
----------------
audio codec index: 1
audio codec id: speex/8000/1
audio codec description: 
----------------
audio codec index: 2
audio codec id: speex/32000/1
audio codec description: 
----------------
audio codec index: 3
audio codec id: iLBC/8000/1
audio codec description: 
----------------
audio codec index: 4
audio codec id: GSM/8000/1
audio codec description: 
----------------
audio codec index: 5
audio codec id: PCMU/8000/1
audio codec description: 
----------------
audio codec index: 6
audio codec id: PCMA/8000/1
audio codec description: 
----------------
audio codec index: 7
audio codec id: G722/16000/1
audio codec description: 
----------------
audio codec index: 8
audio codec id: opus/48000/2
audio codec description: 
----------------
audio codec index: 9
audio codec id: L16/44100/2
audio codec description: 
----------------
audio codec index: 10
audio codec id: L16/44100/1
audio codec description: 
----------------
```

不同平台可能有所不同，但一定会有 SDL、H264、pulse、opus 这四个。
其他影响不大。

至此，支持视频的 libpjproject 的静态库编译完成！

