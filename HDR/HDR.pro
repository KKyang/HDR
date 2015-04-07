#-------------------------------------------------
#
# Project created by QtCreator 2015-03-20T13:34:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HDR
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    hdr.cpp \
    gsolve.cpp \
    exif.cpp \
    qsmartgraphicsview.cpp \

HEADERS  += mainwindow.h \
    hdr.h \
    gsolve.h \
    exif.h \
    qsmartgraphicsview.h \

FORMS    += mainwindow.ui

msvc {
  QMAKE_CXXFLAGS += -openmp -arch:AVX -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}

OpenCV_Libd = D:/libraries/opencv300b_o/x64/vc12/lib
OpenCV_Lib = D:/libraries/opencv300b_o/x64/vc12/lib
INCLUDEPATH += D:/libraries/opencv300b_o/include\
               $$PWD

LIBS += $$OpenCV_Lib/opencv_world300.lib\
        $$OpenCV_Lib/opencv_ts300.lib\


DEFINES += HAVE_OPENCV
