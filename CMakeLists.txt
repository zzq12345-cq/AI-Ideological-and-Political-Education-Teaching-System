cmake_minimum_required(VERSION 3.16)

project(AILoginSystem VERSION 2.1.10 LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network QuickWidgets Svg SvgWidgets PrintSupport Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Charts QuickWidgets Svg SvgWidgets PrintSupport Concurrent)
# zlib: 系统有就用系统的，Windows 上使用 Qt 内置 bundled zlib
find_package(ZLIB QUIET)
if(NOT ZLIB_FOUND AND WIN32)
    # Qt 6 在 Windows 上自带 zlib 头文件和静态库
    # 获取 Qt 安装根目录
    get_target_property(_qt_include_dir Qt${QT_VERSION_MAJOR}::Core INTERFACE_INCLUDE_DIRECTORIES)
    # 从 Qt include 路径推断根目录（取第一个路径的父目录）
    list(GET _qt_include_dir 0 _qt_first_include)
    get_filename_component(_qt_root "${_qt_first_include}" DIRECTORY)

    # 尝试多种可能的 zlib.h 路径
    foreach(_zlib_candidate
        "${_qt_root}/include/QtZlib"
        "${_qt_root}/QtZlib"
        "${_qt_first_include}/QtZlib"
    )
        if(EXISTS "${_zlib_candidate}/zlib.h")
            set(_qt_zlib_include "${_zlib_candidate}")
            break()
        endif()
    endforeach()

    if(_qt_zlib_include)
        message(STATUS "Using Qt bundled zlib from: ${_qt_zlib_include}")
        add_library(qt_bundled_zlib INTERFACE)
        target_include_directories(qt_bundled_zlib INTERFACE "${_qt_zlib_include}")
        # 查找 zlib 静态库
        find_library(QT_ZLIB_LIB NAMES zlib z PATHS "${_qt_root}/lib" NO_DEFAULT_PATH)
        if(QT_ZLIB_LIB)
            target_link_libraries(qt_bundled_zlib INTERFACE "${QT_ZLIB_LIB}")
        endif()
    else()
        message(WARNING "Cannot find Qt bundled zlib.h, SimpleZipWriter may fail to compile")
        add_library(qt_bundled_zlib INTERFACE)
    endif()
endif()

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
    src/dashboard/ChatManager.cpp
    src/dashboard/ChatManager.h
    src/dashboard/SidebarManager.cpp
    src/dashboard/SidebarManager.h
    src/questionbank/questionbankwindow.cpp
    src/questionbank/questionbankwindow.h
    src/questionbank/QuestionBasket.cpp
    src/questionbank/QuestionBasket.h
    src/questionbank/QuestionBasketWidget.cpp
    src/questionbank/QuestionBasketWidget.h
    src/questionbank/PaperComposerDialog.cpp
    src/questionbank/PaperComposerDialog.h
    src/questionbank/QualityCheckDialog.cpp
    src/questionbank/QualityCheckDialog.h
    src/ui/aipreparationwidget.cpp
    src/ui/aipreparationwidget.h
    src/ui/pptpreviewpage.cpp
    src/ui/pptpreviewpage.h
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
    src/services/ZhipuPPTAgentService.cpp
    src/services/ZhipuPPTAgentService.h
    src/services/PaperService.cpp
    src/services/PaperService.h
    src/services/QuestionQualityService.cpp
    src/services/QuestionQualityService.h
    src/services/CurriculumService.cpp
    src/services/CurriculumService.h
    src/ui/ChatWidget.cpp
    src/ui/ChatWidget.h
    src/ui/LessonPlanEditor.cpp
    src/ui/LessonPlanEditor.h
    src/ui/AIChatDialog.cpp
    src/ui/AIChatDialog.h
    src/utils/MarkdownRenderer.cpp
    src/utils/MarkdownRenderer.h
    src/utils/NetworkRequestFactory.cpp
    src/utils/NetworkRequestFactory.h
    src/utils/NetworkRetryHelper.cpp
    src/utils/NetworkRetryHelper.h
    src/utils/FailedTaskTracker.cpp
    src/utils/FailedTaskTracker.h
    src/utils/LayoutUtils.h
    src/utils/SimpleZipWriter.cpp
    src/utils/SimpleZipWriter.h
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
    src/hotspot/NewsCategoryUtils.cpp
    src/hotspot/NewsCategoryUtils.h
    src/hotspot/RealNewsProvider.cpp
    src/hotspot/RealNewsProvider.h
    src/settings/UserSettingsDialog.cpp
    src/settings/UserSettingsDialog.h
    src/settings/UserSettingsManager.cpp
    src/settings/UserSettingsManager.h
    src/analytics/DataAnalyticsWidget.cpp
    src/analytics/DataAnalyticsWidget.h
    src/analytics/AnalyticsDataService.cpp
    src/analytics/AnalyticsDataService.h
    # 数据模型
    src/analytics/models/Student.cpp
    src/analytics/models/Student.h
    src/analytics/models/CourseClass.cpp
    src/analytics/models/CourseClass.h
    src/analytics/models/ScoreRecord.cpp
    src/analytics/models/ScoreRecord.h
    src/analytics/models/KnowledgePoint.cpp
    src/analytics/models/KnowledgePoint.h
    src/analytics/models/AnalysisResult.cpp
    src/analytics/models/AnalysisResult.h
    src/analytics/models/ClassStatistics.cpp
    src/analytics/models/ClassStatistics.h
    # 数据源接口和实现
    src/analytics/interfaces/IAnalyticsDataSource.h
    src/analytics/datasources/MockDataSource.cpp
    src/analytics/datasources/MockDataSource.h
    # UI组件
    src/analytics/ui/AnalyticsNavigationBar.cpp
    src/analytics/ui/AnalyticsNavigationBar.h
    src/analytics/ui/PersonalAnalyticsPage.cpp
    src/analytics/ui/PersonalAnalyticsPage.h
    src/analytics/ui/ClassAnalyticsPage.cpp
    src/analytics/ui/ClassAnalyticsPage.h
    src/analytics/ui/RadarChartWidget.cpp
    src/analytics/ui/RadarChartWidget.h
    # 通知模块
    src/notifications/models/Notification.cpp
    src/notifications/models/Notification.h
    src/notifications/NotificationService.cpp
    src/notifications/NotificationService.h
    src/notifications/ui/NotificationBadge.cpp
    src/notifications/ui/NotificationBadge.h
    src/notifications/ui/NotificationWidget.cpp
    src/notifications/ui/NotificationWidget.h
    src/attendance/models/AttendanceRecord.cpp
    src/attendance/models/AttendanceRecord.h
    src/attendance/models/AttendanceStatus.h
    src/attendance/models/AttendanceSummary.h
    src/attendance/services/AttendanceService.cpp
    src/attendance/services/AttendanceService.h
    src/attendance/ui/AttendanceWidget.cpp
    src/attendance/ui/AttendanceWidget.h
    # 智能组卷模块
    src/smartpaper/SmartPaperConfig.h
    src/smartpaper/SmartPaperService.cpp
    src/smartpaper/SmartPaperService.h
    src/smartpaper/SmartPaperWidget.cpp
    src/smartpaper/SmartPaperWidget.h
    src/config/AppConfig.cpp
    src/config/AppConfig.h
    src/config/embedded_keys.h
    resources.qrc
    resources/QtTheme/QtTheme.qrc
)

