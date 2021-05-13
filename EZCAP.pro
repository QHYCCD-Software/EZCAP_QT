##-------------------------------------------------
#
# Project created by QtCreator 2013-09-05T17:30:21
#
#-------------------------------------------------

QT       += core gui
win32{ QT += axcontainer }

TRANSLATIONS += language/lan_zh_cn.ts \
                language/lan_en_us.ts \
                language/lan_ja_jp.ts

RC_FILE   = myRc.rc

ICON = image/ezcap.icns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EZCAP
TEMPLATE = app

SOURCES += src/main.cpp \
    src/dllqhyccd.cpp \
    src/borderLayout.cpp \
    src/ezCap.cpp \
    src/cameraChooser.cpp \
    src/mainMenu.cpp \
    src/planner.cpp \
    src/about.cpp \
    src/favorite.cpp \
    src/fitHeader.cpp \
    src/tempControl.cpp \
    src/delegate.cpp \
    src/managementMenu.cpp \
    src/disktools.cpp \
    src/phdLink.cpp \
    src/darkFrameTool.cpp \
    src/downloadPreThread.cpp \
    src/downloadCapThread.cpp \
    src/downloadFocThread.cpp \
    src/cfwControl.cpp \
    src/cfwSetup.cpp \
    src/readmode.cpp

HEADERS  += \
    include/dllqhyccd.h \
    include/ezCap.h \
    include/borderLayout.h \
    include/cameraChooser.h \
    include/mainMenu.h \
    include/fitsio.h \
    include/longnam.h \
    include/tempControl.h \
    include/planner.h \
    include/fitHeader.h \
    include/favorite.h \
    include/about.h \
    include/delegate.h \
    #include/qhyccdStatus.h \
    include/managementMenu.h \
    include/disktools.h \
    #include/qhyccd.h \
    #include/qhyccdcamdef.h \
    #include/qhyccderr.h\
    include/qhyccdstruct.h \
    include/phdLink.h \
    #include/dithercontrol.h \
    include/darkframetool.h \
    include/downloadPreThread.h \
    include/downloadCapThread.h \
    include/downloadFocThread.h \
    include/myStruct.h \
    include/cfwControl.h \
    include/cfwSetup.h \
    include/readmode.h

FORMS    += \
    ui/ezCap.ui \
    ui/cameraChooser.ui \
    ui/tempControl.ui \
    ui/planner.ui \
    ui/fitHeader.ui \
    ui/favorite.ui \
    ui/about.ui \
    ui/managementMenu.ui \
    ui/phdLink.ui \
    ui/darkFrameTool.ui \
    ui/cfwControl.ui \
    ui/cfwSetup.ui \
    ui/readmode.ui

RESOURCES += \
    res.qrc

OTHER_FILES += \
    uac.manifest

INCLUDEPATH += include
unix:{
    INCLUDEPATH += /usr/local/include
#    INCLUDEPATH += /usr/local/Cellar/libusb/1.0.20/include

#For x86_64 Ubuntu
    LIBS += -L/usr/local/lib -lqhyccd
    LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_imgcodecs
    LIBS += -L/usr/lib/x86_64-linux-gnu -lusb-1.0 -lcfitsio
    
##For Arm32
#    LIBS += -L/usr/local/lib -lqhyccd
#    LIBS += -L/usr/lib/arm-linux-gnueabihf -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_imgcodecs
#    LIBS += -L/usr/lib/arm-linux-gnueabihf -lusb-1.0 -lcfitsio

#For Arm64
#    LIBS += -L/usr/local/lib -lqhyccd
#    LIBS += -L/usr/lib/aarch64-linux-gnu -lopencv_imgproc -lopencv_highgui -lopencv_core -lopencv_imgcodecs
#    LIBS += -L/usr/lib/aarch64-linux-gnu -lusb-1.0 -lcfitsio
}

win32: {
    contains(QT_ARCH, i386) {
        INCLUDEPATH += -I../EZCAP_Qt/include

        #LIBS += -L../EZCAP_Qt/winlib/x86 -lqhyccd
        LIBS += -L../EZCAP_Qt/winlib/x86 -lopencv_calib3d249    -lopencv_contrib249 -lopencv_core249
        LIBS += -L../EZCAP_Qt/winlib/x86 -lopencv_features2d249 -lopencv_flann249   -lopencv_gpu249
        LIBS += -L../EZCAP_Qt/winlib/x86 -lopencv_highgui249    -lopencv_imgproc249
        LIBS += -L../EZCAP_Qt/winlib/x86 -lcfitsio

        QMAKE_POST_LINK += echo @ECHO OFF >> config.bat &
        QMAKE_POST_LINK += echo SETLOCAL ENABLEDELAYEDEXPANSION >> config.bat &
        CONFIG(debug, debug|release) {
            QMAKE_POST_LINK += echo PATH=..\EZCAP_Qt\Depend\x86\*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\debug >> config.bat &
        }else{
            QMAKE_POST_LINK += echo PATH=..\EZCAP_Qt\Depend\x86\*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        }



    } else {
        INCLUDEPATH += -I../EZCAP_Qt/include

        #LIBS += -L../EZCAP_Qt/winlib/x64 -lqhyccd
        LIBS += -L../EZCAP_Qt/winlib/x64 -lopencv_calib3d249    -lopencv_contrib249 -lopencv_core249
        LIBS += -L../EZCAP_Qt/winlib/x64 -lopencv_features2d249 -lopencv_flann249   -lopencv_gpu249
        LIBS += -L../EZCAP_Qt/winlib/x64 -lopencv_highgui249    -lopencv_imgproc249
        LIBS += -L../EZCAP_Qt/winlib/x64 -lcfitsio

        QMAKE_POST_LINK += echo @ECHO OFF >> config.bat &
        QMAKE_POST_LINK += echo SETLOCAL ENABLEDELAYEDEXPANSION >> config.bat &
        CONFIG(debug, debug|release) {
            QMAKE_POST_LINK += echo PATH=..\EZCAP_Qt\Depend\x64\*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\debug >> config.bat &
        }else{
            QMAKE_POST_LINK += echo PATH=..\EZCAP_Qt\Depend\x64\*.dll >> config.bat &
            QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        }
    }

    CONFIG(release, debug|release) {
        QMAKE_POST_LINK += echo mkdir .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/plugins/platforms/qminimal.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/plugins/platforms/qoffscreen.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/plugins/platforms/qwindows.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release\platforms >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt5Core.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt5Gui.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/Qt5Widgets.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/libgcc_s_dw2-1.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/libstdc*.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
        QMAKE_POST_LINK += echo PATH=$$[QT_HOST_DATA]/bin/libwinpthread-1.dll >> config.bat &
        QMAKE_POST_LINK += echo copy %%PATH:/=\%% .\release >> config.bat &
    }
    QMAKE_POST_LINK += .\config.bat

}

release: {
    DEFINES += QT_NO_WARNING_OUTPUT\
    QT_NO_DEBUG_OUTPUT
#   CONFIG += console
}

