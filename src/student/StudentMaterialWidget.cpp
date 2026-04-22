#include "StudentMaterialWidget.h"
#include "../shared/StyleConfig.h"
#include <QDesktopServices>
#include <QDebug>

StudentMaterialWidget::StudentMaterialWidget(const QString &classId, QWidget *parent)
    : QWidget(parent), m_classId(classId)
{
    setupUI();

    connect(MaterialManager::instance(), &MaterialManager::materialsLoaded, this,
        [this](const QString &folderId, const QList<MaterialManager::MaterialInfo> &list) {
        if (folderId != m_currentFolderId && !(folderId.isEmpty() && m_currentFolderId.isEmpty())) return;
        clearList();
        if (list.isEmpty()) {
            auto *empty = new QLabel("此文件夹为空");
            empty->setAlignment(Qt::AlignCenter);
            empty->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 60px; background: transparent;");
            m_listLayout->addWidget(empty);
            return;
        }
        for (const auto &info : list) {
            m_listLayout->addWidget(createMaterialRow(info));
        }
    });

    loadFolder(QString());
}

void StudentMaterialWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    // 顶部
    auto *headerRow = new QHBoxLayout();
    auto *backBtn = new QPushButton("< 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet("QPushButton { background: transparent; border: none; color: #6B7280; font-size: 14px; padding: 4px 8px; } QPushButton:hover { color: #E53935; }");
    connect(backBtn, &QPushButton::clicked, this, &StudentMaterialWidget::backRequested);

    auto *title = new QLabel("课程资料");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    headerRow->addWidget(backBtn);
    headerRow->addWidget(title);
    headerRow->addStretch();
    mainLayout->addLayout(headerRow);

    // 面包屑
    m_breadcrumbBar = new QFrame();
    m_breadcrumbBar->setStyleSheet(QString("QFrame { background: %1; border: 1px solid %2; border-radius: 8px; }").arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));
    m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbBar);
    m_breadcrumbLayout->setContentsMargins(12, 8, 12, 8);
    m_breadcrumbLayout->setSpacing(4);
    mainLayout->addWidget(m_breadcrumbBar);

    // 列表
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #D1D5DB; border-radius: 3px; min-height: 30px; }");
    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background: transparent;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setSpacing(4);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setAlignment(Qt::AlignTop);
    m_scrollArea->setWidget(m_listContainer);
    mainLayout->addWidget(m_scrollArea, 1);
}

void StudentMaterialWidget::loadFolder(const QString &folderId, const QString &folderName)
{
    m_currentFolderId = folderId;
    if (!folderName.isEmpty()) m_breadcrumb.append({folderId, folderName});
    updateBreadcrumbUI();
    MaterialManager::instance()->loadMaterials(m_classId, folderId);
}

void StudentMaterialWidget::navigateToBreadcrumb(int index)
{
    if (index < 0) { m_breadcrumb.clear(); m_currentFolderId.clear(); }
    else { while (m_breadcrumb.size() > index + 1) m_breadcrumb.removeLast(); m_currentFolderId = m_breadcrumb.isEmpty() ? QString() : m_breadcrumb.last().first; }
    updateBreadcrumbUI();
    MaterialManager::instance()->loadMaterials(m_classId, m_currentFolderId);
}

void StudentMaterialWidget::refreshCurrentFolder()
{
    MaterialManager::instance()->loadMaterials(m_classId, m_currentFolderId);
}

