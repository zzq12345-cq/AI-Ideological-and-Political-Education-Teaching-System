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

signals:
    void backRequested();  // 返回请求

private slots:
    void onClassChanged(int index);
    void onDateChanged(const QDate &date);
    void onLessonChanged(int index);
    void onSaveClicked();
    void onRefreshClicked();
    void onAllPresentClicked();
    void onStatusButtonClicked(int studentIndex, AttendanceStatus status);
    void onRemarkClicked(int studentIndex);
    void onStudentsLoaded();
    void onAttendanceLoaded();

private:
    void setupUI();
    void setupStyles();
    void setupConnections();
    void createHeaderCard();
    void createFilterCard();
    void createSummaryCard();
    void createStudentListCard();
    void createActionCard();
    void applyCardShadow(QWidget *widget, qreal blur = 16, qreal offset = 4);

    // 新增：创建统计卡片
    QWidget* createStatCard(const QString &label, QLabel* &countLabel, const QString &color, const QString &iconPath);
    // 新增：获取莫兰迪色背景
    QString getMorandiColor(int index);

    void loadStudentList();
    void updateStatistics();
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
    QComboBox *m_lessonCombo = nullptr;

    // 统计标签
    QLabel *m_presentCountLabel = nullptr;
    QLabel *m_absentCountLabel = nullptr;
    QLabel *m_lateCountLabel = nullptr;
    QLabel *m_leaveCountLabel = nullptr;
    QLabel *m_earlyCountLabel = nullptr;

    // 操作按钮
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_allPresentBtn = nullptr;
    QPushButton *m_saveBtn = nullptr;
    QPushButton *m_statsBtn = nullptr;
    QPushButton *m_exportBtn = nullptr;

    // 数据
    QList<AttendanceRecord> m_records;
    QList<QButtonGroup*> m_statusButtonGroups;

    // 服务
    AttendanceService *m_service = nullptr;
};

#endif // ATTENDANCEWIDGET_H
