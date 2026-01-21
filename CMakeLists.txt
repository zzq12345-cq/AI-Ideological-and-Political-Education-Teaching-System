cmake_minimum_required(VERSION 3.16)

project(AILoginSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network QuickWidgets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Charts QuickWidgets Svg)

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
    src/questionbank/QuestionBasket.cpp
    src/questionbank/QuestionBasket.h
    src/questionbank/QuestionBasketWidget.cpp
    src/questionbank/QuestionBasketWidget.h
    src/questionbank/PaperComposerDialog.cpp
    src/questionbank/PaperComposerDialog.h
    src/ui/aipreparationwidget.cpp
    src/ui/aipreparationwidget.h
    src/ui/moderncheckbox.cpp
    src/ui/moderncheckbox.h
    src/services/ExportService.cpp
    src/services/ExportService.h
    src/services/DocxGenerator.cpp
    src/services/DocxGenerator.h
    src/services/DifyService.cpp
    src/services/DifyService.h
    src/services/PPTXGenerator.cpp
    src/services/PPTXGenerator.h
    src/services/PaperService.cpp
    src/services/PaperService.h
    src/ui/ChatWidget.cpp
    src/ui/ChatWidget.h
    src/ui/AIChatDialog.cpp
    src/ui/AIChatDialog.h
    src/utils/MarkdownRenderer.cpp
    src/utils/MarkdownRenderer.h
    src/ui/ChatHistoryWidget.cpp
    src/ui/ChatHistoryWidget.h
    src/ui/NetworkImageTextBrowser.cpp
    src/ui/NetworkImageTextBrowser.h
    src/ui/HotspotTrackingWidget.cpp
    src/ui/HotspotTrackingWidget.h
    src/services/HotspotService.cpp
    src/services/HotspotService.h
    src/hotspot/INewsProvider.h
    src/hotspot/NewsItem.h
    src/hotspot/MockNewsProvider.cpp
    src/hotspot/MockNewsProvider.h
    src/hotspot/RealNewsProvider.cpp
    src/hotspot/RealNewsProvider.h
    resources.qrc
    resources/QtTheme/QtTheme.qrc
)

# macOS 应用图标
set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/resources/AppIcon.icns)
set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

qt_add_executable(AILoginSystem
    ${PROJECT_SOURCES}
    ${APP_ICON_MACOSX}
)

target_link_libraries(AILoginSystem PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Svg
)

# 设置目标属性
set_target_properties(AILoginSystem PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.aiedu.loginsystem
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE ${MACOSX_BUNDLE_ICON_FILE}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    RESOURCE "${CMAKE_SOURCE_DIR}/resources/ppt/爱国主义精神传承.pptx"
)

# 复制 PPT 资源到 App Bundle (macOS)
if(APPLE)
    add_custom_command(TARGET AILoginSystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:AILoginSystem>/Contents/Resources/ppt"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/resources/ppt/爱国主义精神传承.pptx"
            "$<TARGET_BUNDLE_DIR:AILoginSystem>/Contents/Resources/ppt/"
        COMMENT "Copying PPT resources to app bundle"
    )
endif()

# 安装规则
include(GNUInstallDirs)
install(TARGETS AILoginSystem
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ==================== ImportTool 导入工具 ====================
# 共享的服务源文件
set(SHARED_SERVICES
    src/services/PaperService.cpp
    src/services/PaperService.h
    src/services/BulkImportService.cpp
    src/services/BulkImportService.h
    src/services/QuestionParserService.cpp
    src/services/QuestionParserService.h
    src/services/DocumentReaderService.cpp
    src/services/DocumentReaderService.h
    src/services/SupabaseStorageService.cpp
    src/services/SupabaseStorageService.h
    src/auth/supabase/supabaseconfig.cpp
    src/auth/supabase/supabaseconfig.h
)

qt_add_executable(ImportTool
    src/tools/import_tool.cpp
    ${SHARED_SERVICES}
)

target_link_libraries(ImportTool PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)

set_target_properties(ImportTool PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE FALSE
)