void StudentMaterialWidget::updateBreadcrumbUI()
{
    while (m_breadcrumbLayout->count()) { auto *item = m_breadcrumbLayout->takeAt(0); delete item->widget(); delete item; }
    auto *root = new QLabel("📁 全部资料");
    root->setCursor(Qt::PointingHandCursor);
    root->setStyleSheet("font-size: 14px; font-weight: 600; color: #1A1A1A; background: transparent;");
    root->installEventFilter(new StudentBreadcrumbFilter(-1, [this](int idx) { navigateToBreadcrumb(idx); }, this));
    m_breadcrumbLayout->addWidget(root);
    for (int i = 0; i < m_breadcrumb.size(); ++i) {
        auto *sep = new QLabel("›"); sep->setStyleSheet("font-size: 14px; color: #9CA3AF; background: transparent;"); m_breadcrumbLayout->addWidget(sep);
        bool isLast = (i == m_breadcrumb.size() - 1);
        auto *lbl = new QLabel("📁 " + m_breadcrumb[i].second);
        lbl->setCursor(Qt::PointingHandCursor);
        lbl->setStyleSheet(QString("font-size: 14px; font-weight: %1; color: %2; background: transparent;").arg(isLast ? "600" : "500", isLast ? "#1A1A1A" : "#6B7280"));
        lbl->installEventFilter(new StudentBreadcrumbFilter(i, [this](int idx) { navigateToBreadcrumb(idx); }, this));
        m_breadcrumbLayout->addWidget(lbl);
    }
    m_breadcrumbLayout->addStretch();
}

void StudentMaterialWidget::clearList()
{
    QLayoutItem *child;
    while ((child = m_listLayout->takeAt(0)) != nullptr) { delete child->widget(); delete child; }
}

QWidget* StudentMaterialWidget::createMaterialRow(const MaterialManager::MaterialInfo &info)
{
    bool isFolder = (info.type == "folder");
    QString icon = getFileIcon(info.mimeType, info.type);

    auto *row = new QFrame();
    row->setObjectName("sMatRow");
    row->setCursor(Qt::PointingHandCursor);
    row->setFixedHeight(48);
    row->setStyleSheet(QString(
        "#sMatRow { background: transparent; border: none; border-bottom: 1px solid %1; }"
        "#sMatRow:hover { background: %2; }"
    ).arg(StyleConfig::BORDER_LIGHT, StyleConfig::PATRIOTIC_RED_LIGHT));

    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 4, 12, 4);
    layout->setSpacing(10);

    auto *iconLabel = new QLabel(icon);
    iconLabel->setFixedSize(28, 28);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    layout->addWidget(iconLabel);

    auto *nameLabel = new QLabel(info.name);
    nameLabel->setStyleSheet(QString("font-size: 14px; font-weight: 500; color: %1; background: transparent;").arg(StyleConfig::TEXT_PRIMARY));
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // 长文件名省略
    QFontMetrics fm(nameLabel->font());
    int maxTextWidth = 400;
    QString elidedText = fm.elidedText(info.name, Qt::ElideMiddle, maxTextWidth);
    nameLabel->setText(elidedText);
    if (elidedText != info.name) nameLabel->setToolTip(info.name);
    layout->addWidget(nameLabel, 1);

    // 右侧文件大小或"文件夹"标签
    auto *sizeLabel = new QLabel(isFolder ? "" : formatFileSize(info.fileSize));
    sizeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sizeLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
    sizeLabel->setFixedWidth(80);
    layout->addWidget(sizeLabel);

    // 点击导航
    QString folderId = info.id, folderName = info.name, fileUrl = info.fileUrl;
    row->installEventFilter(new StudentRowFilter(isFolder, folderId, folderName, fileUrl,
        [this, folderId, folderName]() { loadFolder(folderId, folderName); },
        [](const QString &url) { QDesktopServices::openUrl(QUrl(url)); }, this));

    return row;
}

QString StudentMaterialWidget::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
}

QString StudentMaterialWidget::getFileIcon(const QString &mimeType, const QString &type) const
{
    if (type == "folder") return "📁";
    if (mimeType.startsWith("image/")) return "🖼";
    if (mimeType.startsWith("video/")) return "🎬";
    if (mimeType.contains("pdf")) return "📕";
    if (mimeType.contains("word") || mimeType.contains("document")) return "📘";
    if (mimeType.contains("sheet") || mimeType.contains("excel")) return "📊";
    if (mimeType.contains("presentation") || mimeType.contains("powerpoint")) return "📙";
    return "📄";
}
