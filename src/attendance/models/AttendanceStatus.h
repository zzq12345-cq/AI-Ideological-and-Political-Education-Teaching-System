#ifndef ATTENDANCESTATUS_H
#define ATTENDANCESTATUS_H

#include <QString>

/**
 * @brief 考勤状态枚举
 * 老王说：5种状态够用了，别整那些花里胡哨的
 */
enum class AttendanceStatus {
    Present = 0,    // 出勤
    Absent = 1,     // 缺勤
    Late = 2,       // 迟到
    Leave = 3,      // 请假
    EarlyLeave = 4  // 早退
};

/**
 * @brief 考勤状态工具类
 * 老王说：枚举转字符串、字符串转枚举，这种SB操作封装起来
 */
class AttendanceStatusHelper
{
public:
    // 枚举转字符串（用于API传输）
    static QString toString(AttendanceStatus status)
    {
        switch (status) {
        case AttendanceStatus::Present:   return "present";
        case AttendanceStatus::Absent:    return "absent";
        case AttendanceStatus::Late:      return "late";
        case AttendanceStatus::Leave:     return "leave";
        case AttendanceStatus::EarlyLeave: return "early_leave";
        default: return "present";
        }
    }

    // 字符串转枚举
    static AttendanceStatus fromString(const QString &str)
    {
        if (str == "present")     return AttendanceStatus::Present;
        if (str == "absent")      return AttendanceStatus::Absent;
        if (str == "late")        return AttendanceStatus::Late;
        if (str == "leave")       return AttendanceStatus::Leave;
        if (str == "early_leave") return AttendanceStatus::EarlyLeave;
        return AttendanceStatus::Present;  // 默认出勤
    }

    // 整数转枚举（用于数据库存储）
    static AttendanceStatus fromInt(int value)
    {
        if (value >= 0 && value <= 4) {
            return static_cast<AttendanceStatus>(value);
        }
        return AttendanceStatus::Present;
    }

    // 枚举转整数
    static int toInt(AttendanceStatus status)
    {
        return static_cast<int>(status);
    }

    // 获取中文显示名称
    static QString displayName(AttendanceStatus status)
    {
        switch (status) {
        case AttendanceStatus::Present:   return "出勤";
        case AttendanceStatus::Absent:    return "缺勤";
        case AttendanceStatus::Late:      return "迟到";
        case AttendanceStatus::Leave:     return "请假";
        case AttendanceStatus::EarlyLeave: return "早退";
        default: return "出勤";
        }
    }

    // 获取状态对应的颜色（用于UI显示）
    static QString colorHex(AttendanceStatus status)
    {
        switch (status) {
        case AttendanceStatus::Present:   return "#4CAF50";  // 绿色
        case AttendanceStatus::Absent:    return "#F44336";  // 红色
        case AttendanceStatus::Late:      return "#FF9800";  // 橙色
        case AttendanceStatus::Leave:     return "#2196F3";  // 蓝色
        case AttendanceStatus::EarlyLeave: return "#9C27B0"; // 紫色
        default: return "#4CAF50";
        }
    }
};

#endif // ATTENDANCESTATUS_H
