#ifndef AIPREPARATIONWIDGET_H
#define AIPREPARATIONWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QColor>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QImage>
#include <QDialog>
#include <QVector>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QScrollArea>

// Forward declarations
class SlidePreviewDialog;

class AIPreparationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int animationProgress READ animationProgress WRITE setAnimationProgress)

public:
    enum class GenerationState {
        Idle,
        Generating,
        Success,
        Failed
    };

    explicit AIPreparationWidget(QWidget *parent = nullptr, bool enableOnlineEdit = false);
    ~AIPreparationWidget();

    // Getters
    QString selectedTextbook() const;
    QString selectedGrade() const;
    QString selectedChapter() const;
    QString selectedTemplateKey() const;
    QString selectedDuration() const;
    QString contentFocus() const;
    GenerationState generationState() const { return m_state; }

    // API
    void setProgress(int percent);
    void setGenerationState(GenerationState state);
    void setSlides(const QVector<QImage> &images);

signals:
    void generateRequested(const QMap<QString, QString> &params);
    void previewRequested(int index);
    void downloadRequested();
    void saveToLibraryRequested();
    void onlineEditRequested();
    void regenerateRequested();
    void slidePreviewRequested(int index);
    void slidesReordered(const QList<int> &newOrder);

private slots:
    void onGenerateClicked();
    void onToggleAdvanced(bool checked);
    void onTemplateSelected(const QString &key);
    void onPreviewClicked(int index);
    void onDownloadClicked();
    void onSaveClicked();
    void onOnlineEditClicked();
    void onRegenerateClicked();
    void onPreviewNavigate(int offset);
    void onPreviewDelete(int index);
    void onProgressTimerTimeout();

private:
    void initUI();
    void setupTopSection();
    void setupSelectionSection();
    void setupAdvancedSection();
    void setupPreviewSection();
    void setupStatusStack(QHBoxLayout *headerLayout);
    void setupActionsBar();

    QWidget *createLabeledField(const QString &labelText, QWidget *field);
    void createTemplateCard(const QString &key, const QString &name, const QColor &thumbnailColor, bool isSelected);
    void updateTemplateCardStyle(const QString &key, bool isSelected);
    void createPreviewCard(int index, const QImage *image = nullptr);
    void updatePreviewCard(int index, const QImage &image);
    void refreshPreviewCards();
    void updatePlaceholderCard();
    void showPreviewDialog(int index);
    void updateActionsVisibility();
    void updateStatusForCurrentState();
    void startProgressAnimation();
    void stopProgressAnimation();

    // Event filter
    bool eventFilter(QObject *obj, QEvent *event) override;

    // Drag and drop
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    // Animation
    int animationProgress() const { return m_animationProgress; }
    void setAnimationProgress(int progress);
    void updateAdvancedSectionHeight();

    // Constants
    void setupConstants();

    // UI Components
    QVBoxLayout *m_mainLayout;

    // Top Section
    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;

    // Selection Section
    QComboBox *m_textbookCombo;
    QComboBox *m_gradeCombo;
    QComboBox *m_chapterCombo;
    QFrame *m_selectionCard;
    QPushButton *m_generateBtn;
    QPushButton *m_toggleAdvancedBtn;
    QFrame *m_advancedSection;
    QVBoxLayout *m_advancedLayout;
    QPropertyAnimation *m_advancedAnimation;
    int m_advancedMaxHeight;
    int m_animationProgress;

    // Advanced Options
    QMap<QString, QFrame*> m_templateCards;
    QMap<QString, QLabel*> m_templateNameLabels;
    QString m_selectedTemplateKey;
    QComboBox *m_durationCombo;
    QLineEdit *m_contentFocusEdit;

    // Preview Section
    QFrame *m_previewContainer;
    QGridLayout *m_previewGrid;
    QStackedWidget *m_statusStack;
    QFrame *m_generatingStatus;
    QFrame *m_successStatus;
    QFrame *m_failedStatus;
    QLabel *m_progressDot;
    QLabel *m_progressLabel;
    QLabel *m_successChipLabel;
    QLabel *m_failedChipLabel;
    QTimer *m_progressTimer;
    bool m_progressDotVisible;

    // Actions Bar
    QFrame *m_actionsBar;
    QPushButton *m_downloadBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_onlineEditBtn;
    QPushButton *m_regenerateBtn;

    // Preview Cards
    QVector<QFrame*> m_previewCards;
    QVector<QWidget*> m_previewOverlays;
    QVector<QPushButton*> m_previewButtons;
    QVector<QLabel*> m_previewImageLabels;
    QVector<QLabel*> m_previewCaptionLabels;
    QFrame *m_previewPlaceholderCard;
    QLabel *m_placeholderLabel;
    QLabel *m_placeholderIcon;
    QVector<QImage> m_slideImages;
    QVector<int> m_slideOrder;
    bool m_enableOnlineEdit;
    QPoint m_dragStartPos;
    int m_dragSourceIndex;
    bool m_dragging;
    QPointer<SlidePreviewDialog> m_activePreviewDialog;

    // State
    GenerationState m_state;
    int m_currentProgress;

    // Constants
    QString COLOR_PRIMARY;
    QString COLOR_ACCENT;
    QString COLOR_TEXT;
    QString COLOR_SUBTEXT;
    QString COLOR_BORDER;
    QString COLOR_SURFACE;
    QString COLOR_BG_LIGHT;
    QString COLOR_BG_DARK;
    QString COLOR_SUCCESS;
    QString COLOR_FAILED;
    int WIDGET_HEIGHT;
    int CORNER_RADIUS;
    int BORDER_WIDTH;
    int SPACING_SMALL;
    int SPACING_MEDIUM;
    int SPACING_LARGE;
    int TEMPLATE_CARD_WIDTH;
    int TEMPLATE_CARD_HEIGHT;
    int PREVIEW_CARD_HEIGHT;
};

// Preview Dialog
class SlidePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SlidePreviewDialog(const QImage &image, int index, QWidget *parent = nullptr);
    void setImage(const QImage &image);
    void setIndex(int index) { m_index = index; }
    int index() const { return m_index; }
    void setTotalSlides(int total);
    void updateNavigationButtons();

signals:
    void navigateRequested(int offset);
    void deleteRequested(int index);

private slots:
    void onPrevClicked();
    void onNextClicked();
    void onZoomInClicked();
    void onZoomOutClicked();
    void onFitToWindowClicked();
    void onDeleteClicked();

private:
    void setupUI();
    void updateScale();

    QLabel *m_imageLabel;
    QLabel *m_indexLabel;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_zoomInBtn;
    QPushButton *m_zoomOutBtn;
    QPushButton *m_fitBtn;
    QPushButton *m_deleteBtn;
    QScrollArea *m_scrollArea;
    QImage m_image;
    int m_index;
    int m_total;
    qreal m_scaleFactor;
    qreal m_fitScale;
};

#endif // AIPREPARATIONWIDGET_H
