QT += core
QT -= gui
CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = coreaudio_channel_map
INCLUDEPATH += ../../app

SOURCES += main.cpp ../../app/streaming/audio/renderers/coreaudiochannelmap.cpp
HEADERS += ../../app/streaming/audio/renderers/coreaudiochannelmap.h
