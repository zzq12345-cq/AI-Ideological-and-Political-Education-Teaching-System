#ifndef INAPPPPTPREVIEWPAGE_H
#define INAPPPPTPREVIEWPAGE_H

#include <QImage>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>

class QLabel;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

class InAppPPTPreviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit InAppPPTPreviewPage(QWidget *parent = nullptr);

    void setSlides(const QString &title, const QVector<QImage> &slides);
    void setSlidesFromPaths(const QString &title, const QStringList &paths);
    void setFilePath(const QString &filePath);
    QString filePath() const;

signals:
    void exitRequested();
    void saveRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    void setupStyles();
    QWidget* createHeader();
    QWidget* createBody();
    QWidget* createFooter();
    QPushButton* createActionButton(const QString &text, const QString &objectName);
    void rebuildThumbnails();
    void showSlide(int index);
    void showPreviousSlide();
    void showNextSlide();
    void updateSlideImage();
    void updateNavigationState();
    void updateThumbnailState();

    QString m_title;
    QString m_filePath;
    QVector<QImage> m_slides;
    QVector<QPushButton*> m_thumbnailButtons;
    int m_currentIndex = 0;

    QLabel *m_titleLabel = nullptr;
    QLabel *m_slideLabel = nullptr;
    QLabel *m_pageLabel = nullptr;
    QPushButton *m_exitButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_previousButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QScrollArea *m_thumbnailScrollArea = nullptr;
    QVBoxLayout *m_thumbnailLayout = nullptr;
};

#endif // INAPPPPTPREVIEWPAGE_H
