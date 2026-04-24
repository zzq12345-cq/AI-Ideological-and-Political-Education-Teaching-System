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
    mainLayout->setContentsMargins(36, 32, 36, 32);
    mainLayout->setSpacing(24);

    // ── 顶部：返回 + 班级名 ──
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(16);

    QPushButton *backBtn = new QPushButton("← 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #9CA3AF;"
        "  font-size: 13px; font-weight: 500; padding: 4px; }"
        "QPushButton:hover { color: #4B5563; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &ClassDetailWidget::backRequested);

    m_classNameLabel = new QLabel(m_classInfo.name);
    m_classNameLabel->setStyleSheet("font-size: 24px; font-weight: 700; color: #111827;");

    // 状态徽标
    QLabel *statusBadge = new QLabel("● 活跃中");
    statusBadge->setStyleSheet(
        "font-size: 12px; color: #059669; font-weight: 500; padding-top: 4px;");

    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(m_classNameLabel);
    headerLayout->addSpacing(8);
    headerLayout->addWidget(statusBadge);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // ── 班级码区域 ──
    mainLayout->addWidget(createCodeSection());

    // ── 下半区：左侧导航 + 右侧内容 ──
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(24);
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
    section->setStyleSheet(
        "#codeSection { background: #F8FAFC; border: none; border-radius: 8px; }");

    QHBoxLayout *layout = new QHBoxLayout(section);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(32);

    // 左侧：加课码
    QVBoxLayout *codeCol = new QVBoxLayout();
    codeCol->setSpacing(4);

    QLabel *titleLabel = new QLabel("加课码");
    titleLabel->setStyleSheet("font-size: 12px; color: #6B7280; font-weight: 500;");

    m_codeLabel = new QLabel(m_classInfo.code);
    m_codeLabel->setStyleSheet(
        "font-size: 28px; font-weight: 700; color: #111827; letter-spacing: 4px;"
    );

    codeCol->addWidget(titleLabel);
    codeCol->addWidget(m_codeLabel);
    layout->addLayout(codeCol);

    // 竖分隔线
    QFrame *vLine = new QFrame();
    vLine->setFrameShape(QFrame::VLine);
    vLine->setStyleSheet("background: #E5E7EB; max-width: 1px; border: none; margin: 4px 0;");
    layout->addWidget(vLine);

    // 中间：学生数信息
    QVBoxLayout *infoCol = new QVBoxLayout();
    infoCol->setSpacing(4);
    QLabel *infoTitle = new QLabel("学生人数");
    infoTitle->setStyleSheet("font-size: 12px; color: #6B7280; font-weight: 500;");
    m_codeCountLabel = new QLabel("加载中...");
    m_codeCountLabel->setStyleSheet("font-size: 16px; color: #374151; font-weight: 600;");
    infoCol->addWidget(infoTitle);
    infoCol->addWidget(m_codeCountLabel);
    layout->addLayout(infoCol);

    layout->addStretch();

    // 右侧：操作按钮
    QVBoxLayout *btnCol = new QVBoxLayout();
    btnCol->setSpacing(8);

    QPushButton *copyBtn = new QPushButton("复制");
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setStyleSheet(
        "QPushButton { padding: 6px 16px; border: 1px solid #D1D5DB;"
        "  border-radius: 6px; background: white; color: #374151; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background: #F9FAFB; border-color: #9CA3AF; }"
    );
    connect(copyBtn, &QPushButton::clicked, this, [this, copyBtn]() {
        QApplication::clipboard()->setText(m_classInfo.code);
        copyBtn->setText("已复制 ✓");
        QTimer::singleShot(1500, this, [copyBtn]() { copyBtn->setText("复制"); });
    });

    QPushButton *refreshBtn = new QPushButton("刷新");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { padding: 6px 16px; border: 1px solid transparent;"
        "  border-radius: 6px; background: transparent; color: #6B7280; font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background: #F3F4F6; color: #374151; }"
    );
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

    btnCol->addWidget(copyBtn);
    btnCol->addWidget(refreshBtn);
    layout->addLayout(btnCol);

    return section;
}

