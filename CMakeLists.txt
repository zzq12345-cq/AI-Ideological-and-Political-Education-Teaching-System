cmake_minimum_required(VERSION 3.16)

project(AILoginSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network QuickWidgets Svg SvgWidgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Charts QuickWidgets Svg SvgWidgets PrintSupport)
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    set(AI_ZLIB_TARGET ZLIB::ZLIB)
elseif(WIN32)
    get_target_property(_qt_include_dir Qt${QT_VERSION_MAJOR}::Core INTERFACE_INCLUDE_DIRECTORIES)
    list(GET _qt_include_dir 0 _qt_first_include)
    get_filename_component(_qt_root "${_qt_first_include}" DIRECTORY)

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

    if(NOT _qt_zlib_include)
        message(FATAL_ERROR "Could NOT find ZLIB or Qt bundled zlib")
    endif()

    message(STATUS "Using Qt bundled zlib from: ${_qt_zlib_include}")
    add_library(qt_bundled_zlib INTERFACE)
    target_include_directories(qt_bundled_zlib INTERFACE "${_qt_zlib_include}")
    find_library(QT_ZLIB_LIB NAMES zlib z PATHS "${_qt_root}/lib" NO_DEFAULT_PATH)
    if(QT_ZLIB_LIB)
        target_link_libraries(qt_bundled_zlib INTERFACE "${QT_ZLIB_LIB}")
    endif()
    set(AI_ZLIB_TARGET qt_bundled_zlib)
else()
    message(FATAL_ERROR "Could NOT find ZLIB")
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
    src/questionbank/AIQuestionGenWidget.cpp
    src/questionbank/AIQuestionGenWidget.h
    src/questionbank/CurriculumData.h
    src/questionbank/QuestionRepository.cpp
    src/questionbank/QuestionRepository.h
    src/questionbank/QuestionBasket.cpp
    src/questionbank/QuestionBasket.h
    src/questionbank/QuestionBasketWidget.cpp
    src/questionbank/QuestionBasketWidget.h
    src/questionbank/QualityCheckDialog.cpp
    src/questionbank/QualityCheckDialog.h
    src/questionbank/PaperComposerDialog.cpp
    src/questionbank/PaperComposerDialog.h
    src/smartpaper/SmartPaperConfig.h
    src/smartpaper/SmartPaperService.cpp
    src/smartpaper/SmartPaperService.h
    src/smartpaper/SmartPaperWidget.cpp
    src/smartpaper/SmartPaperWidget.h
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
    src/services/QuestionParserService.cpp
    src/services/QuestionParserService.h
    src/services/QuestionQualityService.cpp
    src/services/QuestionQualityService.h
    src/services/PPTXGenerator.cpp
    src/services/PPTXGenerator.h
    src/services/ZhipuPPTAgentService.cpp
    src/services/ZhipuPPTAgentService.h
    src/services/PaperService.cpp
    src/services/PaperService.h
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
    src/ui/ChatHistoryWidget.cpp
    src/ui/ChatHistoryWidget.h
    src/ui/NetworkImageTextBrowser.cpp
    src/ui/NetworkImageTextBrowser.h
    src/ui/HotspotTrackingWidget.cpp
    src/ui/HotspotTrackingWidget.h
    src/ui/HelpCenterWidget.cpp
    src/ui/HelpCenterWidget.h
    src/services/HotspotService.cpp
    src/services/HotspotService.h
    src/hotspot/INewsProvider.h
    src/hotspot/NewsItem.h
    src/hotspot/NewsCategoryUtils.cpp
    src/hotspot/NewsCategoryUtils.h
    src/hotspot/MockNewsProvider.cpp
    src/hotspot/MockNewsProvider.h
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
    src/analytics/ui/KnowledgeGraphWidget.cpp
    src/analytics/ui/KnowledgeGraphWidget.h
    src/analytics/models/KnowledgeGraph.cpp
    src/analytics/models/KnowledgeGraph.h
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
    src/student/MyClassWidget.cpp
    src/student/MyClassWidget.h
    src/student/JoinClassDialog.cpp
    src/student/JoinClassDialog.h
    src/student/ClassInfo.h
    src/student/ClassManager.cpp
    src/student/ClassManager.h
    src/student/CreateClassDialog.cpp
    src/student/CreateClassDialog.h
    src/student/ClassDetailWidget.cpp
    src/student/ClassDetailWidget.h
    src/student/AttendanceManager.cpp
    src/student/AttendanceManager.h
    src/student/AttendanceActiveWidget.cpp
    src/student/AttendanceActiveWidget.h
    src/student/AttendanceResultWidget.cpp
    src/student/AttendanceResultWidget.h
    src/student/AttendanceSignInDialog.cpp
    src/student/AttendanceSignInDialog.h
    src/student/ClassSettingsWidget.cpp
    src/student/ClassSettingsWidget.h
    src/student/HomeworkManager.cpp
    src/student/HomeworkManager.h
    src/student/HomeworkListWidget.cpp
    src/student/HomeworkListWidget.h
    src/student/HomeworkCreateWidget.cpp
    src/student/HomeworkCreateWidget.h
    src/student/HomeworkSubmissionsWidget.cpp
    src/student/HomeworkSubmissionsWidget.h
    src/student/MaterialManager.cpp
    src/student/MaterialManager.h
    src/student/MaterialWidget.cpp
    src/student/MaterialWidget.h
    src/student/StudentClassDetailWidget.cpp
    src/student/StudentClassDetailWidget.h
    src/student/StudentMaterialWidget.cpp
    src/student/StudentMaterialWidget.h
    src/admin/AdminManager.cpp
    src/admin/AdminManager.h
    src/admin/AdminDashboard.cpp
    src/admin/AdminDashboard.h
    src/config/AppConfig.cpp
    src/config/AppConfig.h
    src/config/AiConfig.h
    src/config/embedded_keys.h
    src/utils/FailedTaskTracker.cpp
    src/utils/FailedTaskTracker.h
    src/utils/NetworkRetryHelper.cpp
    src/utils/NetworkRetryHelper.h
    src/utils/SimpleZipWriter.cpp
    src/utils/SimpleZipWriter.h
    src/shared/ModernDialogHelper.cpp
    src/shared/ModernDialogHelper.h
    resources.qrc
    resources/QtTheme/QtTheme.qrc
)
list(TRANSFORM PROJECT_SOURCES PREPEND "${CMAKE_SOURCE_DIR}/")

