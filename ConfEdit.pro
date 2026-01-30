QT += widgets
CONFIG += c++17

TEMPLATE = app
TARGET = ConfEdit

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    confparser.cpp \
    translationstore.cpp \
    configmodel.cpp \
    editentrydialog.cpp

HEADERS += \
    mainwindow.h \
    confparser.h \
    translationstore.h \
    configmodel.h \
    editentrydialog.h