QWidget* ClassDetailWidget::createActionButtons()
{
    QWidget *container = new QWidget();
    container->setObjectName("navPanel");
    container->setFixedWidth(200);
    container->setStyleSheet("#navPanel { background: transparent; }");
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setSpacing(2);
    layout->setContentsMargins(0, 0, 0, 0);

    struct Action { QString name; QString svgPath; };
    QList<Action> actions = {
        {"班级成员", ":/icons/resources/icons/users.svg"},
        {"考勤",     ":/icons/resources/icons/attendance.svg"},
        {"发布作业", ":/icons/resources/icons/document.svg"},
        {"课程资料", ":/icons/resources/icons/folder.svg"},
        {"课程设置", ":/icons/resources/icons/settings.svg"},
    };

    bool isFirst = true;
    for (const auto &act : actions) {
        QPushButton *btn = new QPushButton();
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(36);

        if (isFirst) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: #E5E7EB; border: none;"
                "  border-radius: 6px; padding: 4px 12px; text-align: left;"
                "}");
            isFirst = false;
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: transparent; border: none;"
                "  border-radius: 6px; padding: 4px 12px; text-align: left;"
                "}"
                "QPushButton:hover { background: #F3F4F6; }");
        }

        QHBoxLayout *row = new QHBoxLayout(btn);
        row->setSpacing(8);
        row->setContentsMargins(8, 0, 8, 0);

        QLabel *iconLabel = new QLabel();
        QIcon svgIcon(act.svgPath);
        iconLabel->setPixmap(svgIcon.pixmap(16, 16));
        iconLabel->setFixedSize(16, 16);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("background: transparent; border: none;");

        QLabel *nameLabel = new QLabel(act.name);
        nameLabel->setStyleSheet(QString(
            "font-size: 13px; font-weight: %1; color: %2; background: transparent; border: none;"
        ).arg(isFirst ? "600" : "500", isFirst ? "#111827" : "#4B5563"));

        row->addWidget(iconLabel);
        row->addWidget(nameLabel);
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
    m_memberSection->setStyleSheet(
        "#memberSection { background: white; border: 1px solid #E5E7EB; border-radius: 12px; }");

    m_memberLayout = new QVBoxLayout(m_memberSection);
    m_memberLayout->setContentsMargins(20, 16, 20, 16);
    m_memberLayout->setSpacing(0);

    // 标题行
    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("班级成员");
    titleLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #1E293B; background: transparent;");

    m_memberCountLabel = new QLabel("加载中...");
    m_memberCountLabel->setStyleSheet(
        "font-size: 12px; color: #94A3B8; background: transparent;");

    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(m_memberCountLabel);
    m_memberLayout->addLayout(headerRow);

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background: #F1F5F9; max-height: 1px; border: none;");
    m_memberLayout->addWidget(line);
    m_memberLayout->addSpacing(4);

    QLabel *placeholder = new QLabel("加载中...");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("font-size: 13px; color: #94A3B8; padding: 40px; background: transparent;");
    m_memberLayout->addWidget(placeholder);
    m_memberLayout->addStretch();

    return m_memberSection;
}

void ClassDetailWidget::updateMemberList(const QList<ClassManager::MemberInfo> &members)
{
    while (m_memberLayout->count() > 3) {
        QLayoutItem *item = m_memberLayout->takeAt(m_memberLayout->count() - 1);
        delete item->widget();
        delete item;
    }

    m_memberCountLabel->setText(QString("共 %1 人").arg(members.size()));

    if (members.isEmpty()) {
        QLabel *emptyLabel = new QLabel("暂无学生加入\n将加课码分享给学生即可邀请加入");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 40px;"
                                  "background: transparent; line-height: 1.6;");
        m_memberLayout->addWidget(emptyLabel);
    } else {
        for (int i = 0; i < members.size(); ++i) {
            const auto &m = members[i];
            auto *rowFrame = new QFrame();
            rowFrame->setObjectName(QString("memberRow_%1").arg(i));
            rowFrame->setStyleSheet(QString(
                "#memberRow_%1 { background: transparent; border: none;"
                "  border-bottom: 1px solid #F3F4F6; border-radius: 0; }"
                "#memberRow_%1:hover { background: #F8FAFC; }").arg(i));
            QHBoxLayout *row = new QHBoxLayout(rowFrame);
            row->setSpacing(12);
            row->setContentsMargins(8, 10, 8, 10);

            // 极简灰色序号圆
            QLabel *avatarLabel = new QLabel(QString::number(i + 1));
            avatarLabel->setFixedSize(28, 28);
            avatarLabel->setAlignment(Qt::AlignCenter);
            avatarLabel->setStyleSheet(
                "background: #F3F4F6; color: #4B5563; font-size: 12px; font-weight: 600;"
                "border-radius: 14px; border: none;");

            QString displayName = m.name.isEmpty() ? m.email.split('@')[0] : m.name;
            if (!m.number.isEmpty()) {
                displayName += " (" + m.number + ")";
            }

            QLabel *nameLabel = new QLabel(displayName);
            nameLabel->setStyleSheet(
                "font-size: 13px; font-weight: 500; color: #111827; background: transparent; border: none;");

            // 角色标签：线框/极简灰
            bool isTeacher = m.name.contains("老师") || m.name.contains("教师");
            QLabel *roleTag = new QLabel(isTeacher ? "教师" : "学生");
            roleTag->setStyleSheet(QString(
                "font-size: 11px; color: %1; background: transparent; padding: 2px 6px;"
                "border: 1px solid %2; border-radius: 4px; font-weight: 500;")
                .arg(isTeacher ? "#4B5563" : "#9CA3AF",
                     isTeacher ? "#D1D5DB" : "#E5E7EB"));

            QPushButton *removeBtn = new QPushButton("移除");
            removeBtn->setCursor(Qt::PointingHandCursor);
            removeBtn->setFixedSize(48, 26);
            removeBtn->setStyleSheet(
                "QPushButton { font-size: 11px; color: #9CA3AF; background: transparent;"
                "  border: none; border-radius: 4px; }"
                "QPushButton:hover { color: #EF4444; background: #FEF2F2; }");

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
            row->addWidget(roleTag);
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
