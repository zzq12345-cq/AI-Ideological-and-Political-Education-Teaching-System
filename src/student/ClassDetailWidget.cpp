#include "ClassDetailWidget.h"
#include "ClassManager.h"
#include "AttendanceManager.h"
#include "AttendanceActiveWidget.h"
#include "AttendanceResultWidget.h"
#include "ClassSettingsWidget.h"
#include "HomeworkListWidget.h"
#include "HomeworkCreateWidget.h"
#include "HomeworkSubmissionsWidget.h"
#include "MaterialWidget.h"
#include "../shared/StyleConfig.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

ClassDetailWidget::ClassDetailWidget(const ClassInfo &info, QWidget *parent)
    : QWidget(parent)
    , m_classInfo(info)
{
    setupUI();

    // 加载成员列表
    ClassManager::instance()->loadClassMembers(m_classInfo.id);

    // 监听成员加载完成
    connect(ClassManager::instance(), &ClassManager::membersLoaded, this,
            [this](const QString &classId, const QList<ClassManager::MemberInfo> &members) {
        if (classId != m_classInfo.id) return;
        m_cachedMembers = members;
        updateMemberList(members);
        if (m_codeCountLabel) {
            m_codeCountLabel->setText(QString("共 %1 名学生").arg(members.size()));
        }
    });

    // 监听考勤开始
    connect(AttendanceManager::instance(), &AttendanceManager::attendanceStarted, this,
            [this](const AttendanceManager::SessionInfo &session) {
        if (session.classId != m_classInfo.id) return;
        showAttendanceActive(session);
    });
}

void ClassDetailWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(20);

    // 顶部：返回 + 班级名
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    QPushButton *backBtn = new QPushButton("< 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #6B7280;"
        "  font-size: 14px; padding: 4px 8px; }"
        "QPushButton:hover { color: #E53935; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &ClassDetailWidget::backRequested);

    m_classNameLabel = new QLabel(m_classInfo.name);
    m_classNameLabel->setStyleSheet(QString(
        "font-size: 24px; font-weight: 700; color: %1;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(m_classNameLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::BORDER_LIGHT));
    mainLayout->addWidget(line);

    // 班级码区域
    mainLayout->addWidget(createCodeSection());

    // 下半区：左侧按钮始终可见 + 右侧 QStackedWidget 切换
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);
    contentLayout->addWidget(createActionButtons());

    m_rightStack = new QStackedWidget();
    m_rightStack->addWidget(createMemberList());  // page 0: 成员列表
    contentLayout->addWidget(m_rightStack, 1);

    mainLayout->addLayout(contentLayout, 1);
}

QWidget* ClassDetailWidget::createCodeSection()
{
    QFrame *section = new QFrame();
    section->setObjectName("codeSection");
    section->setStyleSheet(QString(
        "#codeSection {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 #EF5350);"
        "  border-radius: 16px;"
        "}"
    ).arg(StyleConfig::PATRIOTIC_RED));

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(section);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(229, 57, 53, 50));
    shadow->setOffset(0, 4);
    section->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(section);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(12);

    // 标题行
    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("班级加课码");
    titleLabel->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 14px; background: transparent;");

    m_codeCountLabel = new QLabel("加载中...");
    m_codeCountLabel->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 12px; background: transparent;");

    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    titleRow->addWidget(m_codeCountLabel);
    layout->addLayout(titleRow);

    // 班级码显示
    QHBoxLayout *codeRow = new QHBoxLayout();
    m_codeLabel = new QLabel(m_classInfo.code);
    m_codeLabel->setStyleSheet(
        "color: white; font-size: 36px; font-weight: 700; letter-spacing: 8px; background: transparent;"
    );
    codeRow->addWidget(m_codeLabel);
    codeRow->addStretch();
    layout->addLayout(codeRow);

    // 按钮行
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *copyBtn = new QPushButton("复制");
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setStyleSheet(
        "QPushButton { padding: 6px 20px; border: 1px solid rgba(255,255,255,0.4);"
        "  border-radius: 6px; background: rgba(255,255,255,0.15); color: white; font-size: 13px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.3); }"
    );
    connect(copyBtn, &QPushButton::clicked, this, [this, copyBtn]() {
        QApplication::clipboard()->setText(m_classInfo.code);
        copyBtn->setText("已复制!");
        QTimer::singleShot(1500, this, [copyBtn]() { copyBtn->setText("复制"); });
    });

    QPushButton *refreshBtn = new QPushButton("刷新码");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(copyBtn->styleSheet());
    connect(refreshBtn, &QPushButton::clicked, this, [this]() {
        ClassManager::instance()->refreshCode(m_classInfo.id);
    });
    connect(ClassManager::instance(), &ClassManager::codeRefreshed, this,
            [this](const QString &classId, const QString &newCode) {
        if (classId == m_classInfo.id) {
            m_classInfo.code = newCode;
            m_codeLabel->setText(newCode);
        }
    });

    btnRow->addWidget(copyBtn);
    btnRow->addWidget(refreshBtn);
    layout->addLayout(btnRow);

    return section;
}

