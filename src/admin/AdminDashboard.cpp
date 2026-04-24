#include "AdminDashboard.h"
#include "AdminManager.h"
#include "../shared/StyleConfig.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QScrollArea>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include "../shared/ModernDialogHelper.h"
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QClipboard>
#include <QApplication>

namespace {
QString cardSS() {
    return QString(
        "QFrame { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "QFrame:hover { border-color: %3; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT, StyleConfig::PATRIOTIC_RED);
}
}

AdminDashboard::AdminDashboard(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void AdminDashboard::setupUI()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── 左侧导航 ──
    auto *sidebar = new QFrame();
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet(QString(
        "QFrame { background: %1; border-right: 1px solid %2; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));

    auto *sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(16, 24, 16, 16);
    sideLayout->setSpacing(8);

    auto *logoLabel = new QLabel("管理后台");
    logoLabel->setStyleSheet(QString("font-size: 18px; font-weight: 700; color: %1; padding: 0 0 16px 0;")
        .arg(StyleConfig::PATRIOTIC_RED));
    sideLayout->addWidget(logoLabel);

    struct NavItem { QString name; int page; };
    QList<NavItem> navItems = {
        {"数据总览", 0},
        {"学校管理", 1},
        {"教师管理", 2},
        {"班级管理", 3},
    };

    for (const auto &item : navItems) {
        auto *btn = new QPushButton(item.name);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(44);
        btn->setStyleSheet(QString(
            "QPushButton { background: transparent; border: none; border-radius: 8px;"
            "  text-align: left; padding: 0 16px; font-size: 14px; font-weight: 500; color: %1; }"
            "QPushButton:hover { background: #FFF5F5; color: %2; }"
        ).arg(StyleConfig::TEXT_PRIMARY, StyleConfig::PATRIOTIC_RED));
        int page = item.page;
        connect(btn, &QPushButton::clicked, this, [this, page]() {
            m_stack->setCurrentIndex(page);
            if (page == 0) refreshOverview();
            else if (page == 1) refreshSchools();
            else if (page == 2) refreshTeachers();
            else if (page == 3) refreshClasses();
        });
        sideLayout->addWidget(btn);
    }
    sideLayout->addStretch();

    mainLayout->addWidget(sidebar);

    // ── 右侧内容 ──
    m_stack = new QStackedWidget();
    m_stack->addWidget(createOverviewPage());  // 0
    m_stack->addWidget(createSchoolPage());    // 1
    m_stack->addWidget(createTeacherPage());   // 2
    m_stack->addWidget(createClassPage());     // 3
    mainLayout->addWidget(m_stack, 1);

    // 初始加载总览
    refreshOverview();
}

// ══════════════════════════════════════
//  总览页
// ══════════════════════════════════════

QWidget* AdminDashboard::createOverviewPage()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(20);

    auto *title = new QLabel("数据总览");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    layout->addWidget(title);

    // 统计卡片行
    auto *cardsRow = new QHBoxLayout();
    cardsRow->setSpacing(16);

    auto mkCard = [](const QString &label, const QString &color, QLabel *&valueLabel) {
        auto *card = new QFrame();
        card->setStyleSheet(QString(
            "QFrame { background: white; border: 1px solid #E5E7EB; border-radius: 12px; border-left: 4px solid %1; }"
        ).arg(color));
        auto *l = new QVBoxLayout(card);
        l->setContentsMargins(20, 16, 20, 16);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent; border: none;");
        valueLabel = new QLabel("-");
        valueLabel->setStyleSheet(QString("font-size: 28px; font-weight: 700; color: %1; background: transparent; border: none;").arg(color));
        l->addWidget(lbl);
        l->addWidget(valueLabel);
        return card;
    };

    cardsRow->addWidget(mkCard("学校总数", "#2563EB", m_statSchools));
    cardsRow->addWidget(mkCard("教师总数", "#059669", m_statTeachers));
    cardsRow->addWidget(mkCard("班级总数", "#D97706", m_statClasses));
    cardsRow->addWidget(mkCard("学生总数", StyleConfig::PATRIOTIC_RED, m_statStudents));
    layout->addLayout(cardsRow);
    layout->addStretch();

    scroll->setWidget(page);
    return scroll;
}

void AdminDashboard::refreshOverview()
{
    connect(AdminManager::instance(), &AdminManager::overviewStatsLoaded, this,
        [this](int schools, int teachers, int classes, int students) {
        m_statSchools->setText(QString::number(schools));
        m_statTeachers->setText(QString::number(teachers));
        m_statClasses->setText(QString::number(classes));
        m_statStudents->setText(QString::number(students));
    }, Qt::SingleShotConnection);
    AdminManager::instance()->loadOverviewStats();
}

// ══════════════════════════════════════
//  学校管理页
// ══════════════════════════════════════

QWidget* AdminDashboard::createSchoolPage()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(16);

    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("学校管理");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    m_schoolCountLabel = new QLabel();
    m_schoolCountLabel->setStyleSheet("font-size: 13px; color: #9CA3AF;");
    auto *addBtn = new QPushButton("+ 创建学校");
    addBtn->setCursor(Qt::PointingHandCursor);
    addBtn->setFixedHeight(36);
    addBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  padding: 0 20px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(addBtn, &QPushButton::clicked, this, &AdminDashboard::showCreateSchoolDialog);

    headerRow->addWidget(title);
    headerRow->addWidget(m_schoolCountLabel);
    headerRow->addStretch();
    headerRow->addWidget(addBtn);
    layout->addLayout(headerRow);

    auto *listContainer = new QWidget();
    listContainer->setStyleSheet("background: transparent;");
    m_schoolListLayout = new QVBoxLayout(listContainer);
    m_schoolListLayout->setContentsMargins(0, 0, 0, 0);
    m_schoolListLayout->setSpacing(12);
    m_schoolListLayout->setAlignment(Qt::AlignTop);
    layout->addWidget(listContainer, 1);

    scroll->setWidget(page);
    return scroll;
}

QWidget* AdminDashboard::createTeacherPage()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(16);

    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("教师管理");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    m_teacherCountLabel = new QLabel();
    m_teacherCountLabel->setStyleSheet("font-size: 13px; color: #9CA3AF;");
    headerRow->addWidget(title);
    headerRow->addWidget(m_teacherCountLabel);
    headerRow->addStretch();
    layout->addLayout(headerRow);

