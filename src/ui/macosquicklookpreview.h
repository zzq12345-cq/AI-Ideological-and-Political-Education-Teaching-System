#ifndef MACOSQUICKLOOKPREVIEW_H
#define MACOSQUICKLOOKPREVIEW_H

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QWindow;

class MacOSQuickLookPreview : public QWidget
{
    Q_OBJECT

public:
    explicit MacOSQuickLookPreview(QWidget *parent = nullptr);
    ~MacOSQuickLookPreview() override;

    void setFilePath(const QString &filePath);
    QString filePath() const;
    bool isPreviewAvailable() const;

private:
    void setPlaceholderText(const QString &text);

    QString m_filePath;
    bool m_previewAvailable = false;
    QVBoxLayout *m_layout = nullptr;
    QWidget *m_container = nullptr;
    QLabel *m_placeholderLabel = nullptr;
    QWindow *m_foreignWindow = nullptr;
    void *m_nativePreviewView = nullptr;
};

#endif // MACOSQUICKLOOKPREVIEW_H
