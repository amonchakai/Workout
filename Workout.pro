APP_NAME = Workout

CONFIG += qt warn_on cascades10
LIBS += -lbbsystem -lbbdevice -lbbcascadespickers -lz -lbbdata -lbbplatform  -lbb 
QT += network

INCLUDEPATH += ../../Libraries/

include(config.pri)