if(APPLE)
    list(APPEND PROJECT_SOURCES
        src/ui/macosquicklookpreview.mm
        src/ui/macosquicklookpreview.h
    )
else()
    list(APPEND PROJECT_SOURCES
        src/ui/macosquicklookpreview.cpp
        src/ui/macosquicklookpreview.h
    )
endif()

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
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Concurrent
    $<$<TARGET_EXISTS:ZLIB::ZLIB>:ZLIB::ZLIB>
    $<$<TARGET_EXISTS:qt_bundled_zlib>:qt_bundled_zlib>
)

if(APPLE)
    target_link_libraries(AILoginSystem PRIVATE
        "-framework QuickLookUI"
        "-framework Quartz"
    )
endif()

# 设置目标属性
set(PPT_DEMO_RESOURCE "${CMAKE_SOURCE_DIR}/resources/ppt/爱国主义精神传承.pptx")
set(PPT_TEMPLATE_RESOURCE "${CMAKE_SOURCE_DIR}/resources/templates/party_red_template.pptx")

set_target_properties(AILoginSystem PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.aiedu.loginsystem
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_ICON_FILE ${MACOSX_BUNDLE_ICON_FILE}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    RESOURCE "${PPT_DEMO_RESOURCE};${PPT_TEMPLATE_RESOURCE}"
)

# 复制 PPT 资源到 App Bundle (macOS)
if(APPLE)
    add_custom_command(TARGET AILoginSystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:AILoginSystem>/Contents/Resources/ppt"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:AILoginSystem>/Contents/Resources/templates"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PPT_DEMO_RESOURCE}"
            "$<TARGET_BUNDLE_DIR:AILoginSystem>/Contents/Resources/ppt/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PPT_TEMPLATE_RESOURCE}"
            "$<TARGET_BUNDLE_DIR:AILoginSystem>/Contents/Resources/templates/"
        COMMENT "Copying PPT resources to app bundle"
    )
elseif(WIN32)
    # 复制 PPT 资源到 exe 同级目录 (Windows)
    add_custom_command(TARGET AILoginSystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AILoginSystem>/resources/ppt"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AILoginSystem>/resources/templates"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PPT_DEMO_RESOURCE}"
            "$<TARGET_FILE_DIR:AILoginSystem>/resources/ppt/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PPT_TEMPLATE_RESOURCE}"
            "$<TARGET_FILE_DIR:AILoginSystem>/resources/templates/"
        COMMENT "Copying PPT resources to exe directory"
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
    src/services/QuestionQualityService.cpp
    src/services/QuestionQualityService.h
    src/services/DifyService.cpp
    src/services/DifyService.h
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
    src/utils/NetworkRequestFactory.cpp
    src/utils/NetworkRequestFactory.h
    src/utils/NetworkRetryHelper.cpp
    src/utils/NetworkRetryHelper.h
    src/utils/FailedTaskTracker.cpp
    src/utils/FailedTaskTracker.h
    src/config/AppConfig.cpp
    src/config/AppConfig.h
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