    auto *listContainer = new QWidget();
    listContainer->setStyleSheet("background: transparent;");
    m_teacherListLayout = new QVBoxLayout(listContainer);
    m_teacherListLayout->setContentsMargins(0, 0, 0, 0);
    m_teacherListLayout->setSpacing(8);
    m_teacherListLayout->setAlignment(Qt::AlignTop);
    layout->addWidget(listContainer, 1);

    scroll->setWidget(page);
    return scroll;
}

QWidget* AdminDashboard::createClassPage()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *page = new QWidget();
    page->setStyleSheet("background: transparent;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(16);

    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("班级管理");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    m_classCountLabel = new QLabel();
    m_classCountLabel->setStyleSheet("font-size: 13px; color: #9CA3AF;");
    headerRow->addWidget(title);
    headerRow->addWidget(m_classCountLabel);
    headerRow->addStretch();
    layout->addLayout(headerRow);

    auto *listContainer = new QWidget();
    listContainer->setStyleSheet("background: transparent;");
    m_classListLayout = new QVBoxLayout(listContainer);
    m_classListLayout->setContentsMargins(0, 0, 0, 0);
    m_classListLayout->setSpacing(8);
    m_classListLayout->setAlignment(Qt::AlignTop);
    layout->addWidget(listContainer, 1);

    scroll->setWidget(page);
    return scroll;
}

// ══════════════════════════════════════
//  学校列表
// ══════════════════════════════════════

