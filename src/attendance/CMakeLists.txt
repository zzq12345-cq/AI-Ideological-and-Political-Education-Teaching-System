# 考勤管理模块
# 老王说：模块化构建，清晰明了

set(ATTENDANCE_SOURCES
    models/AttendanceRecord.cpp
    services/AttendanceService.cpp
    ui/AttendanceWidget.cpp
)

set(ATTENDANCE_HEADERS
    models/AttendanceStatus.h
    models/AttendanceRecord.h
    models/AttendanceSummary.h
    services/AttendanceService.h
    ui/AttendanceWidget.h
)

# 将源文件添加到父级目标（在主CMakeLists.txt中引用）
set(ATTENDANCE_SOURCES ${ATTENDANCE_SOURCES} PARENT_SCOPE)
set(ATTENDANCE_HEADERS ${ATTENDANCE_HEADERS} PARENT_SCOPE)
