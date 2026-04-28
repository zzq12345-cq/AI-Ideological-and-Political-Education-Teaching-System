#include "InAppPPTPreviewPage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSize>
#include <QStyle>
#include <QVBoxLayout>
#include <QtGlobal>

namespace {
const int ThumbnailWidth = 132;
const int ThumbnailHeight = 74;
const int ButtonMinHeight = 44;
}

InAppPPTPreviewPage::InAppPPTPreviewPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
    setFocusPolicy(Qt::StrongFocus);

    auto bindShortcut = [this](const QKeySequence &key, auto handler) {
        auto *shortcut = new QShortcut(key, this);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, &QShortcut::activated, this, handler);
    };
    bindShortcut(QKeySequence(Qt::Key_Escape), [this]() { emit exitRequested(); });
    bindShortcut(QKeySequence(Qt::Key_Left), [this]() { showPreviousSlide(); });
    bindShortcut(QKeySequence(Qt::Key_Right), [this]() { showNextSlide(); });
    bindShortcut(QKeySequence(Qt::Key_Home), [this]() { showSlide(0); });
    bindShortcut(QKeySequence(Qt::Key_End), [this]() { showSlide(m_slides.size() - 1); });
}

void InAppPPTPreviewPage::setSlides(const QString &title, const QVector<QImage> &slides)
{
    m_title = title.trimmed().isEmpty() ? QStringLiteral("PPT 预览") : title;
    m_slides = slides;
    m_currentIndex = 0;
    if (m_titleLabel) {
        m_titleLabel->setText(m_title);
    }
    rebuildThumbnails();
    showSlide(0);
}

void InAppPPTPreviewPage::setSlidesFromPaths(const QString &title, const QStringList &paths)
{
    QVector<QImage> slides;
    for (const QString &path : paths) {
        QImage image(path);
        if (!image.isNull()) {
            slides.append(image);
        }
    }
    setSlides(title, slides);
}

void InAppPPTPreviewPage::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

QString InAppPPTPreviewPage::filePath() const
{
    return m_filePath;
}

void InAppPPTPreviewPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateSlideImage();
}

void InAppPPTPreviewPage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);
    mainLayout->addWidget(createHeader());
    mainLayout->addWidget(createBody(), 1);
    mainLayout->addWidget(createFooter());
}

