#ifndef ATTENDANCESUMMARY_H
#define ATTENDANCESUMMARY_H

#include <QJsonObject>

/**
 * @brief 考勤统计汇总模型
 * 老王说：统计数据就这几个字段，出勤率是核心指标
 */
class AttendanceSummary
{
public:
    AttendanceSummary()
        : m_totalCount(0)
        , m_presentCount(0)
        , m_absentCount(0)
        , m_lateCount(0)
        , m_leaveCount(0)
        , m_earlyLeaveCount(0)
        , m_attendanceRate(0.0)
    {}

    // 从JSON构造
    static AttendanceSummary fromJson(const QJsonObject &json)
    {
        AttendanceSummary summary;
        summary.m_totalCount = json["total_count"].toInt();
        summary.m_presentCount = json["present_count"].toInt();
        summary.m_absentCount = json["absent_count"].toInt();
        summary.m_lateCount = json["late_count"].toInt();
        summary.m_leaveCount = json["leave_count"].toInt();
        summary.m_earlyLeaveCount = json["early_leave_count"].toInt();
        summary.m_attendanceRate = json["attendance_rate"].toDouble();
        return summary;
    }

    // 转换为JSON
    QJsonObject toJson() const
    {
        QJsonObject json;
        json["total_count"] = m_totalCount;
        json["present_count"] = m_presentCount;
        json["absent_count"] = m_absentCount;
        json["late_count"] = m_lateCount;
        json["leave_count"] = m_leaveCount;
        json["early_leave_count"] = m_earlyLeaveCount;
        json["attendance_rate"] = m_attendanceRate;
        return json;
    }

    // 计算出勤率（出勤+迟到 / 总人次）
    void calculateRate()
    {
        if (m_totalCount > 0) {
            // 出勤和迟到都算"到场"
            m_attendanceRate = static_cast<double>(m_presentCount + m_lateCount) / m_totalCount * 100.0;
        } else {
            m_attendanceRate = 0.0;
        }
    }

    // 从考勤记录列表统计
    void reset()
    {
        m_totalCount = 0;
        m_presentCount = 0;
        m_absentCount = 0;
        m_lateCount = 0;
        m_leaveCount = 0;
        m_earlyLeaveCount = 0;
        m_attendanceRate = 0.0;
    }

    // Getters
    int totalCount() const { return m_totalCount; }
    int presentCount() const { return m_presentCount; }
    int absentCount() const { return m_absentCount; }
    int lateCount() const { return m_lateCount; }
    int leaveCount() const { return m_leaveCount; }
    int earlyLeaveCount() const { return m_earlyLeaveCount; }
    double attendanceRate() const { return m_attendanceRate; }

    // Setters
    void setTotalCount(int count) { m_totalCount = count; }
    void setPresentCount(int count) { m_presentCount = count; }
    void setAbsentCount(int count) { m_absentCount = count; }
    void setLateCount(int count) { m_lateCount = count; }
    void setLeaveCount(int count) { m_leaveCount = count; }
    void setEarlyLeaveCount(int count) { m_earlyLeaveCount = count; }
    void setAttendanceRate(double rate) { m_attendanceRate = rate; }

    // 累加操作
    void addPresent() { m_presentCount++; m_totalCount++; }
    void addAbsent() { m_absentCount++; m_totalCount++; }
    void addLate() { m_lateCount++; m_totalCount++; }
    void addLeave() { m_leaveCount++; m_totalCount++; }
    void addEarlyLeave() { m_earlyLeaveCount++; m_totalCount++; }

private:
    int m_totalCount;        // 总应到人次
    int m_presentCount;      // 出勤人次
    int m_absentCount;       // 缺勤人次
    int m_lateCount;         // 迟到人次
    int m_leaveCount;        // 请假人次
    int m_earlyLeaveCount;   // 早退人次
    double m_attendanceRate; // 出勤率（百分比）
};

#endif // ATTENDANCESUMMARY_H
