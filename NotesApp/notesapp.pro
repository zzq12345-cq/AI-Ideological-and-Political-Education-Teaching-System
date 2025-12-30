QT += core gui widgets webenginewidgets

CONFIG += c++17

TARGET = NotesApp
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    markdownhighlighter.cpp \
    note.cpp

HEADERS += \
    mainwindow.h \
    markdownhighlighter.h \
    note.h

# Install path
target.path = /Users/zhouzhiqi/QtProjects/AItechnology/NotesApp
INSTALLS += target
