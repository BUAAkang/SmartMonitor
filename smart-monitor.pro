include(config.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = SmartMonitor
TARGETNAME = SmartMonitor
RC_ICONS = smart-monitor.ico

SOURCES += \
    detect_thread.cpp \
    main.cpp \
    mainwindow.cpp \
    video_widget.cpp

HEADERS += \
    detect_thread.h \
    mainwindow.h \
    video_widget.h

FORMS += \
    mainwindow.ui

# language config
TRANSLATIONS += \
    smart-monitor_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# opencv
INCLUDEPATH += $$OPENCV_HOME\install\include
LIBS += $$OPENCV_HOME\lib\libopencv_*.a

# ffmpeg
INCLUDEPATH += $$FFMPEG_HOME/include
LIBS +=  -L$$FFMPEG_HOME/lib \
            -lavcodec \
            -lavdevice \
            -lavfilter \
            -lavformat \
            -lavutil \
            -lpostproc \
            -lswresample \
            -lswscale

RESOURCES += \
    detection_net.qrc