void AdminDashboard::refreshSchools()
{
    connect(AdminManager::instance(), &AdminManager::schoolsLoaded, this,
        [this](const QList<AdminManager::SchoolInfo> &schools) {
        while (m_schoolListLayout->count()) {
            auto *item = m_schoolListLayout->takeAt(0);
            delete item->widget(); delete item;
        }

        m_schoolCountLabel->setText(QString("共 %1 所学校").arg(schools.size()));

        // 缓存学校列表
        m_schoolList.clear();
        for (const auto &s : schools) m_schoolList.append({s.id, s.name});

        if (schools.isEmpty()) {
            auto *empty = new QLabel("暂无学校，点击右上角创建");
            empty->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 40px;");
            empty->setAlignment(Qt::AlignCenter);
            m_schoolListLayout->addWidget(empty);
            return;
        }

        for (const auto &s : schools) {
            auto *card = new QFrame();
            card->setObjectName("schoolCard");
            card->setStyleSheet(cardSS());
            auto *cardLayout = new QVBoxLayout(card);
            cardLayout->setContentsMargins(20, 16, 20, 16);
            cardLayout->setSpacing(8);

            auto *nameRow = new QHBoxLayout();
            auto *nameL = new QLabel(s.name);
            nameL->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1; background: transparent; border: none;")
                .arg(StyleConfig::TEXT_PRIMARY));
            nameRow->addWidget(nameL);
            nameRow->addStretch();

            auto mkBtn = [](const QString &text, int w, const QString &hoverColor) {
                auto *b = new QPushButton(text);
                b->setCursor(Qt::PointingHandCursor);
                b->setFixedSize(w, 28);
                b->setStyleSheet(
                    "QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 6px;"
                    "  font-size: 12px; color: #6B7280; }"
                    "QPushButton:hover { border-color: " + hoverColor + "; color: " + hoverColor + "; }");
                return b;
            };

            auto *editBtn = mkBtn("编辑", 48, "#E53935");
            QString sId = s.id, sName = s.name; int quota = s.classQuota;
            connect(editBtn, &QPushButton::clicked, this, [this, sId, sName, quota]() {
                showEditSchoolDialog(sId, sName, quota);
            });
            nameRow->addWidget(editBtn);

            auto *inviteBtn = mkBtn("邀请码", 60, "#2563EB");
            connect(inviteBtn, &QPushButton::clicked, this, [this, sId, sName]() {
                showInviteCodeDialog(sId, sName);
            });
            nameRow->addWidget(inviteBtn);

            auto *delBtn = mkBtn("删除", 48, "#EF4444");
            connect(delBtn, &QPushButton::clicked, this, [this, sId, sName]() {
                if (ModernDialogHelper::confirm(this, "确认删除",
                    QString("确定删除学校 \"%1\" 吗？").arg(sName))) {
                    connect(AdminManager::instance(), &AdminManager::schoolDeleted, this,
                        [this](const QString &) { refreshSchools(); }, Qt::SingleShotConnection);
                    AdminManager::instance()->deleteSchool(sId);
                }
            });
            nameRow->addWidget(delBtn);

            cardLayout->addLayout(nameRow);

            auto *statRow = new QHBoxLayout();
            auto *quotaL = new QLabel(QString("班级名额: %1").arg(s.classQuota));
            quotaL->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent; border: none;");
            statRow->addWidget(quotaL);
            auto *dot = new QLabel("·");
            dot->setStyleSheet("font-size: 13px; color: #D1D5DB; background: transparent; border: none;");
            statRow->addWidget(dot);
            auto *dateL = new QLabel(s.createdAt.toString("yyyy-MM-dd"));
            dateL->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
            statRow->addWidget(dateL);
            statRow->addStretch();
            cardLayout->addLayout(statRow);

            m_schoolListLayout->addWidget(card);
        }
        m_schoolListLayout->addStretch();
    }, Qt::SingleShotConnection);

    AdminManager::instance()->loadSchools();
}

