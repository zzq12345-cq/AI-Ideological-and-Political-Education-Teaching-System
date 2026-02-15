cmake_minimum_required(VERSION 3.16)

project(AILoginSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network QuickWidgets Svg SvgWidgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Charts QuickWidgets Svg SvgWidgets PrintSupport)

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
    src/services/HotspotService.cpp
    src/services/HotspotService.h
    src/hotspot/INewsProvider.h
    src/hotspot/NewsItem.h
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
    src/config/embedded_keys.h
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
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
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
    src/utils/NetworkRequestFactory.cpp
    src/utils/NetworkRequestFactory.h
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
