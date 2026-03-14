#ifndef PPTPREVIEWPAGE_H
#define PPTPREVIEWPAGE_H

#include <QWidget>

class QLabel;
class QPushButton;
class MacOSQuickLookPreview;

class PPTPreviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit PPTPreviewPage(QWidget *parent = nullptr);

    void setPresentation(const QString &title, const QString &filePath);
    QString presentationPath() const;

signals:
    void backRequested();
    void downloadRequested();

private:
    void updateState();

    QString m_title;
    QString m_filePath;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_hintLabel = nullptr;
    QLabel *m_fileLabel = nullptr;
    QPushButton *m_backButton = nullptr;
    QPushButton *m_downloadButton = nullptr;
    MacOSQuickLookPreview *m_previewWidget = nullptr;
};

#endif // PPTPREVIEWPAGE_H