void AdminDashboard::showCreateSchoolDialog()
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("创建学校");
    dialog->setMinimumWidth(380);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 20, 24, 24);

    layout->addWidget(new QLabel("学校名称:"));
    auto *nameEdit = new QTextEdit();
    nameEdit->setFixedHeight(36);
    nameEdit->setPlaceholderText("输入学校名称");
    nameEdit->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    layout->addWidget(nameEdit);

    layout->addWidget(new QLabel("班级名额:"));
    auto *quotaSpin = new QSpinBox();
    quotaSpin->setRange(1, 999); quotaSpin->setValue(5); quotaSpin->setSuffix(" 个");
    quotaSpin->setFixedHeight(36);
    layout->addWidget(quotaSpin);

    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px;");
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    auto *okBtn = new QPushButton("创建");
    okBtn->setFixedHeight(36);
    okBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 0 20px; font-weight: 600; } QPushButton:hover { background: #C62828; }").arg(StyleConfig::PATRIOTIC_RED));
    connect(okBtn, &QPushButton::clicked, this, [this, dialog, nameEdit, quotaSpin]() {
        QString name = nameEdit->toPlainText().trimmed();
        if (name.isEmpty()) return;
        dialog->accept();
        connect(AdminManager::instance(), &AdminManager::schoolCreated, this,
            [this](const AdminManager::SchoolInfo &) { refreshSchools(); }, Qt::SingleShotConnection);
        connect(AdminManager::instance(), &AdminManager::error, this,
            [](const QString &msg) { qWarning() << "[Admin] create school error:" << msg; }, Qt::SingleShotConnection);
        AdminManager::instance()->createSchool(name, quotaSpin->value());
    });
    btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);
    dialog->exec();
    dialog->deleteLater();
}

void AdminDashboard::showEditSchoolDialog(const QString &id, const QString &name, int quota)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("编辑学校");
    dialog->setMinimumWidth(380);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 20, 24, 24);

    layout->addWidget(new QLabel("学校名称:"));
    auto *nameEdit = new QTextEdit();
    nameEdit->setFixedHeight(36); nameEdit->setPlainText(name);
    nameEdit->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    layout->addWidget(nameEdit);

    layout->addWidget(new QLabel("班级名额:"));
    auto *quotaSpin = new QSpinBox();
    quotaSpin->setRange(1, 999); quotaSpin->setValue(quota); quotaSpin->setSuffix(" 个");
    quotaSpin->setFixedHeight(36);
    layout->addWidget(quotaSpin);

    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px;");
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    auto *okBtn = new QPushButton("保存");
    okBtn->setFixedHeight(36);
    okBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 0 20px; font-weight: 600; } QPushButton:hover { background: #C62828; }").arg(StyleConfig::PATRIOTIC_RED));
    connect(okBtn, &QPushButton::clicked, this, [this, dialog, id, nameEdit, quotaSpin]() {
        QString n = nameEdit->toPlainText().trimmed();
        if (n.isEmpty()) return;
        dialog->accept();
        connect(AdminManager::instance(), &AdminManager::schoolUpdated, this,
            [this]() { refreshSchools(); }, Qt::SingleShotConnection);
        AdminManager::instance()->updateSchool(id, n, quotaSpin->value());
    });
    btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);
    dialog->exec();
    dialog->deleteLater();
}

