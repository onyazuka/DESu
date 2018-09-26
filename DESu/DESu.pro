TEMPLATE = app
CONFIG += console c++11 qml_debug
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += \
    stdafx.h \
    des.h \
    desfilecrypt.h \
    destechtools.h \
    Multithread/ThreadsafeQueue.h \
    Multithread/ThreadPoolMy.h
SOURCES += \
    stdafx.cpp \
    main.cpp \
    des.cpp \
    desfilecrypt.cpp \
    destechtools.cpp \
    Multithread/ThreadPoolMy.cpp

QMAKE_CXXFLAGS += -std=c++0x -pthread
QMAKE_CFLAGS += -std=gnu++0x -pthread
LIBS += -pthread