if(APPLE)
    # macOS 应用图标
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
    set(APP_ICON_MACOSX ${CMAKE_SOURCE_DIR}/resources/AppIcon.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(WIN32)
    set(WINDOWS_ICON_FILE "${CMAKE_SOURCE_DIR}/resources/AppIcon_transparent.ico")
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/windows_app_icon.rc"
        "${CMAKE_CURRENT_BINARY_DIR}/windows_app_icon.rc"
        @ONLY
    )
    set(APP_ICON_WINDOWS "${CMAKE_CURRENT_BINARY_DIR}/windows_app_icon.rc")
endif()

qt_add_executable(AILoginSystem
    ${PROJECT_SOURCES}
    ${APP_ICON_MACOSX}
    ${APP_ICON_WINDOWS}
)

target_link_libraries(AILoginSystem PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    ${AI_ZLIB_TARGET}
)

# 设置目标属性
set_target_properties(AILoginSystem PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    set_target_properties(AILoginSystem PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.aiedu.loginsystem
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE ${MACOSX_BUNDLE_ICON_FILE}
    MACOSX_BUNDLE TRUE
    RESOURCE "${CMAKE_SOURCE_DIR}/resources/ppt/爱国主义精神传承.pptx"
    )
endif()

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

if(WIN32)
    add_custom_command(TARGET AILoginSystem POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AILoginSystem>/ppt"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/resources/ppt/爱国主义精神传承.pptx"
            "$<TARGET_FILE_DIR:AILoginSystem>/ppt/"
        COMMENT "Copying PPT resources next to Windows executable"
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
    src/services/DifyService.cpp
    src/services/DifyService.h
    src/services/BulkImportService.cpp
    src/services/BulkImportService.h
    src/services/QuestionParserService.cpp
    src/services/QuestionParserService.h
    src/services/QuestionQualityService.cpp
    src/services/QuestionQualityService.h
    src/services/DocumentReaderService.cpp
    src/services/DocumentReaderService.h
    src/services/SupabaseStorageService.cpp
    src/services/SupabaseStorageService.h
    src/auth/supabase/supabaseconfig.cpp
    src/auth/supabase/supabaseconfig.h
    src/config/AppConfig.cpp
    src/config/AppConfig.h
    src/config/AiConfig.h
    src/utils/FailedTaskTracker.cpp
    src/utils/FailedTaskTracker.h
    src/utils/NetworkRequestFactory.cpp
    src/utils/NetworkRequestFactory.h
    src/utils/NetworkRetryHelper.cpp
    src/utils/NetworkRetryHelper.h
)
list(TRANSFORM SHARED_SERVICES PREPEND "${CMAKE_SOURCE_DIR}/")

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