void AdminDashboard::showInviteCodeDialog(const QString &schoolId, const QString &schoolName)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("邀请码管理 - " + schoolName);
    dialog->setMinimumSize(500, 400);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(12);
    layout->setContentsMargins(24, 20, 24, 24);

    auto *headerRow = new QHBoxLayout();
    auto *title = new QLabel("邀请码列表");
    title->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));
    auto *genBtn = new QPushButton("+ 生成邀请码");
    genBtn->setCursor(Qt::PointingHandCursor);
    genBtn->setFixedHeight(32);
    genBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; border-radius: 6px; padding: 0 16px; font-size: 12px; font-weight: 600; } QPushButton:hover { background: #C62828; }").arg(StyleConfig::PATRIOTIC_RED));
    connect(genBtn, &QPushButton::clicked, this, [this, schoolId, dialog]() {
        bool ok; int count = QInputDialog::getInt(dialog, "生成邀请码", "生成数量:", 1, 1, 50, 1, &ok);
        if (!ok) return;
        connect(AdminManager::instance(), &AdminManager::inviteCodeGenerated, this,
            [this, schoolId](const QStringList &) { showInviteCodeDialog(schoolId, ""); }, Qt::SingleShotConnection);
        AdminManager::instance()->generateInviteCode(schoolId, count);
    });
    headerRow->addWidget(title); headerRow->addStretch(); headerRow->addWidget(genBtn);
    layout->addLayout(headerRow);

    auto *codeListWidget = new QWidget();
    auto *codeListLayout = new QVBoxLayout(codeListWidget);
    codeListLayout->setContentsMargins(0, 0, 0, 0);
    codeListLayout->setSpacing(6);
    codeListLayout->setAlignment(Qt::AlignTop);
    layout->addWidget(codeListWidget, 1);

    connect(AdminManager::instance(), &AdminManager::inviteCodesLoaded, this,
        [codeListLayout, schoolId, this](const QList<AdminManager::InviteCodeInfo> &codes) {
        while (codeListLayout->count()) { auto *i = codeListLayout->takeAt(0); delete i->widget(); delete i; }
        if (codes.isEmpty()) {
            auto *e = new QLabel("暂无邀请码，点击右上角生成"); e->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 20px;");
            e->setAlignment(Qt::AlignCenter); codeListLayout->addWidget(e);
        }
        for (const auto &c : codes) {
            auto *row = new QFrame();
            row->setStyleSheet("QFrame { background: #F9FAFB; border: 1px solid #E5E7EB; border-radius: 8px; }");
            auto *rl = new QHBoxLayout(row); rl->setContentsMargins(12, 8, 12, 8);
            auto *codeL = new QLabel(c.code);
            codeL->setStyleSheet("font-size: 15px; font-weight: 700; font-family: monospace; color: #1A1A1A; background: transparent; border: none;");
            rl->addWidget(codeL);
            auto *stL = new QLabel(c.used ? "已使用" : "未使用");
            stL->setFixedWidth(60); stL->setAlignment(Qt::AlignCenter);
            stL->setStyleSheet(c.used ? "font-size: 11px; color: #9CA3AF; background: #F3F4F6; border-radius: 4px; padding: 2px 8px;"
                                       : "font-size: 11px; color: #059669; background: #D1FAE5; border-radius: 4px; padding: 2px 8px;");
            rl->addWidget(stL);
            if (c.used && !c.usedByEmail.isEmpty()) {
                auto *em = new QLabel(c.usedByEmail); em->setStyleSheet("font-size: 11px; color: #6B7280; background: transparent; border: none;");
                rl->addWidget(em);
            }
            rl->addStretch();
            if (!c.used) {
                auto *copyBtn = new QPushButton("复制"); copyBtn->setCursor(Qt::PointingHandCursor); copyBtn->setFixedSize(40, 24);
                copyBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #6B7280; } QPushButton:hover { border-color: #2563EB; color: #2563EB; }");
                QString code = c.code;
                connect(copyBtn, &QPushButton::clicked, copyBtn, [code]() { QApplication::clipboard()->setText(code); });
                rl->addWidget(copyBtn);

                QString codeId = c.id;
                auto *delBtn = new QPushButton("删除"); delBtn->setCursor(Qt::PointingHandCursor); delBtn->setFixedSize(40, 24);
                delBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #9CA3AF; } QPushButton:hover { border-color: #EF4444; color: #EF4444; }");
                connect(delBtn, &QPushButton::clicked, this, [this, schoolId, codeId]() {
                    connect(AdminManager::instance(), &AdminManager::inviteCodeDeleted, this,
                        [this, schoolId](const QString &) { showInviteCodeDialog(schoolId, ""); }, Qt::SingleShotConnection);
                    AdminManager::instance()->deleteInviteCode(codeId);
                });
                rl->addWidget(delBtn);
            }
            codeListLayout->addWidget(row);
        }
        codeListLayout->addStretch();
    }, Qt::SingleShotConnection);
    AdminManager::instance()->loadInviteCodes(schoolId);
    dialog->exec();
    dialog->deleteLater();
}

// ══════════════════════════════════════
//  教师管理
// ══════════════════════════════════════

