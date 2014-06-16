TEMPLATE = app
QT += xml network
CONFIG	+= qt
CONFIG	+= warn_on release

!macx {
	INCLUDEPATH	+= .. ./include /usr/include/opencv ../src/
} else {
	INCLUDEPATH	+= .. ./include /opt/local/include /opt/local/include/opencv ../src/
}

DEPENDPATH += ./include

HEADERS	+= include/ImageView.h \
           include/MainWindow.h \
           include/WebcamGrabber.h \
           include/FilterThread.h \
           include/CommandEditor.h \
           include/DialogAbout.h \
           include/ImageConverter.h \
           include/DialogLicence.h

SOURCES	+= src/WebcamGrabber.cpp \
           src/ImageView.cpp \
           src/MainWindow.cpp \
           src/ZArt.cpp \
           src/FilterThread.cpp \
           src/DialogAbout.cpp \
           src/CommandEditor.cpp \
           src/ImageConverter.cpp \
           src/DialogLicence.cpp

RESOURCES = zart.qrc
FORMS = ui/MainWindow.ui ui/DialogAbout.ui ui/DialogLicence.ui

exists( /usr/include/opencv2 ) {
 DEFINES += OPENCV2_HEADERS
}

system(pkg-config opencv --libs > /dev/null 2>&1) {
# LIBS += -lX11 ../src/libgmic.a `pkg-config opencv --libs` -lfftw3 -lfftw3_threads
 OPENCVLIBS = $$system(pkg-config opencv --libs)
 OPENCVLIBS = $$replace( OPENCVLIBS, -lcvaux, )
 LIBS += -lX11 ../src/libgmic.a $$OPENCVLIBS -lfftw3 -lfftw3_threads
} else {
  LIBS += -lX11 ../src/libgmic.a -lopencv_core -lopencv_highgui -lfftw3 -lfftw3_threads -lopencv_imgproc -lopencv_objdetect
# LIBS += -lX11 ../src/libgmic.a -lcxcore -lcv -lml -lhighgui -lfftw3 -lfftw3_threads
}

PRE_TARGETDEPS +=
QMAKE_CXXFLAGS_DEBUG += -Dcimg_use_fftw3
QMAKE_CXXFLAGS_RELEASE += -ffast-math -Dcimg_use_fftw3
UI_DIR = .ui
MOC_DIR = .moc
OBJECTS_DIR = .obj

unix:!macx {
	DEFINES += _IS_UNIX_
}

DEFINES += cimg_display=0

#QMAKE_LIBS =
#QMAKE_LFLAGS_DEBUG = -lcxcore -lcv -lhighgui -lml
#QMAKE_LFLAGS_RELEASE = -lcxcore -lcv -lhighgui -lml
