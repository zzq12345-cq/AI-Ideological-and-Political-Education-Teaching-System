#include "AttendanceService.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>
#include <QTimer>
#include <QRandomGenerator>

// Supabase配置（复用项目现有配置）
static const QString SUPABASE_URL = "https://your-project.supabase.co";
static const QString SUPABASE_ANON_KEY = "your-anon-key";

AttendanceService::AttendanceService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

AttendanceService::~AttendanceService()
{
}

void AttendanceService::setCurrentUserId(const QString &userId)
{
    m_currentUserId = userId;
}

QNetworkRequest AttendanceService::createRequest(const QString &endpoint) const
{
    QUrl url(SUPABASE_URL + endpoint);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("apikey", SUPABASE_ANON_KEY.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + SUPABASE_ANON_KEY).toUtf8());
    request.setRawHeader("Prefer", "return=representation");

    // 禁用HTTP/2（项目统一配置）
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    return request;
}

void AttendanceService::setLoading(bool loading)
{
    if (m_isLoading != loading) {
        m_isLoading = loading;
        emit loadingStateChanged(loading);
    }
}

// ============ 获取班级学生列表 ============

void AttendanceService::fetchStudentsByClass(int classId)
{
    setLoading(true);

    // 开发阶段使用示例数据
    loadSampleStudents(classId);
}

void AttendanceService::loadSampleStudents(int classId)
{
    // 老王说：用真实感的名字，不要那种SB的"小X"格式
    m_students.clear();

    // 真实风格的中国学生姓名
    QStringList names = {
        "林雨萱", "张浩然", "王思琪", "李俊杰", "陈雨涵",
        "杨子轩", "刘梓涵", "赵晨曦", "孙一诺", "周雨彤",
        "吴宇航", "郑佳怡", "马天翼", "黄诗涵", "何嘉豪"
    };

    for (int i = 0; i < names.size(); ++i) {
        Student student;
        student.setId(i + 1);
        student.setName(names[i]);
        student.setClassId(classId);
        student.setStudentNo(QString("2024%1%2").arg(classId, 2, 10, QChar('0')).arg(i + 1, 2, 10, QChar('0')));
        m_students.append(student);
    }

    setLoading(false);
    emit studentsReceived(m_students);
}

// ============ 获取考勤记录 ============

void AttendanceService::fetchAttendanceByLesson(int classId, const QDate &date, int lessonNumber)
{
    setLoading(true);

    // 开发阶段使用示例数据
    loadSampleAttendance(classId, date, lessonNumber);
}

void AttendanceService::loadSampleAttendance(int classId, const QDate &date, int lessonNumber)
{
    // 老王说：假数据，大部分出勤，少数缺勤迟到
    m_attendanceRecords.clear();

    // 先确保有学生数据
    if (m_students.isEmpty()) {
        loadSampleStudents(classId);
    }

    for (const Student &student : m_students) {
        AttendanceRecord record(student.id(), classId, date, lessonNumber);
        record.setRecorderId(m_currentUserId);

        // 随机分配状态（90%出勤）
        int random = QRandomGenerator::global()->bounded(100);
        if (random < 90) {
            record.setStatus(AttendanceStatus::Present);
        } else if (random < 95) {
            record.setStatus(AttendanceStatus::Late);
        } else if (random < 98) {
            record.setStatus(AttendanceStatus::Leave);
            record.setRemark("事假");
        } else {
            record.setStatus(AttendanceStatus::Absent);
        }

        m_attendanceRecords.append(record);
    }

    // 计算统计
    m_currentSummary.reset();
    for (const AttendanceRecord &record : m_attendanceRecords) {
        switch (record.status()) {
        case AttendanceStatus::Present:   m_currentSummary.addPresent(); break;
        case AttendanceStatus::Absent:    m_currentSummary.addAbsent(); break;
        case AttendanceStatus::Late:      m_currentSummary.addLate(); break;
        case AttendanceStatus::Leave:     m_currentSummary.addLeave(); break;
        case AttendanceStatus::EarlyLeave: m_currentSummary.addEarlyLeave(); break;
        }
    }
    m_currentSummary.calculateRate();

    setLoading(false);
    emit attendanceReceived(m_attendanceRecords);
    emit statisticsReceived(m_currentSummary);
}