void AdminDashboard::refreshTeachers()
{
    // 加载教师列表
    connect(AdminManager::instance(), &AdminManager::teachersLoaded, this,
        [this](const QList<AdminManager::TeacherInfo> &teachers) {
        while (m_teacherListLayout->count()) { auto *i = m_teacherListLayout->takeAt(0); delete i->widget(); delete i; }
        m_teacherCountLabel->setText(QString("共 %1 名教师").arg(teachers.size()));
        if (teachers.isEmpty()) {
            auto *e = new QLabel("暂无教师注册"); e->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 40px;"); e->setAlignment(Qt::AlignCenter);
            m_teacherListLayout->addWidget(e); return;
        }
        for (const auto &t : teachers) {
            auto *row = new QFrame(); row->setObjectName("tRow");
            row->setStyleSheet("#tRow { background: white; border: 1px solid #E5E7EB; border-radius: 10px; } #tRow:hover { border-color: #E53935; }");
            auto *rl = new QHBoxLayout(row); rl->setContentsMargins(16, 12, 16, 12);

            auto *nameL = new QLabel(t.email.split('@')[0]);
            nameL->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));
            rl->addWidget(nameL);
            auto *emailL = new QLabel(t.email);
            emailL->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
            rl->addWidget(emailL);

            // 学校标签
            QString schoolName;
            for (const auto &p : m_schoolList) { if (p.first == t.schoolId) { schoolName = p.second; break; } }
            auto *schL = new QLabel(schoolName.isEmpty() ? "未绑定学校" : schoolName);
            schL->setStyleSheet("font-size: 12px; color: #6B7280; background: #F3F4F6; border-radius: 4px; padding: 2px 10px; border: none;");
            rl->addWidget(schL);
            rl->addStretch();

            // 分配学校按钮
            auto *assignBtn = new QPushButton("分配学校"); assignBtn->setCursor(Qt::PointingHandCursor); assignBtn->setFixedSize(64, 24);
            assignBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #6B7280; } QPushButton:hover { border-color: #2563EB; color: #2563EB; }");
            QString email = t.email;
            connect(assignBtn, &QPushButton::clicked, this, [this, email]() { showAssignTeacherDialog(email); });
            rl->addWidget(assignBtn);

            // 重置密码按钮
            auto *pwdBtn = new QPushButton("重置密码"); pwdBtn->setCursor(Qt::PointingHandCursor); pwdBtn->setFixedSize(60, 24);
            pwdBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #6B7280; } QPushButton:hover { border-color: #F59E0B; color: #F59E0B; }");
            connect(pwdBtn, &QPushButton::clicked, this, [this, email]() { showResetPasswordDialog(email); });
            rl->addWidget(pwdBtn);

            // 删除按钮
            auto *delBtn = new QPushButton("删除"); delBtn->setCursor(Qt::PointingHandCursor); delBtn->setFixedSize(40, 24);
            delBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #9CA3AF; } QPushButton:hover { border-color: #EF4444; color: #EF4444; background: #FEF2F2; }");
            connect(delBtn, &QPushButton::clicked, this, [this, email]() {
                if (ModernDialogHelper::confirm(this, "确认", QString("确定删除教师 %1？").arg(email))) {
                    connect(AdminManager::instance(), &AdminManager::teacherDeleted, this,
                        [this](const QString &) { refreshTeachers(); }, Qt::SingleShotConnection);
                    AdminManager::instance()->deleteTeacher(email);
                }
            });
            rl->addWidget(delBtn);

            m_teacherListLayout->addWidget(row);
        }
        m_teacherListLayout->addStretch();
    }, Qt::SingleShotConnection);

    // 先加载学校列表缓存，再加载教师
    connect(AdminManager::instance(), &AdminManager::schoolsLoaded, this,
        [this](const QList<AdminManager::SchoolInfo> &schools) {
        m_schoolList.clear();
        for (const auto &s : schools) m_schoolList.append({s.id, s.name});
        AdminManager::instance()->loadTeachers();
    }, Qt::SingleShotConnection);
    AdminManager::instance()->loadSchools();
}

void AdminDashboard::showAssignTeacherDialog(const QString &email)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("分配教师到学校 - " + email);
    dialog->setMinimumWidth(350);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 20, 24, 24);

    layout->addWidget(new QLabel("选择学校:"));
    auto *combo = new QComboBox();
    combo->setFixedHeight(36);
    combo->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    for (const auto &p : m_schoolList) combo->addItem(p.second, p.first);
    layout->addWidget(combo);

    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消"); cancelBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px;");
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    auto *okBtn = new QPushButton("确认"); okBtn->setFixedHeight(36);
    okBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 0 20px; font-weight: 600; } QPushButton:hover { background: #C62828; }").arg(StyleConfig::PATRIOTIC_RED));
    connect(okBtn, &QPushButton::clicked, this, [this, dialog, email, combo]() {
        QString schoolId = combo->currentData().toString();
        dialog->accept();
        connect(AdminManager::instance(), &AdminManager::teacherUpdated, this,
            [this]() { refreshTeachers(); }, Qt::SingleShotConnection);
        AdminManager::instance()->assignTeacherToSchool(email, schoolId);
    });
    btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);
    dialog->exec();
    dialog->deleteLater();
}

