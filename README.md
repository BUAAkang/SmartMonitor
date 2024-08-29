# Smart Monitor
基于 QT + FFmpeg + OpenCV 开发的Windows GUI视频流播放工具
> 此工具仅为软件原型，请勿在生产环境下使用！

## 快速开始（下载源代码运行）

### 项目环境依赖
1. Editor: QT Creator 13.0.2
2. Core: QT 6.7.2
3. Build System: qmake
    ```bash
    # build demo
    C:/Qt/6.7.2/mingw_64/bin/qmake.exe C:\Users\Public\Downloads\qt_code\smart-monitor\smart-monitor.pro -spec win32-g++ "CONFIG+=debug" "CONFIG+=qml_debug" && C:/Qt/Tools/mingw1120_64/bin/mingw32-make.exe qmake_all
    ```
4. OpenCV: 4.10.0
    download: https://opencv.org/releases/
5. FFmpeg: 7.0.2
    download: https://www.gyan.dev/ffmpeg/builds/

### 运行步骤
1. 将该项目添加到QT Creator中
2. 编辑配置文件config.pri，将OPENCV_HOME和FFMPEG_HOME改为本地下载的全量编译库的路径
3. 在QT Creator中选择Debug构建选项并运行
4. 若出现一个新的名为SmartMonitor的窗口，则说明执行成功

## 作者
BUAAkang（康宇翔） lzy925（刘子渊）