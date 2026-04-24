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
#include <QDialog>
#include <QLineEdit>

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
    // 统一页面背景色，与 ModernMainWindow 保持一致
    setStyleSheet("ClassDetailWidget { background: #F5F7FA; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 28, 40, 28);
    mainLayout->setSpacing(20);

    // ── 顶部：返回 + 班级名 ──
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    QPushButton *backBtn = new QPushButton("← 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #9CA3AF;"
        "  font-size: 13px; font-weight: 500; padding: 6px 10px; border-radius: 8px; }"
        "QPushButton:hover { color: #374151; background: rgba(0,0,0,0.04); }"
    );
    connect(backBtn, &QPushButton::clicked, this, &ClassDetailWidget::backRequested);

    m_classNameLabel = new QLabel(m_classInfo.name);
    m_classNameLabel->setStyleSheet(
        "font-size: 22px; font-weight: 700; color: #0F172A; letter-spacing: -0.3px;");

    // 状态徽标 — 使用柔和绿底+深绿字
    QLabel *statusBadge = new QLabel("● 活跃中");
    statusBadge->setStyleSheet(
        "font-size: 11px; color: #059669; font-weight: 600;"
        "background: rgba(5,150,105,0.08); padding: 3px 10px; border-radius: 10px;");

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
    section->setStyleSheet(
        "#codeSection { background: #FFFFFF; border: 1px solid #F0F0F0; border-radius: 16px; }");

    // 不使用 QGraphicsDropShadowEffect，避免阴影渲染到子控件文字上

    QHBoxLayout *layout = new QHBoxLayout(section);
    layout->setContentsMargins(28, 22, 28, 22);
    layout->setSpacing(40);

    // 左侧：加课码
    QVBoxLayout *codeCol = new QVBoxLayout();
    codeCol->setSpacing(6);

    QLabel *titleLabel = new QLabel("加课码");
    titleLabel->setStyleSheet(
        "font-size: 11px; color: #94A3B8; font-weight: 600;"
        "text-transform: uppercase; letter-spacing: 1px; background: transparent;");

    m_codeLabel = new QLabel(m_classInfo.code);
    m_codeLabel->setStyleSheet(
        "font-size: 30px; font-weight: 800; color: #0F172A;"
        "letter-spacing: 5px; background: transparent;"
    );

    codeCol->addWidget(titleLabel);
    codeCol->addWidget(m_codeLabel);
    layout->addLayout(codeCol);

    // 竖分隔线 — 更柔和
    QFrame *vLine = new QFrame();
    vLine->setFrameShape(QFrame::VLine);
    vLine->setStyleSheet("background: #F1F5F9; max-width: 1px; border: none;");
    layout->addWidget(vLine);

    // 中间：学生数信息
    QVBoxLayout *infoCol = new QVBoxLayout();
    infoCol->setSpacing(6);
    QLabel *infoTitle = new QLabel("学生人数");
    infoTitle->setStyleSheet(
        "font-size: 11px; color: #94A3B8; font-weight: 600;"
        "text-transform: uppercase; letter-spacing: 1px; background: transparent;");
    m_codeCountLabel = new QLabel("加载中...");
    m_codeCountLabel->setStyleSheet(
        "font-size: 18px; color: #1E293B; font-weight: 700; background: transparent;");
    infoCol->addWidget(infoTitle);
    infoCol->addWidget(m_codeCountLabel);
    layout->addLayout(infoCol);

    layout->addStretch();

    // 右侧：操作按钮
    QVBoxLayout *btnCol = new QVBoxLayout();
    btnCol->setSpacing(6);

    QPushButton *copyBtn = new QPushButton("复制");
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setStyleSheet(
        "QPushButton { padding: 7px 20px; border: none;"
        "  border-radius: 10px; background: #F1F5F9; color: #374151;"
        "  font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #E2E8F0; color: #0F172A; }"
        "QPushButton:pressed { background: #CBD5E1; }"
    );
    connect(copyBtn, &QPushButton::clicked, this, [this, copyBtn]() {
        QApplication::clipboard()->setText(m_classInfo.code);
        copyBtn->setText("已复制 ✓");
        QTimer::singleShot(1500, this, [copyBtn]() { copyBtn->setText("复制"); });
    });

    QPushButton *refreshBtn = new QPushButton("刷新");
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(
        "QPushButton { padding: 7px 20px; border: none;"
        "  border-radius: 10px; background: transparent; color: #94A3B8;"
        "  font-size: 13px; font-weight: 500; }"
        "QPushButton:hover { background: #F8FAFC; color: #475569; }"
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
    container->setFixedWidth(240);
    container->setStyleSheet("#navPanel { background: transparent; }");
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 4, 0, 0);

    struct Action { QString name; QString svgPath; };
    QList<Action> actions = {
        {"班级成员", ":/icons/resources/icons/users.svg"},
        {"考勤",     ":/icons/resources/icons/attendance.svg"},
        {"发布作业", ":/icons/resources/icons/document.svg"},
        {"课程资料", ":/icons/resources/icons/folder.svg"},
        {"课程设置", ":/icons/resources/icons/settings.svg"},
    };

    // 导航按钮样式
    const QString navActiveStyle =
        "QPushButton {"
        "  background: rgba(99,102,241,0.08); border: none;"
        "  border-radius: 12px; padding: 6px 16px; text-align: left;"
        "}";
    const QString navInactiveStyle =
        "QPushButton {"
        "  background: transparent; border: none;"
        "  border-radius: 12px; padding: 6px 16px; text-align: left;"
        "}"
        "QPushButton:hover { background: rgba(0,0,0,0.04); }";

    m_navButtons.clear();

    for (int idx = 0; idx < actions.size(); ++idx) {
        const auto &act = actions[idx];
        QPushButton *btn = new QPushButton();
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(48);

        bool isActive = (idx == 0);
        btn->setStyleSheet(isActive ? navActiveStyle : navInactiveStyle);

        QHBoxLayout *row = new QHBoxLayout(btn);
        row->setSpacing(12);
        row->setContentsMargins(16, 0, 16, 0);

        QLabel *iconLabel = new QLabel();
        QIcon svgIcon(act.svgPath);
        iconLabel->setPixmap(svgIcon.pixmap(20, 20));
        iconLabel->setFixedSize(20, 20);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("background: transparent; border: none;");

        QLabel *nameLabel = new QLabel(act.name);
        nameLabel->setObjectName("navLabel");
        nameLabel->setStyleSheet(QString(
            "font-size: 15px; font-weight: %1; color: %2;"
            "background: transparent; border: none;"
        ).arg(isActive ? "600" : "500",
              isActive ? "#4F46E5" : "#64748B"));

        row->addWidget(iconLabel);
        row->addWidget(nameLabel);
        row->addStretch();

        m_navButtons.append(btn);

        QString actName = act.name;
        int navIdx = idx;
        connect(btn, &QPushButton::clicked, this, [this, actName, navIdx]() {
            setActiveNavButton(navIdx);
            if (actName == "考勤") {
                // ── 现代风格自定义弹窗 ──
                QDialog *dlg = new QDialog(this);
                dlg->setWindowTitle("开始考勤");
                dlg->setFixedSize(560, 320);
                dlg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
                dlg->setAttribute(Qt::WA_TranslucentBackground);

                // 外层容器（承载圆角和阴影）
                QFrame *card = new QFrame(dlg);
                card->setObjectName("attDlgCard");
                card->setStyleSheet(
                    "#attDlgCard { background: #FFFFFF; border-radius: 20px; }");
                card->setGeometry(16, 16, 528, 288);

                auto *dlgShadow = new QGraphicsDropShadowEffect(card);
                dlgShadow->setBlurRadius(40);
                dlgShadow->setOffset(0, 10);
                dlgShadow->setColor(QColor(15, 23, 42, 50));
                card->setGraphicsEffect(dlgShadow);

                QVBoxLayout *cardLayout = new QVBoxLayout(card);
                cardLayout->setContentsMargins(32, 28, 32, 24);
                cardLayout->setSpacing(16);

                QLabel *dlgTitle = new QLabel("开始考勤");
                dlgTitle->setStyleSheet(
                    "font-size: 18px; font-weight: 700; color: #0F172A;");

                QLabel *dlgHint = new QLabel("请输入此次考勤名称（如：第3周周一）");
                dlgHint->setStyleSheet(
                    "font-size: 13px; color: #64748B; font-weight: 400;");

                QLineEdit *dlgInput = new QLineEdit();
                dlgInput->setText(QDate::currentDate().toString("MM月dd日考勤"));
                dlgInput->selectAll();
                dlgInput->setStyleSheet(
                    "QLineEdit { padding: 10px 14px; border: 1.5px solid #E2E8F0;"
                    "  border-radius: 12px; font-size: 14px; color: #1E293B;"
                    "  background: #F8FAFC; }"
                    "QLineEdit:focus { border-color: #6366F1; background: #FFFFFF; }");

                QHBoxLayout *btnRow = new QHBoxLayout();
                btnRow->setSpacing(10);
                btnRow->addStretch();

                QPushButton *cancelBtn = new QPushButton("取消");
                cancelBtn->setCursor(Qt::PointingHandCursor);
                cancelBtn->setFixedSize(90, 38);
                cancelBtn->setStyleSheet(
                    "QPushButton { background: #F1F5F9; color: #64748B; border: none;"
                    "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
                    "QPushButton:hover { background: #E2E8F0; color: #475569; }");
                connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

                QPushButton *okBtn = new QPushButton("确定");
                okBtn->setCursor(Qt::PointingHandCursor);
                okBtn->setFixedSize(90, 38);
                okBtn->setStyleSheet(
                    "QPushButton { background: #6366F1; color: white; border: none;"
                    "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
                    "QPushButton:hover { background: #4F46E5; }"
                    "QPushButton:pressed { background: #4338CA; }");
                connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);
                connect(dlgInput, &QLineEdit::returnPressed, dlg, &QDialog::accept);

                btnRow->addWidget(cancelBtn);
                btnRow->addWidget(okBtn);

                cardLayout->addWidget(dlgTitle);
                cardLayout->addWidget(dlgHint);
                cardLayout->addWidget(dlgInput);
                cardLayout->addLayout(btnRow);

                if (dlg->exec() == QDialog::Accepted) {
                    QString attName = dlgInput->text().trimmed();
                    if (!attName.isEmpty()) {
                        AttendanceManager::instance()->startAttendance(m_classInfo.id, attName);
                    }
                }
                dlg->deleteLater();
            } else if (actName == "课程设置") {
                showClassSettings();
            } else if (actName == "发布作业") {
                showHomeworkList();
            } else if (actName == "班级成员") {
                showMemberList();
            } else if (actName == "课程资料") {
                showMaterials();
            } else {
                showModernInfo(this, actName, actName + "功能开发中...");
            }
        });

        layout->addWidget(btn);
    }

    layout->addStretch();
    return container;
}