// ══════════════════════════════════════
//  班级管理
// ══════════════════════════════════════

void AdminDashboard::refreshClasses()
{
    connect(AdminManager::instance(), &AdminManager::allClassesLoaded, this,
        [this](const QList<AdminManager::ClassBrief> &classes) {
        while (m_classListLayout->count()) { auto *i = m_classListLayout->takeAt(0); delete i->widget(); delete i; }
        m_classCountLabel->setText(QString("共 %1 个班级").arg(classes.size()));
        if (classes.isEmpty()) {
            auto *e = new QLabel("暂无班级"); e->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 40px;"); e->setAlignment(Qt::AlignCenter);
            m_classListLayout->addWidget(e); return;
        }
        for (const auto &c : classes) {
            auto *row = new QFrame(); row->setObjectName("cRow");
            row->setStyleSheet("#cRow { background: white; border: 1px solid #E5E7EB; border-radius: 10px; } #cRow:hover { border-color: #E53935; }");
            auto *rl = new QHBoxLayout(row); rl->setContentsMargins(16, 12, 16, 12);

            auto *nameL = new QLabel(c.name);
            nameL->setStyleSheet(QString("font-size: 14px; font-weight: 600; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));
            rl->addWidget(nameL);
            auto *teacherL = new QLabel(c.teacherName);
            teacherL->setStyleSheet("font-size: 12px; color: #6B7280; background: transparent; border: none;");
            rl->addWidget(teacherL);

            QString schoolName;
            for (const auto &p : m_schoolList) { if (p.first == c.schoolId) { schoolName = p.second; break; } }
            auto *schL = new QLabel(schoolName.isEmpty() ? "未分配学校" : schoolName);
            schL->setStyleSheet("font-size: 12px; color: #6B7280; background: #F3F4F6; border-radius: 4px; padding: 2px 10px; border: none;");
            rl->addWidget(schL);
            rl->addStretch();

            // 编辑按钮
            auto *editBtn = new QPushButton("编辑"); editBtn->setCursor(Qt::PointingHandCursor); editBtn->setFixedSize(40, 24);
            editBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #6B7280; } QPushButton:hover { border-color: #2563EB; color: #2563EB; }");
            QString cId = c.id, cName = c.name, cDesc = c.description, cStatus = c.status;
            connect(editBtn, &QPushButton::clicked, this, [this, cId, cName, cDesc, cStatus]() { showEditClassDialog(cId, cName, cDesc, cStatus); });
            rl->addWidget(editBtn);

            // 删除
            QString classId = c.id; QString className = c.name;
            auto *delBtn = new QPushButton("删除"); delBtn->setCursor(Qt::PointingHandCursor); delBtn->setFixedSize(40, 24);
            delBtn->setStyleSheet("QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px; font-size: 11px; color: #9CA3AF; } QPushButton:hover { border-color: #EF4444; color: #EF4444; background: #FEF2F2; }");
            connect(delBtn, &QPushButton::clicked, this, [this, classId, className]() {
                if (ModernDialogHelper::confirm(this, "确认", QString("确定删除班级 \"%1\"？").arg(className))) {
                    connect(AdminManager::instance(), &AdminManager::classDeleted, this,
                        [this](const QString &) { refreshClasses(); }, Qt::SingleShotConnection);
                    AdminManager::instance()->deleteClass(classId);
                }
            });
            rl->addWidget(delBtn);

            m_classListLayout->addWidget(row);
        }
        m_classListLayout->addStretch();
    }, Qt::SingleShotConnection);

    // 先加载学校列表缓存，再加载班级
    connect(AdminManager::instance(), &AdminManager::schoolsLoaded, this,
        [this](const QList<AdminManager::SchoolInfo> &schools) {
        m_schoolList.clear();
        for (const auto &s : schools) m_schoolList.append({s.id, s.name});
        AdminManager::instance()->loadAllClasses();
    }, Qt::SingleShotConnection);
    AdminManager::instance()->loadSchools();
}

void AdminDashboard::showResetPasswordDialog(const QString &email)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("重置密码 - " + email);
    dialog->setMinimumWidth(350);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 20, 24, 24);

    layout->addWidget(new QLabel(QString("为 %1 设置新密码：").arg(email)));
    auto *pwdEdit = new QLineEdit();
    pwdEdit->setPlaceholderText("请输入新密码（至少6位）");
    pwdEdit->setEchoMode(QLineEdit::Password);
    pwdEdit->setFixedHeight(36);
    pwdEdit->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    layout->addWidget(pwdEdit);

    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消"); cancelBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px;");
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    auto *okBtn = new QPushButton("确认重置"); okBtn->setFixedHeight(36);
    okBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 0 20px; font-weight: 600; } QPushButton:hover { background: #C62828; }").arg(StyleConfig::PATRIOTIC_RED));
    connect(okBtn, &QPushButton::clicked, this, [this, dialog, email, pwdEdit]() {
        QString pwd = pwdEdit->text().trimmed();
        if (pwd.length() < 6) {
            ModernDialogHelper::warning(this, "提示", "密码至少6位");
            return;
        }
        dialog->accept();
        connect(AdminManager::instance(), &AdminManager::passwordReset, this,
            [this, email](const QString &e) {
            ModernDialogHelper::info(this, "成功", QString("已重置 %1 的密码").arg(e));
            refreshTeachers();
        }, Qt::SingleShotConnection);
        connect(AdminManager::instance(), &AdminManager::error, this,
            [this](const QString &msg) { ModernDialogHelper::warning(this, "失败", msg); }, Qt::SingleShotConnection);
        AdminManager::instance()->resetTeacherPassword(email, pwd);
    });
    btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);
    dialog->exec();
    dialog->deleteLater();
}