void InAppPPTPreviewPage::setupStyles()
{
    setStyleSheet(R"(
        InAppPPTPreviewPage { background-color: #F8FAFC; }
        QLabel#previewTitleLabel { color: #1E293B; font-size: 18px; font-weight: 700; }
        QLabel#slidePreviewLabel { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 16px; color: #94A3B8; font-size: 15px; }
        QLabel#pageLabel { color: #334155; font-size: 15px; font-weight: 700; min-width: 80px; }
        QScrollArea#thumbnailScrollArea { background-color: #FFFFFF; border: 1px solid #E5E7EB; border-radius: 14px; }
        QPushButton#primaryButton { background-color: #C62828; color: #FFFFFF; border: 1px solid #C62828; border-radius: 12px; padding: 10px 18px; font-size: 14px; font-weight: 700; }
        QPushButton#primaryButton:hover, QPushButton#primaryButton:focus { background-color: #A61F1F; border-color: #A61F1F; }
        QPushButton#secondaryButton, QPushButton#navButton { background-color: #FFFFFF; color: #334155; border: 1px solid #CBD5E1; border-radius: 12px; padding: 10px 18px; font-size: 14px; font-weight: 700; }
        QPushButton#secondaryButton:hover, QPushButton#secondaryButton:focus, QPushButton#navButton:hover, QPushButton#navButton:focus { background-color: #F1F5F9; border-color: #C62828; color: #C62828; }
        QPushButton#navButton:disabled { background-color: #F8FAFC; color: #94A3B8; border-color: #E5E7EB; }
        QPushButton#thumbnailButton { background-color: #FFFFFF; color: #475569; border: 1px solid #E5E7EB; border-radius: 10px; padding: 8px; text-align: center; font-size: 12px; font-weight: 600; }
        QLabel#thumbnailImageLabel { background-color: transparent; border: none; }
        QLabel#thumbnailCaptionLabel { color: #475569; background-color: transparent; border: none; font-size: 12px; font-weight: 700; }
        QPushButton#thumbnailButton:hover, QPushButton#thumbnailButton:focus { background-color: #FFF5F5; border-color: #C62828; color: #C62828; }
        QPushButton#thumbnailButton[selected="true"] { background-color: #FFF1F2; border: 2px solid #C62828; color: #C62828; }
    )");
}

QWidget* InAppPPTPreviewPage::createHeader()
{
    auto *header = new QWidget(this);
    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    m_exitButton = createActionButton(QStringLiteral("退出预览"), QStringLiteral("secondaryButton"));
    m_titleLabel = new QLabel(QStringLiteral("PPT 预览"), header);
    m_titleLabel->setObjectName(QStringLiteral("previewTitleLabel"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_saveButton = createActionButton(QStringLiteral("保存到桌面"), QStringLiteral("primaryButton"));
    connect(m_exitButton, &QPushButton::clicked, this, &InAppPPTPreviewPage::exitRequested);
    connect(m_saveButton, &QPushButton::clicked, this, &InAppPPTPreviewPage::saveRequested);
    layout->addWidget(m_exitButton);
    layout->addWidget(m_titleLabel, 1);
    layout->addWidget(m_saveButton);
    return header;
}

QWidget* InAppPPTPreviewPage::createBody()
{
    auto *body = new QWidget(this);
    auto *layout = new QHBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
    auto *thumbContent = new QWidget();
    m_thumbnailLayout = new QVBoxLayout(thumbContent);
    m_thumbnailLayout->setContentsMargins(10, 10, 10, 10);
    m_thumbnailLayout->setSpacing(10);
    m_thumbnailLayout->addStretch();
    m_thumbnailScrollArea = new QScrollArea(body);
    m_thumbnailScrollArea->setObjectName(QStringLiteral("thumbnailScrollArea"));
    m_thumbnailScrollArea->setWidgetResizable(true);
    m_thumbnailScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_thumbnailScrollArea->setWidget(thumbContent);
    m_thumbnailScrollArea->setFixedWidth(176);
    m_slideLabel = new QLabel(QStringLiteral("暂无可预览内容"), body);
    m_slideLabel->setObjectName(QStringLiteral("slidePreviewLabel"));
    m_slideLabel->setAlignment(Qt::AlignCenter);
    m_slideLabel->setMinimumSize(640, 360);
    m_slideLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_thumbnailScrollArea);
    layout->addWidget(m_slideLabel, 1);
    return body;
}

QWidget* InAppPPTPreviewPage::createFooter()
{
    auto *footer = new QWidget(this);
    auto *layout = new QHBoxLayout(footer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    m_previousButton = createActionButton(QStringLiteral("上一页"), QStringLiteral("navButton"));
    m_nextButton = createActionButton(QStringLiteral("下一页"), QStringLiteral("navButton"));
    m_pageLabel = new QLabel(QStringLiteral("0 / 0"), footer);
    m_pageLabel->setObjectName(QStringLiteral("pageLabel"));
    m_pageLabel->setAlignment(Qt::AlignCenter);
    connect(m_previousButton, &QPushButton::clicked, this, &InAppPPTPreviewPage::showPreviousSlide);
    connect(m_nextButton, &QPushButton::clicked, this, &InAppPPTPreviewPage::showNextSlide);
    layout->addStretch();
    layout->addWidget(m_previousButton);
    layout->addWidget(m_pageLabel);
    layout->addWidget(m_nextButton);
    layout->addStretch();
    return footer;
}

QPushButton* InAppPPTPreviewPage::createActionButton(const QString &text, const QString &objectName)
{
    auto *button = new QPushButton(text, this);
    button->setObjectName(objectName);
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(ButtonMinHeight);
    button->setFocusPolicy(Qt::StrongFocus);
    return button;
}

void InAppPPTPreviewPage::rebuildThumbnails()
{
    if (!m_thumbnailLayout) {
        return;
    }
    while (m_thumbnailLayout->count() > 0) {
        QLayoutItem *item = m_thumbnailLayout->takeAt(0);
        delete item->widget();
        delete item;
    }
    m_thumbnailButtons.clear();
    for (int i = 0; i < m_slides.size(); ++i) {
        auto *button = new QPushButton(this);
        button->setObjectName(QStringLiteral("thumbnailButton"));
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(124);
        button->setFocusPolicy(Qt::StrongFocus);
        auto *buttonLayout = new QVBoxLayout(button);
        buttonLayout->setContentsMargins(6, 6, 6, 6);
        buttonLayout->setSpacing(6);
        auto *imageLabel = new QLabel(button);
        imageLabel->setObjectName(QStringLiteral("thumbnailImageLabel"));
        imageLabel->setFixedSize(ThumbnailWidth, ThumbnailHeight);
        imageLabel->setAlignment(Qt::AlignCenter);
        auto *captionLabel = new QLabel(QStringLiteral("第 %1 页").arg(i + 1), button);
        captionLabel->setObjectName(QStringLiteral("thumbnailCaptionLabel"));
        captionLabel->setAlignment(Qt::AlignCenter);
        if (!m_slides.at(i).isNull()) {
            imageLabel->setPixmap(QPixmap::fromImage(m_slides.at(i)).scaled(ThumbnailWidth, ThumbnailHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        buttonLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
        buttonLayout->addWidget(captionLabel);
        connect(button, &QPushButton::clicked, this, [this, i]() { showSlide(i); });
        m_thumbnailLayout->addWidget(button);
        m_thumbnailButtons.append(button);
    }
    m_thumbnailLayout->addStretch();
}

void InAppPPTPreviewPage::showSlide(int index)
{
    if (m_slides.isEmpty()) {
        m_currentIndex = 0;
    } else {
        m_currentIndex = qBound(0, index, m_slides.size() - 1);
    }
    updateSlideImage();
    updateNavigationState();
    updateThumbnailState();
}

void InAppPPTPreviewPage::showPreviousSlide()
{
    showSlide(m_currentIndex - 1);
}

void InAppPPTPreviewPage::showNextSlide()
{
    showSlide(m_currentIndex + 1);
}

void InAppPPTPreviewPage::updateSlideImage()
{
    if (!m_slideLabel) {
        return;
    }
    if (m_slides.isEmpty() || m_slides.value(m_currentIndex).isNull()) {
        m_slideLabel->setPixmap(QPixmap());
        m_slideLabel->setText(QStringLiteral("暂无可预览内容"));
        return;
    }
    m_slideLabel->setText(QString());
    const QSize target = m_slideLabel->size() - QSize(32, 32);
    m_slideLabel->setPixmap(QPixmap::fromImage(m_slides.at(m_currentIndex)).scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void InAppPPTPreviewPage::updateNavigationState()
{
    const bool hasSlides = !m_slides.isEmpty();
    if (m_pageLabel) {
        m_pageLabel->setText(hasSlides ? QStringLiteral("%1 / %2").arg(m_currentIndex + 1).arg(m_slides.size()) : QStringLiteral("0 / 0"));
    }
    if (m_previousButton) {
        m_previousButton->setEnabled(hasSlides && m_currentIndex > 0);
    }
    if (m_nextButton) {
        m_nextButton->setEnabled(hasSlides && m_currentIndex < m_slides.size() - 1);
    }
    if (m_saveButton) {
        m_saveButton->setEnabled(hasSlides);
    }
}

void InAppPPTPreviewPage::updateThumbnailState()
{
    for (int i = 0; i < m_thumbnailButtons.size(); ++i) {
        QPushButton *button = m_thumbnailButtons.at(i);
        if (!button) {
            continue;
        }
        button->setProperty("selected", i == m_currentIndex);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}
