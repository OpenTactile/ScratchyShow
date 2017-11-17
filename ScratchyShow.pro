TEMPLATE = app
QT += core gui widgets
CONFIG += c++14
INCLUDEPATH += /usr/include/eigen3
#INCLUDEPATH += /usr/include/python3.6m/

TARGET = ScratchyShow

SOURCES += main.cpp \
    util/hardwareabstraction.cpp \
    view/displaybreakout.cpp \
    view/hdmiscreen.cpp \
    view/graphicalobjects.cpp \
    model/scene.cpp \
    model/tactiledisplay.cpp \    
    model/nullmodel.cpp \
    model/frequencymodel.cpp \    
    util/lookuptable.cpp \
    util/filelogger.cpp

FORMS += \
    ui/xlessgui.ui \
    ui/infobox.ui

LIBS += -lusb -lSCRATCHy -lITCHy -latomic -lpthread

DEFINES += "MAP_DIRECTORY=\\\"tactileScenes/\\\""

HEADERS += \
    util/hardwareabstraction.h \
    view/view.h \
    view/displaybreakout.h \
    view/hdmiscreen.h \
    view/graphicalobjects.h \
    model/scene.h \
    model/tactiledisplay.h \
    model/actuator.h \
    model/model.h \
    model/nullmodel.h \
    model/frequencymodel.h \    
    util/lookuptable.h \
    util/filelogger.h \
    model/entity.h \
    view/viewentity.h

fake {
    DEFINES += FAKEMODE    
    QMAKE_CXXFLAGS_RELEASE += -Og -g
}

!fake {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

RESOURCES += \
    graphics.qrc

unix {
    target.path = /usr/bin/
    INSTALLS += target
}

