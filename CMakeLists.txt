cmake_minimum_required(VERSION 3.16)

project(AILoginSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network QuickWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Charts QuickWidgets)

set(PROJECT_SOURCES
    src/main/main.cpp
    src/auth/login/simpleloginwindow.cpp
    src/auth/login/simpleloginwindow.h
    src/auth/signup/signupwindow.cpp
    src/auth/signup/signupwindow.h
    src/auth/supabase/supabaseclient.cpp
    src/auth/supabase/supabaseclient.h
    src/auth/supabase/supabaseconfig.cpp
    src/auth/supabase/supabaseconfig.h
    src/dashboard/modernmainwindow.cpp
    src/dashboard/modernmainwindow.h
    src/questionbank/questionbankwindow.cpp
    src/questionbank/questionbankwindow.h
    src/questionbank/QuestionRepository.cpp
    src/questionbank/QuestionRepository.h
    src/ui/aipreparationwidget.cpp
    src/ui/aipreparationwidget.h
    src/ui/moderncheckbox.cpp
    src/ui/moderncheckbox.h
    src/services/ExportService.cpp
    src/services/ExportService.h
    src/services/DifyService.cpp
    src/services/DifyService.h
    src/ui/ChatWidget.cpp
    src/ui/ChatWidget.h
    src/ui/AIChatDialog.cpp
    src/ui/AIChatDialog.h
    resources.qrc
    resources/QtTheme/QtTheme.qrc
)

qt_add_executable(AILoginSystem
    ${PROJECT_SOURCES}
)

target_link_libraries(AILoginSystem PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::QuickWidgets
)

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