void ClassDetailWidget::setActiveNavButton(int index)
{
    const QString navActiveStyle =
        "QPushButton {"
        "  background: rgba(99,102,241,0.08); border: none;"
        "  border-radius: 12px; padding: 6px 16px; text-align: left;"
        "}";
    const QString navInactiveStyle =
        "QPushButton {"
        "  background: transparent; border: none;"
        "  border-radius: 12px; padding: 6px 16px; text-align: left;"
        "}"
        "QPushButton:hover { background: rgba(0,0,0,0.04); }";

    for (int i = 0; i < m_navButtons.size(); ++i) {
        bool active = (i == index);
        m_navButtons[i]->setStyleSheet(active ? navActiveStyle : navInactiveStyle);
        // 更新文字标签样式
        QLabel *label = m_navButtons[i]->findChild<QLabel*>("navLabel");
        if (label) {
            label->setStyleSheet(QString(
                "font-size: 15px; font-weight: %1; color: %2;"
                "background: transparent; border: none;"
            ).arg(active ? "600" : "500",
                  active ? "#4F46E5" : "#64748B"));
        }
    }
}

bool ClassDetailWidget::showModernConfirm(QWidget *parent, const QString &title, const QString &message)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setFixedSize(480, 260);
    dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg.setAttribute(Qt::WA_TranslucentBackground);

    QFrame *card = new QFrame(&dlg);
    card->setObjectName("confirmCard");
    card->setStyleSheet("#confirmCard { background: #FFFFFF; border-radius: 20px; }");
    card->setGeometry(16, 16, 448, 228);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(15, 23, 42, 50));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(14);

    // 警告图标 + 标题
    QHBoxLayout *titleRow = new QHBoxLayout();
    QLabel *icon = new QLabel("\u26a0\ufe0f");
    icon->setStyleSheet("font-size: 22px; background: transparent;");
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");
    titleRow->addWidget(icon);
    titleRow->addSpacing(6);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    QLabel *msgLabel = new QLabel(message);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.5;");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);
    btnRow->addStretch();

    QPushButton *cancelBtn = new QPushButton("\u53d6\u6d88");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setFixedSize(100, 40);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #F1F5F9; color: #64748B; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #E2E8F0; color: #475569; }");
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    QPushButton *okBtn = new QPushButton("\u786e\u5b9a");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #EF4444; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #DC2626; }"
        "QPushButton:pressed { background: #B91C1C; }");
    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);

    layout->addLayout(titleRow);
    layout->addWidget(msgLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    return dlg.exec() == QDialog::Accepted;
}