void AdminDashboard::showEditClassDialog(const QString &classId, const QString &name, const QString &description, const QString &status)
{
    auto *dialog = new QDialog(this);
    dialog->setWindowTitle("编辑班级 - " + name);
    dialog->setMinimumWidth(400);
    dialog->setStyleSheet("QDialog { background: white; }");

    auto *layout = new QVBoxLayout(dialog);
    layout->setSpacing(16);
    layout->setContentsMargins(24, 20, 24, 24);

    // 班级名称
    layout->addWidget(new QLabel("班级名称:"));
    auto *nameEdit = new QLineEdit(name);
    nameEdit->setFixedHeight(36);
    nameEdit->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    layout->addWidget(nameEdit);

    // 描述
    layout->addWidget(new QLabel("描述:"));
    auto *descEdit = new QTextEdit();
    descEdit->setPlainText(description);
    descEdit->setFixedHeight(80);
    descEdit->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    layout->addWidget(descEdit);

    // 状态
    layout->addWidget(new QLabel("状态:"));
    auto *statusCombo = new QComboBox();
    statusCombo->addItem("活跃", "active");
    statusCombo->addItem("已归档", "archived");
    statusCombo->setFixedHeight(36);
    statusCombo->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; padding: 6px 12px; font-size: 14px;");
    if (status == "archived") statusCombo->setCurrentIndex(1);
    layout->addWidget(statusCombo);

    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消"); cancelBtn->setFixedHeight(36);
    cancelBtn->setStyleSheet("border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px;");
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    auto *okBtn = new QPushButton("保存"); okBtn->setFixedHeight(36);
    okBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; border-radius: 8px; padding: 0 20px; font-weight: 600; } QPushButton:hover { background: #C62828; }").arg(StyleConfig::PATRIOTIC_RED));
    connect(okBtn, &QPushButton::clicked, this, [this, dialog, classId, nameEdit, descEdit, statusCombo]() {
        dialog->accept();
        connect(AdminManager::instance(), &AdminManager::classUpdated, this,
            [this]() { refreshClasses(); }, Qt::SingleShotConnection);
        AdminManager::instance()->updateClass(classId, nameEdit->text().trimmed(),
            descEdit->toPlainText().trimmed(), statusCombo->currentData().toString());
    });
    btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
    layout->addLayout(btnRow);
    dialog->exec();
    dialog->deleteLater();
}
