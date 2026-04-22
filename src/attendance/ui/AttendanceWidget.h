#ifndef ATTENDANCEWIDGET_H
#define ATTENDANCEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QListWidget>
#include <QFrame>
#include <QScrollArea>
#include <QButtonGroup>
#include <QIcon>
#include "../models/AttendanceRecord.h"
#include "../../analytics/models/Student.h"

class AttendanceService;

/**
 * @brief 考勤管理主界面组件
 * 老王说：根据设计稿完整实现，中国红主题，卡片式布局
 */
class AttendanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AttendanceWidget(QWidget *parent = nullptr);
    ~AttendanceWidget();

    // 设置考勤服务
    void setAttendanceService(AttendanceService *service);

    // 加载指定 session 的考勤结果，可指定 classId 自动设置选择器
    void loadSessionResults(const QString &sessionId, const QString &classId = QString());

signals:
    void backRequested();  // 返回请求

private slots:
    void onClassChanged(int index);
    void onDateChanged(const QDate &date);
    void onSessionChanged(int index);
    void onRefreshClicked();
    void onSaveChanges();
    void onResetChanges();
    void onStatusButtonClicked(int studentIndex, AttendanceStatus status);
    void onRemarkClicked(int studentIndex);

private:
    void setupUI();
    void setupStyles();
    void setupConnections();
    void createHeaderCard();
    void createSummaryCard();
    void createStudentListCard();
    void applyCardShadow(QWidget *widget, qreal blur = 16, qreal offset = 4);

    QWidget* createStatCard(const QString &label, QLabel* &countLabel, const QString &color, const QString &iconPath);
    QString getMorandiColor(int index);

    void loadAttendanceForCurrentSelection();
    void loadStudentList();
    void updateStatistics();
    void updateSaveButtonState();
    QWidget* createStudentItem(int index, const QString &name, const QString &studentNo, AttendanceStatus status);
    QWidget* createStatusButtonGroup(int studentIndex, AttendanceStatus currentStatus);

    // SVG 图标加载辅助方法
    QIcon loadSvgIcon(const QString &path, const QString &color = QString());
    void setButtonIcon(QPushButton *btn, const QString &iconPath, const QString &color = QString());

    // UI 组件
    QVBoxLayout *m_mainLayout = nullptr;
    QFrame *m_headerCard = nullptr;
    QFrame *m_filterCard = nullptr;
    QFrame *m_summaryCard = nullptr;
    QFrame *m_listCard = nullptr;
    QFrame *m_actionCard = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_listContainer = nullptr;
    QVBoxLayout *m_listLayout = nullptr;

    // 筛选组件
    QComboBox *m_classCombo = nullptr;
    QDateEdit *m_dateEdit = nullptr;
    QComboBox *m_sessionCombo = nullptr;

    // 统计标签
    QLabel *m_presentCountLabel = nullptr;
    QLabel *m_absentCountLabel = nullptr;
    QLabel *m_lateCountLabel = nullptr;
    QLabel *m_leaveCountLabel = nullptr;
    QLabel *m_earlyCountLabel = nullptr;

    // 操作按钮
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_saveChangesBtn = nullptr;
    QPushButton *m_resetBtn = nullptr;

    // 数据
    QList<AttendanceRecord> m_records;
    QList<AttendanceStatus> m_originalStatuses;  // 用于检测变更和重置
    QList<QButtonGroup*> m_statusButtonGroups;
    QString m_currentSessionId;
    QString m_currentClassId;
    QString m_targetSessionId;    // 跳转时需要自动选中的 session
    QStringList m_recordIds;       // Supabase record IDs
    QStringList m_studentEmails;   // 对应的邮箱
    QMap<QString, QString> m_emailToStudentNo; // email → 学号
    QList<Student> m_students;

    // 服务
    AttendanceService *m_service = nullptr;
};

#endif // ATTENDANCEWIDGET_H
