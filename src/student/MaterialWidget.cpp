#include "MaterialWidget.h"
#include "../shared/StyleConfig.h"
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QDesktopServices>
#include <QMenu>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

static const int GRID_COLS = 4;
static const int CARD_W = 160;
static const int CARD_H = 180;

MaterialWidget::MaterialWidget(const QString &classId, const QString &uploaderEmail, QWidget *parent)
    : QWidget(parent)
    , m_classId(classId)
    , m_uploaderEmail(uploaderEmail)
{
    setupUI();

    // 连接 MaterialManager 信号
    connect(MaterialManager::instance(), &MaterialManager::materialsLoaded, this,
            [this](const QString &folderId, const QList<MaterialManager::MaterialInfo> &list) {
        if (folderId != m_currentFolderId && !(folderId.isEmpty() && m_currentFolderId.isEmpty())) return;

        clearGrid();

        if (list.isEmpty()) {
            auto *empty = new QLabel("此文件夹为空\n点击上方按钮添加文件夹或上传文件");
            empty->setAlignment(Qt::AlignCenter);
            empty->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 60px; background: transparent;");
            m_gridLayout->addWidget(empty, 0, 0, 1, GRID_COLS);
            return;
        }

        for (int i = 0; i < list.size(); ++i) {
            int row = i / GRID_COLS;
            int col = i % GRID_COLS;
            m_gridLayout->addWidget(createMaterialCard(list[i]), row, col);
        }
    });

    connect(MaterialManager::instance(), &MaterialManager::folderCreated, this,
            [this](const MaterialManager::MaterialInfo &) { refreshCurrentFolder(); });
    connect(MaterialManager::instance(), &MaterialManager::fileUploaded, this,
            [this](const MaterialManager::MaterialInfo &) { refreshCurrentFolder(); });
    connect(MaterialManager::instance(), &MaterialManager::materialDeleted, this,
            [this](const QString &) { refreshCurrentFolder(); });
    connect(MaterialManager::instance(), &MaterialManager::error, this,
            [this](const QString &msg) { QMessageBox::warning(this, "操作失败", msg); });

    // 加载根目录
    loadFolder(QString());
}

void MaterialWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    // ── 顶部栏 ──
    auto *headerRow = new QHBoxLayout();

    auto *backBtn = new QPushButton("< 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #6B7280;"
        "  font-size: 14px; padding: 4px 8px; }"
        "QPushButton:hover { color: #E53935; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &MaterialWidget::backRequested);

    auto *title = new QLabel("课程资料");
    title->setStyleSheet(QString("font-size: 20px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    auto *uploadBtn = new QPushButton("上传文件");
    uploadBtn->setCursor(Qt::PointingHandCursor);
    uploadBtn->setFixedHeight(36);
    uploadBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: none; border-radius: 8px;"
        "  padding: 0 20px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(uploadBtn, &QPushButton::clicked, this, &MaterialWidget::onUploadFileClicked);

    auto *folderBtn = new QPushButton("+ 新建文件夹");
    folderBtn->setCursor(Qt::PointingHandCursor);
    folderBtn->setFixedHeight(36);
    folderBtn->setStyleSheet(QString(
        "QPushButton { background: white; color: %1; border: 1px solid %2; border-radius: 8px;"
        "  padding: 0 16px; font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #F9FAFB; border-color: #9CA3AF; }"
    ).arg(StyleConfig::TEXT_PRIMARY, StyleConfig::BORDER_LIGHT));
    connect(folderBtn, &QPushButton::clicked, this, &MaterialWidget::onCreateFolderClicked);

    headerRow->addWidget(backBtn);
    headerRow->addWidget(title);
    headerRow->addStretch();
    headerRow->addWidget(folderBtn);
    headerRow->addSpacing(8);
    headerRow->addWidget(uploadBtn);
    mainLayout->addLayout(headerRow);

    // ── 面包屑导航 ──
    m_breadcrumbBar = new QFrame();
    m_breadcrumbBar->setStyleSheet(QString(
        "QFrame { background: %1; border: 1px solid %2; border-radius: 8px; }"
    ).arg(StyleConfig::BG_CARD, StyleConfig::BORDER_LIGHT));
    m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbBar);
    m_breadcrumbLayout->setContentsMargins(12, 8, 12, 8);
    m_breadcrumbLayout->setSpacing(4);
    mainLayout->addWidget(m_breadcrumbBar);

    // ── 滚动网格 ──
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { width: 6px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #D1D5DB; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: #9CA3AF; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    m_gridContainer = new QWidget();
    m_gridContainer->setStyleSheet("background: transparent;");
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setContentsMargins(8, 8, 8, 8);
    m_gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_scrollArea->setWidget(m_gridContainer);
    mainLayout->addWidget(m_scrollArea, 1);
}

void MaterialWidget::loadFolder(const QString &folderId, const QString &folderName)
{
    m_currentFolderId = folderId;
    if (!folderName.isEmpty()) {
        m_breadcrumb.append({folderId, folderName});
    }
    updateBreadcrumbUI();
    MaterialManager::instance()->loadMaterials(m_classId, folderId);
}

void MaterialWidget::navigateToBreadcrumb(int index)
{
    if (index < 0) {
        m_breadcrumb.clear();
        m_currentFolderId.clear();
    } else {
        while (m_breadcrumb.size() > index + 1)
            m_breadcrumb.removeLast();
        m_currentFolderId = m_breadcrumb.isEmpty() ? QString() : m_breadcrumb.last().first;
    }
    updateBreadcrumbUI();
    MaterialManager::instance()->loadMaterials(m_classId, m_currentFolderId);
}

void MaterialWidget::refreshCurrentFolder()
{
    MaterialManager::instance()->loadMaterials(m_classId, m_currentFolderId);
}

void MaterialWidget::updateBreadcrumbUI()
{
    while (m_breadcrumbLayout->count()) {
        auto *item = m_breadcrumbLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    auto *rootLabel = new QLabel("📁 全部资料");
    rootLabel->setCursor(Qt::PointingHandCursor);
    rootLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #1A1A1A; background: transparent;");
    rootLabel->installEventFilter(new BreadcrumbClickFilter(-1,
        [this](int idx) { navigateToBreadcrumb(idx); }, this));
    m_breadcrumbLayout->addWidget(rootLabel);

    for (int i = 0; i < m_breadcrumb.size(); ++i) {
        auto *sep = new QLabel("›");
        sep->setStyleSheet("font-size: 14px; color: #9CA3AF; background: transparent;");
        m_breadcrumbLayout->addWidget(sep);

        bool isLast = (i == m_breadcrumb.size() - 1);
        auto *folderLabel = new QLabel("📁 " + m_breadcrumb[i].second);
        folderLabel->setCursor(Qt::PointingHandCursor);
        folderLabel->setStyleSheet(QString(
            "font-size: 14px; font-weight: %1; color: %2; background: transparent;"
        ).arg(isLast ? "600" : "500", isLast ? "#1A1A1A" : "#6B7280"));
        folderLabel->installEventFilter(new BreadcrumbClickFilter(i,
            [this](int idx) { navigateToBreadcrumb(idx); }, this));
        m_breadcrumbLayout->addWidget(folderLabel);
    }

    m_breadcrumbLayout->addStretch();
}

void MaterialWidget::clearGrid()
{
    QLayoutItem *child;
    while ((child = m_gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
}

QWidget* MaterialWidget::createMaterialCard(const MaterialManager::MaterialInfo &info)
{
    auto *card = new QFrame();
    card->setFixedSize(CARD_W, CARD_H);
    card->setCursor(Qt::PointingHandCursor);
    card->setObjectName("matCard");

    bool isFolder = (info.type == "folder");
    QString icon = getFileIcon(info.mimeType, info.type);
    QString bgColor = isFolder ? "#FFFBF0" : "#F5F7FF";

    card->setStyleSheet(QString(
        "#matCard { background: %1; border: 1px solid %2; border-radius: 12px; }"
        "#matCard:hover { border-color: %3; }"
    ).arg(bgColor, StyleConfig::BORDER_LIGHT, StyleConfig::PATRIOTIC_RED));

    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 10));
    shadow->setOffset(0, 2);
    card->setGraphicsEffect(shadow);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 20, 12, 8);
    layout->setSpacing(4);
    layout->setAlignment(Qt::AlignCenter);

    // 图标
    auto *iconLabel = new QLabel(icon);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 42px; background: transparent;");
    layout->addWidget(iconLabel);

    // 名称
    auto *nameLabel = new QLabel(info.name);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    nameLabel->setMaximumHeight(36);
    nameLabel->setStyleSheet(QString(
        "font-size: 13px; font-weight: 600; color: %1; background: transparent;"
    ).arg(StyleConfig::TEXT_PRIMARY));
    layout->addWidget(nameLabel);

    // 副标题
    auto *subLabel = new QLabel(isFolder ? "文件夹" : formatFileSize(info.fileSize));
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet("font-size: 11px; color: #9CA3AF; background: transparent;");
    layout->addWidget(subLabel);

    // 右上角删除按钮
    auto *deleteBtn = new QPushButton("×", card);
    deleteBtn->setFixedSize(22, 22);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->move(CARD_W - 30, 4);
    deleteBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #9CA3AF; border: none;"
        "  font-size: 15px; font-weight: bold; border-radius: 4px; }"
        "QPushButton:hover { background: #FEE2E2; color: #EF4444; }"
    );
    deleteBtn->raise();

    QString matId = info.id;
    QString matName = info.name;
    connect(deleteBtn, &QPushButton::clicked, this, [this, matId, matName, card]() {
        auto ret = QMessageBox::question(card, "确认删除",
            QString("确定删除 \"%1\" 吗？").arg(matName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            MaterialManager::instance()->deleteMaterial(matId);
        }
    });

    // 卡片点击
    QString folderId = info.id;
    QString folderName = info.name;
    QString fileUrl = info.fileUrl;
    card->installEventFilter(new CardClickFilter(
        isFolder, folderId, folderName, fileUrl,
        [this, folderId, folderName]() { loadFolder(folderId, folderName); },
        [](const QString &url) { QDesktopServices::openUrl(QUrl(url)); },
        this
    ));

    return card;
}

void MaterialWidget::onCreateFolderClicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "新建文件夹",
        "文件夹名称:", QLineEdit::Normal, "新建文件夹", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        MaterialManager::instance()->createFolder(m_classId, m_currentFolderId, name.trimmed(), m_uploaderEmail);
    }
}