void AttendanceService::fetchAttendanceByDateRange(int classId, const QDate &startDate, const QDate &endDate)
{
    Q_UNUSED(classId)
    Q_UNUSED(startDate)
    Q_UNUSED(endDate)
    // TODO: 实现日期范围查询
    setLoading(true);
    setLoading(false);
}

// ============ 提交考勤记录 ============

void AttendanceService::submitAttendance(const QList<AttendanceRecord> &records)
{
    if (records.isEmpty()) {
        emit attendanceSubmitted(false, "没有考勤记录需要提交");
        return;
    }

    setLoading(true);

    // 开发阶段模拟提交成功
    // TODO: 实现真实的Supabase提交
    QTimer::singleShot(500, this, [this, records]() {
        m_attendanceRecords = records;
        setLoading(false);
        emit attendanceSubmitted(true, QString("成功提交 %1 条考勤记录").arg(records.size()));
    });
}

void AttendanceService::onSubmitAttendanceFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    setLoading(false);

    if (reply->error() == QNetworkReply::NoError) {
        emit attendanceSubmitted(true, "考勤记录提交成功");
    } else {
        emit attendanceSubmitted(false, reply->errorString());
    }

    reply->deleteLater();
}

// ============ 更新考勤记录 ============

void AttendanceService::updateAttendance(const AttendanceRecord &record)
{
    Q_UNUSED(record)
    setLoading(true);

    // TODO: 实现真实的更新API
    QTimer::singleShot(300, this, [this]() {
        setLoading(false);
        emit attendanceUpdated(true);
    });
}

void AttendanceService::onUpdateAttendanceFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    setLoading(false);
    emit attendanceUpdated(reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
}

// ============ 删除考勤记录 ============

void AttendanceService::deleteAttendance(int recordId)
{
    Q_UNUSED(recordId)
    setLoading(true);

    // TODO: 实现真实的删除API
    QTimer::singleShot(300, this, [this]() {
        setLoading(false);
        emit attendanceDeleted(true);
    });
}

void AttendanceService::onDeleteAttendanceFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    setLoading(false);
    emit attendanceDeleted(reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
}

// ============ 统计相关 ============

void AttendanceService::fetchClassStatistics(int classId, const QDate &startDate, const QDate &endDate)
{
    Q_UNUSED(classId)
    Q_UNUSED(startDate)
    Q_UNUSED(endDate)

    // TODO: 实现班级统计API
    emit statisticsReceived(m_currentSummary);
}

void AttendanceService::fetchStudentStatistics(int studentId, const QDate &startDate, const QDate &endDate)
{
    Q_UNUSED(studentId)
    Q_UNUSED(startDate)
    Q_UNUSED(endDate)

    // TODO: 实现学生统计API
    AttendanceSummary summary;
    emit statisticsReceived(summary);
}

// ============ 网络回调 ============

void AttendanceService::onFetchStudentsFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    setLoading(false);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();

        m_students.clear();
        for (const QJsonValue &value : array) {
            m_students.append(Student::fromJson(value.toObject()));
        }

        emit studentsReceived(m_students);
    } else {
        emit errorOccurred(reply->errorString());
    }

    reply->deleteLater();
}

void AttendanceService::onFetchAttendanceFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    setLoading(false);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray array = doc.array();

        m_attendanceRecords.clear();
        for (const QJsonValue &value : array) {
            m_attendanceRecords.append(AttendanceRecord::fromJson(value.toObject()));
        }

        emit attendanceReceived(m_attendanceRecords);
    } else {
        emit errorOccurred(reply->errorString());
    }

    reply->deleteLater();
}

void AttendanceService::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
    }
    setLoading(false);
}
