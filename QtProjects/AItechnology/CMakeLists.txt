cmake_minimum_required(VERSION 3.16)

project(AILoginSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    src/main/main.cpp
    src/main/identicalloginwindow.cpp
    src/main/identicalloginwindow.h
    src/ui/simpleloginwindow.cpp
    src/ui/simpleloginwindow.h
    src/ui/simpleloginwindow.ui
    src/ui/simpleloginwindow_designer.ui
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/modernmainwindow.ui
    src/ui/modernmainwindow.cpp
    src/ui/modernmainwindow.h
)

qt_add_executable(AILoginSystem
    ${PROJECT_SOURCES}
)

qt_add_resources(AILoginSystem "resources"
    PREFIX "/"
    FILES
        resources/fonts/Lexend-Regular.ttf
        resources/stylesheet.qss
)

target_link_libraries(AILoginSystem PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 设置目标属性
set_target_properties(AILoginSystem PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.aiedu.loginsystem
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装规则
include(GNUInstallDirs)
install(TARGETS AILoginSystem
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)