QWidget* ClassDetailWidget::createActionButtons()
{
    QWidget *container = new QWidget();
    container->setFixedWidth(200);
    container->setStyleSheet("background: transparent;");
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setSpacing(12);
    layout->setContentsMargins(0, 0, 0, 0);

    struct Action { QString name; QString desc; QString icon; };
    QList<Action> actions = {
        {"班级成员", "查看班级学生", "👥"},
        {"考勤", "查看出勤记录", "📋"},
        {"发布作业", "布置课后作业", "📝"},
        {"课程资料", "管理课程文件", "📂"},
        {"课程设置", "修改班级信息", "⚙"},
    };

    for (const auto &act : actions) {
        QPushButton *btn = new QPushButton();
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(64);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background: %1; border: 1px solid %2; border-radius: 12px;"
            "  padding: 12px 16px; text-align: left;"
            "}"
            "QPushButton:hover { border-color: %3; background: #FFF5F5; }"
        ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT, StyleConfig::PATRIOTIC_RED));

        QHBoxLayout *row = new QHBoxLayout(btn);
        row->setSpacing(10);

        QLabel *iconLabel = new QLabel(act.icon);
        iconLabel->setStyleSheet("font-size: 18px; background: transparent; border: none;");

        QVBoxLayout *textCol = new QVBoxLayout();
        textCol->setSpacing(2);

        QLabel *nameLabel = new QLabel(act.name);
        nameLabel->setStyleSheet(QString(
            "font-size: 14px; font-weight: 600; color: %1; background: transparent;"
        ).arg(StyleConfig::TEXT_PRIMARY));

        QLabel *descLabel = new QLabel(act.desc);
        descLabel->setStyleSheet("font-size: 11px; color: #9CA3AF; background: transparent;");

        textCol->addWidget(nameLabel);
        textCol->addWidget(descLabel);

        row->addWidget(iconLabel);
        row->addLayout(textCol);
        row->addStretch();

        QString actName = act.name;
        connect(btn, &QPushButton::clicked, this, [this, actName]() {
            if (actName == "考勤") {
                bool ok;
                QString attName = QInputDialog::getText(this, "开始考勤",
                    "请输入此次考勤名称（如：第3周周一）:", QLineEdit::Normal,
                    QDate::currentDate().toString("MM月dd日考勤"), &ok);
                if (ok) {
                    AttendanceManager::instance()->startAttendance(m_classInfo.id, attName);
                }
            } else if (actName == "课程设置") {
                showClassSettings();
            } else if (actName == "发布作业") {
                showHomeworkList();
            } else if (actName == "班级成员") {
                showMemberList();
            } else if (actName == "课程资料") {
                showMaterials();
            } else {
                QMessageBox::information(nullptr, actName, actName + "功能开发中...");
            }
        });

        layout->addWidget(btn);
    }

    layout->addStretch();
    return container;
}