void ClassDetailWidget::showModernInfo(QWidget *parent, const QString &title, const QString &message)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setFixedSize(440, 230);
    dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg.setAttribute(Qt::WA_TranslucentBackground);

    QFrame *card = new QFrame(&dlg);
    card->setObjectName("infoCard");
    card->setStyleSheet("#infoCard { background: #FFFFFF; border-radius: 20px; }");
    card->setGeometry(16, 16, 408, 198);

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(15, 23, 42, 50));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(36, 32, 36, 28);
    layout->setSpacing(14);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 700; color: #0F172A;");

    QLabel *msgLabel = new QLabel(message);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("font-size: 14px; color: #475569; line-height: 1.5;");

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    QPushButton *okBtn = new QPushButton("\u77e5\u9053\u4e86");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setFixedSize(100, 40);
    okBtn->setStyleSheet(
        "QPushButton { background: #6366F1; color: white; border: none;"
        "  border-radius: 10px; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #4F46E5; }"
        "QPushButton:pressed { background: #4338CA; }");
    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    btnRow->addWidget(okBtn);

    layout->addWidget(titleLabel);
    layout->addWidget(msgLabel);
    layout->addStretch();
    layout->addLayout(btnRow);

    dlg.exec();
}

QWidget* ClassDetailWidget::createMemberList()
{
    m_memberSection = new QFrame();
    m_memberSection->setObjectName("memberSection");
    m_memberSection->setStyleSheet(
        "#memberSection { background: #FFFFFF; border: 1px solid #F0F0F0; border-radius: 16px; }");

    // 不使用 QGraphicsDropShadowEffect，避免阴影渲染到子控件文字上

    m_memberLayout = new QVBoxLayout(m_memberSection);
    m_memberLayout->setContentsMargins(24, 20, 24, 20);
    m_memberLayout->setSpacing(0);

    // 标题行
    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("班级成员");
    titleLabel->setStyleSheet(
        "font-size: 15px; font-weight: 700; color: #0F172A; background: transparent;");

    m_memberCountLabel = new QLabel("加载中...");
    m_memberCountLabel->setStyleSheet(
        "font-size: 12px; color: #94A3B8; font-weight: 500; background: transparent;");

    headerRow->addWidget(titleLabel);
    headerRow->addStretch();
    headerRow->addWidget(m_memberCountLabel);
    m_memberLayout->addLayout(headerRow);

    // 分隔线
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background: #F1F5F9; max-height: 1px; border: none;");
    m_memberLayout->addWidget(line);
    m_memberLayout->addSpacing(8);

    QLabel *placeholder = new QLabel("加载中...");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet(
        "font-size: 13px; color: #94A3B8; padding: 40px; background: transparent;");
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
        emptyLabel->setStyleSheet("font-size: 13px; color: #94A3B8; padding: 40px;"
                                  "background: transparent; line-height: 1.6;");
        m_memberLayout->addWidget(emptyLabel);
    } else {
        for (int i = 0; i < members.size(); ++i) {
            const auto &m = members[i];
            auto *rowFrame = new QFrame();
            rowFrame->setObjectName(QString("memberRow_%1").arg(i));
            rowFrame->setStyleSheet(QString(
                "#memberRow_%1 { background: transparent; border: none;"
                "  border-radius: 10px; }"
                "#memberRow_%1:hover { background: #F8FAFC; }").arg(i));
            QHBoxLayout *row = new QHBoxLayout(rowFrame);
            row->setSpacing(14);
            row->setContentsMargins(10, 10, 10, 10);

            // 序号圆圈 — 柔和底色 + 微立体感
            QLabel *avatarLabel = new QLabel(QString::number(i + 1));
            avatarLabel->setFixedSize(32, 32);
            avatarLabel->setAlignment(Qt::AlignCenter);
            avatarLabel->setStyleSheet(
                "background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
                "stop:0 #F8FAFC, stop:1 #EEF2F7);"
                "color: #475569; font-size: 12px; font-weight: 700;"
                "border-radius: 16px; border: none;");

            QString displayName = m.name.isEmpty() ? m.email.split('@')[0] : m.name;
            if (!m.number.isEmpty()) {
                displayName += " (" + m.number + ")";
            }

            QLabel *nameLabel = new QLabel(displayName);
            nameLabel->setStyleSheet(
                "font-size: 13px; font-weight: 600; color: #1E293B;"
                "background: transparent; border: none;");

            // 角色标签 — 半透明主题色底色，无边框
            bool isTeacher = m.name.contains("老师") || m.name.contains("教师");
            QLabel *roleTag = new QLabel(isTeacher ? "教师" : "学生");
            roleTag->setStyleSheet(QString(
                "font-size: 11px; color: %1; background: %2; padding: 3px 10px;"
                "border: none; border-radius: 8px; font-weight: 600;")
                .arg(isTeacher ? "#B91C1C" : "#475569",
                     isTeacher ? "rgba(185,28,28,0.06)" : "rgba(71,85,105,0.06)"));

            QPushButton *removeBtn = new QPushButton("移除");
            removeBtn->setCursor(Qt::PointingHandCursor);
            removeBtn->setFixedSize(52, 28);
            removeBtn->setStyleSheet(
                "QPushButton { font-size: 11px; color: #CBD5E1; background: transparent;"
                "  border: none; border-radius: 6px; font-weight: 500; }"
                "QPushButton:hover { color: #EF4444; background: rgba(239,68,68,0.06); }");

            QString email = m.email;
            QString name = displayName;
            connect(removeBtn, &QPushButton::clicked, this, [this, email, name]() {
                bool confirmed = showModernConfirm(
                    this, "确认移除",
                    QString("确定要将 \"%1\" 移出班级吗？").arg(name));
                if (confirmed) {
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
