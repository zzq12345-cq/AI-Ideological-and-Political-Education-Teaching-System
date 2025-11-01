QT += core widgets network
CONFIG += c++17

TARGET = AIPoliticsClassroom
TEMPLATE = app

# 定义
DEFINES += QT_DEPRECATED_WARNINGS

# 源文件
SOURCES += \
    src/main/main.cpp \
    src/auth/login/simpleloginwindow.cpp \
    src/dashboard/modernmainwindow.cpp

# 头文件
HEADERS += \
    src/auth/login/simpleloginwindow.h \
    src/dashboard/modernmainwindow.h

# UI文件
FORMS += \
    src/auth/login/simpleloginwindow.ui \
    src/dashboard/modernmainwindow.ui

# 暂时移除资源文件依赖
# RESOURCES += resources.qrc

# 默认规则用于部署
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# macOS设置
macx {
    QMAKE_INFO_PLIST = Info.plist
    ICON = app_icon.icns
}

# Windows设置
win32 {
    RC_ICONS = app_icon.ico
}

# 设置输出目录
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/debug
} else {
    DESTDIR = $$PWD/build/release
}

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui

RESOURCES += \
    resources.qrc