QWidget* ClassDetailWidget::createMemberList()
{
    m_memberSection = new QFrame();
    m_memberSection->setObjectName("memberSection");
    m_memberSection->setStyleSheet(QString(
        "#memberSection {"
        "  background: %1; border: 1px solid %2; border-radius: 12px;"
        "}"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    m_memberLayout = new QVBoxLayout(m_memberSection);
    m_memberLayout->setContentsMargins(20, 16, 20, 16);
    m_memberLayout->setSpacing(8);

    // 标题行
    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("班级成员");
    titleLabel->setStyleSheet(QString(
        "font-size: 15px; font-weight: 600; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));

    m_memberCountLabel = new QLabel("加载中...");
    m_memberCountLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent;");

    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(m_memberCountLabel);
    m_memberLayout->addLayout(headerRow);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px;").arg(StyleConfig::SEPARATOR));
    m_memberLayout->addWidget(line);

    QLabel *placeholder = new QLabel("加载中...");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 32px; background: transparent;");
    m_memberLayout->addWidget(placeholder);
    m_memberLayout->addStretch();

    return m_memberSection;
}

void ClassDetailWidget::updateMemberList(const QList<ClassManager::MemberInfo> &members)
{
    while (m_memberLayout->count() > 2) {
        QLayoutItem *item = m_memberLayout->takeAt(m_memberLayout->count() - 1);
        delete item->widget();
        delete item;
    }

    m_memberCountLabel->setText(QString("共 %1 人").arg(members.size()));

    if (members.isEmpty()) {
        QLabel *emptyLabel = new QLabel("暂无学生加入，分享班级码邀请学生");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 32px; background: transparent;");
        m_memberLayout->addWidget(emptyLabel);
    } else {
        for (int i = 0; i < members.size(); ++i) {
            const auto &m = members[i];
            auto *rowFrame = new QFrame();
            rowFrame->setObjectName("memberRow");
            rowFrame->setStyleSheet(
                "#memberRow { background: transparent; border: none; border-bottom: 1px solid #F3F4F6; }"
                "#memberRow:hover { background: #FAFAFA; }");
            QHBoxLayout *row = new QHBoxLayout(rowFrame);
            row->setSpacing(10);
            row->setContentsMargins(4, 8, 4, 8);

            QLabel *avatarLabel = new QLabel(QString::number(i + 1));
            avatarLabel->setFixedSize(28, 28);
            avatarLabel->setAlignment(Qt::AlignCenter);
            avatarLabel->setStyleSheet(
                "background: #F3F4F6; color: #6B7280; font-size: 12px; font-weight: 600;"
                "border-radius: 14px; border: none;"
            );

            QString displayName = m.name.isEmpty() ? m.email.split('@')[0] : m.name;
            if (!m.number.isEmpty()) {
                displayName += " (" + m.number + ")";
            }

            QLabel *nameLabel = new QLabel(displayName);
            nameLabel->setStyleSheet(QString(
                "font-size: 13px; font-weight: 500; color: %1; background: transparent; border: none;"
            ).arg(StyleConfig::TEXT_PRIMARY));

            QPushButton *removeBtn = new QPushButton("移除");
            removeBtn->setCursor(Qt::PointingHandCursor);
            removeBtn->setFixedSize(48, 24);
            removeBtn->setStyleSheet(
                "QPushButton { font-size: 11px; color: #9CA3AF; background: transparent;"
                "  border: 1px solid #E5E7EB; border-radius: 4px; }"
                "QPushButton:hover { color: #EF4444; border-color: #FCA5A5; background: #FEF2F2; }"
            );

            QString email = m.email;
            QString name = displayName;
            connect(removeBtn, &QPushButton::clicked, this, [this, email, name]() {
                QMessageBox::StandardButton ret = QMessageBox::question(
                    this, "确认移除",
                    QString("确定要将 \"%1\" 移出班级吗？").arg(name),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    ClassManager::instance()->removeMember(m_classInfo.id, email);
                }
            });

            row->addWidget(avatarLabel);
            row->addWidget(nameLabel);
            row->addStretch();
            row->addWidget(removeBtn);
            m_memberLayout->addWidget(rowFrame);
        }
    }

    m_memberLayout->addStretch();
}

void ClassDetailWidget::setClassInfo(const ClassInfo &info)
{
    m_classInfo = info;
    m_classNameLabel->setText(info.name);
    m_codeLabel->setText(info.code);
    m_codeCountLabel->setText(
        info.studentCount > 0
            ? QString("共 %1 名学生").arg(info.studentCount)
            : "暂无学生加入");
}

void ClassDetailWidget::showAttendanceActive(const AttendanceManager::SessionInfo &session)
{
    // 清理旧的考勤页面，保留 page 0（成员列表）
    while (m_rightStack->count() > 1) {
        QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w);
        w->deleteLater();
    }

    auto *widget = new AttendanceActiveWidget(session, m_cachedMembers.size());
    connect(widget, &AttendanceActiveWidget::attendanceFinished, this,
            [this](const QString &sessionId) {
        showAttendanceResult(sessionId);
    });

    m_rightStack->addWidget(widget);
    m_rightStack->setCurrentWidget(widget);
}

void ClassDetailWidget::showAttendanceResult(const QString &sessionId)
{
    // 清理旧页面，保留 page 0
    while (m_rightStack->count() > 1) {
        QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w);
        w->deleteLater();
    }

    auto *widget = new AttendanceResultWidget(sessionId);
    connect(widget, &AttendanceResultWidget::backToDetail, this, [this]() {
        showMemberList();
    });

    m_rightStack->addWidget(widget);
    m_rightStack->setCurrentWidget(widget);
}

void ClassDetailWidget::showMemberList()
{
    while (m_rightStack->count() > 1) {
        QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w);
        w->deleteLater();
    }
    m_rightStack->setCurrentIndex(0);
}

void ClassDetailWidget::showClassSettings()
{
    while (m_rightStack->count() > 1) {
        QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w);
        w->deleteLater();
    }

    auto *widget = new ClassSettingsWidget(m_classInfo);

    connect(widget, &ClassSettingsWidget::settingsSaved, this,
            [this](const ClassInfo &info) {
        m_classInfo = info;
        m_classNameLabel->setText(info.name);
        showMemberList();
    });

    connect(widget, &ClassSettingsWidget::deleteRequested, this,
            [this](const QString &classId) {
        // 监听删除成功后再返回
        connect(ClassManager::instance(), &ClassManager::classDeleted, this,
                [this](const QString &deletedId) {
            Q_UNUSED(deletedId)
            emit backRequested();
        });
        ClassManager::instance()->deleteClass(classId);
    });

    // 监听 ClassManager::classUpdated 更新本地数据
    connect(ClassManager::instance(), &ClassManager::classUpdated, this,
            [this](const ClassInfo &info) {
        if (info.id == m_classInfo.id) {
            m_classInfo = info;
            m_classNameLabel->setText(info.name);
        }
    });

    m_rightStack->addWidget(widget);
    m_rightStack->setCurrentWidget(widget);
}