void MaterialWidget::onUploadFileClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    QMenu menu;
    auto *fileAction = menu.addAction("选择文件");
    auto *folderAction = menu.addAction("选择文件夹");

    QPoint pos = btn ? btn->mapToGlobal(QPoint(0, btn->height())) : QCursor::pos();
    QAction *selected = menu.exec(pos);
    if (!selected) return;

    QMimeDatabase mimeDb;

    if (selected == fileAction) {
        QStringList files = QFileDialog::getOpenFileNames(this, "选择文件");
        if (files.isEmpty()) return;
        for (const auto &filePath : files) {
            qint64 size = QFileInfo(filePath).size();
            if (size > 50 * 1024 * 1024) {
                QMessageBox::warning(this, "文件过大",
                    QString("文件 \"%1\" 大小为 %2MB，超过 50MB 上传限制。\n"
                            "大文件建议压缩后上传或使用外部链接。")
                    .arg(QFileInfo(filePath).fileName())
                    .arg(size / (1024.0 * 1024.0), 0, 'f', 1));
                continue;
            }
            QString mimeType = mimeDb.mimeTypeForFile(filePath).name();
            MaterialManager::instance()->uploadFile(m_classId, m_currentFolderId, filePath, mimeType, m_uploaderEmail);
        }
    } else if (selected == folderAction) {
        QString dirPath = QFileDialog::getExistingDirectory(this, "选择文件夹");
        if (dirPath.isEmpty()) return;

        QDir rootDir(dirPath);
        QString rootName = rootDir.dirName();

        // 1. 收集所有子目录和文件
        QSet<QString> allDirs;
        QMap<QString, QStringList> dirToFiles;
        QDirIterator it(dirPath, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            QString path = it.next();
            QFileInfo fi(path);
            if (fi.isDir()) {
                allDirs.insert(rootDir.relativeFilePath(path));
            } else if (fi.isFile()) {
                QString relDir = rootDir.relativeFilePath(fi.absolutePath());
                dirToFiles[relDir].append(path);
            }
        }

        // 按深度排序
        QStringList sortedDirs = allDirs.values();
        std::sort(sortedDirs.begin(), sortedDirs.end(), [](const QString &a, const QString &b) {
            return a.count('/') < b.count('/');
        });

        // 2. 先创建用户选中的根文件夹，等成功后再逐级创建子目录
        auto *pathToId = new QMap<QString, QString>();

        connect(MaterialManager::instance(), &MaterialManager::folderCreated, this,
            [this, sortedDirs, dirToFiles, pathToId](const MaterialManager::MaterialInfo &rootInfo) {
            // 根文件夹创建成功，映射 "" → 根ID
            pathToId->insert("", rootInfo.id);

            // 上传根目录里的文件
            QMimeDatabase mimeDb;
            for (const auto &fp : dirToFiles.value("")) {
                if (QFileInfo(fp).size() > 50 * 1024 * 1024) {
                    qWarning() << "[Material] 跳过大文件:" << fp << QFileInfo(fp).size() / (1024*1024) << "MB";
                    continue;
                }
                QString mt = mimeDb.mimeTypeForFile(fp).name();
                MaterialManager::instance()->uploadFile(m_classId, rootInfo.id, fp, mt, m_uploaderEmail);
            }

            // 逐级创建子文件夹
            auto *createNext = new std::function<void(int)>();
            *createNext = [this, sortedDirs, dirToFiles, pathToId, createNext](int idx) mutable {
                if (idx >= sortedDirs.size()) {
                    delete createNext;
                    delete pathToId;
                    return;
                }
                QString relPath = sortedDirs[idx];
                QString parentRel = relPath.contains('/') ? relPath.left(relPath.lastIndexOf('/')) : "";
                QString parentFolderId = pathToId->value(parentRel);
                QString folderName = relPath.contains('/') ? relPath.mid(relPath.lastIndexOf('/') + 1) : relPath;

                connect(MaterialManager::instance(), &MaterialManager::folderCreated, this,
                    [this, idx, relPath, dirToFiles, pathToId, createNext](const MaterialManager::MaterialInfo &info) {
                    pathToId->insert(relPath, info.id);

                    QMimeDatabase mimeDb;
                    for (const auto &fp : dirToFiles.value(relPath)) {
                        if (QFileInfo(fp).size() > 50 * 1024 * 1024) {
                            qWarning() << "[Material] 跳过大文件:" << fp << QFileInfo(fp).size() / (1024*1024) << "MB";
                            continue;
                        }
                        QString mt = mimeDb.mimeTypeForFile(fp).name();
                        MaterialManager::instance()->uploadFile(m_classId, info.id, fp, mt, m_uploaderEmail);
                    }

                    (*createNext)(idx + 1);
                }, Qt::SingleShotConnection);

                MaterialManager::instance()->createFolder(m_classId, parentFolderId, folderName, m_uploaderEmail);
            };

            if (!sortedDirs.isEmpty()) {
                (*createNext)(0);
            } else {
                delete pathToId;
            }
        }, Qt::SingleShotConnection);

        // 创建根文件夹（选中的那个文件夹）
        MaterialManager::instance()->createFolder(m_classId, m_currentFolderId, rootName, m_uploaderEmail);
    }
}

QString MaterialWidget::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024) return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

QString MaterialWidget::getFileIcon(const QString &mimeType, const QString &type) const
{
    if (type == "folder") return "📁";
    if (mimeType.startsWith("image/")) return "🖼";
    if (mimeType.startsWith("video/")) return "🎬";
    if (mimeType.startsWith("audio/")) return "🎵";
    if (mimeType.contains("pdf")) return "📕";
    if (mimeType.contains("word") || mimeType.contains("document")) return "📘";
    if (mimeType.contains("sheet") || mimeType.contains("excel")) return "📊";
    if (mimeType.contains("presentation") || mimeType.contains("powerpoint")) return "📙";
    if (mimeType.contains("zip") || mimeType.contains("rar") || mimeType.contains("archive")) return "📦";
    if (mimeType.startsWith("text/")) return "📄";
    return "📄";
}