void ClassDetailWidget::showHomeworkList()
{
    while (m_rightStack->count() > 1) {
        QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w);
        w->deleteLater();
    }

    auto *widget = new HomeworkListWidget(m_classInfo.id);

    connect(widget, &HomeworkListWidget::backRequested, this, [this]() {
        showMemberList();
    });

    connect(widget, &HomeworkListWidget::createRequested, this, [this]() {
        while (m_rightStack->count() > 1) {
            QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
            m_rightStack->removeWidget(w);
            w->deleteLater();
        }
        auto *createWidget = new HomeworkCreateWidget(m_classInfo.id, m_classInfo.teacherEmail);
        connect(createWidget, &HomeworkCreateWidget::backRequested, this, [this]() {
            showHomeworkList();
        });
        connect(createWidget, &HomeworkCreateWidget::created, this, [this]() {
            showHomeworkList();
        });
        m_rightStack->addWidget(createWidget);
        m_rightStack->setCurrentWidget(createWidget);
    });

    connect(widget, &HomeworkListWidget::viewSubmissions, this,
            [this](const HomeworkManager::AssignmentInfo &assignment) {
        while (m_rightStack->count() > 1) {
            QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
            m_rightStack->removeWidget(w);
            w->deleteLater();
        }
        auto *subWidget = new HomeworkSubmissionsWidget(assignment, m_cachedMembers);
        connect(subWidget, &HomeworkSubmissionsWidget::backRequested, this, [this]() {
            showHomeworkList();
        });
        m_rightStack->addWidget(subWidget);
        m_rightStack->setCurrentWidget(subWidget);
    });

    m_rightStack->addWidget(widget);
    m_rightStack->setCurrentWidget(widget);
}

void ClassDetailWidget::showMaterials()
{
    while (m_rightStack->count() > 1) {
        QWidget *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w);
        w->deleteLater();
    }

    auto *widget = new MaterialWidget(m_classInfo.id, m_classInfo.teacherEmail);

    connect(widget, &MaterialWidget::backRequested, this, [this]() {
        showMemberList();
    });

    m_rightStack->addWidget(widget);
    m_rightStack->setCurrentWidget(widget);
}
