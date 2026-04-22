#ifndef MATERIALWIDGET_H
#define MATERIALWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QGridLayout>
#include <QEvent>
#include "MaterialManager.h"

// 面包屑点击事件过滤器
class BreadcrumbClickFilter : public QObject
{
    Q_OBJECT
public:
    BreadcrumbClickFilter(int index, std::function<void(int)> callback, QObject *parent = nullptr)
        : QObject(parent), m_index(index), m_callback(callback) {}
protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            m_callback(m_index);
            return true;
        }
        return QObject::eventFilter(watched, event);
    }
private:
    int m_index;
    std::function<void(int)> m_callback;
};

// 卡片点击事件过滤器
class CardClickFilter : public QObject
{
    Q_OBJECT
public:
    CardClickFilter(bool isFolder, const QString &folderId, const QString &folderName,
                    const QString &fileUrl, std::function<void()> onFolder,
                    std::function<void(const QString &)> onFile, QObject *parent = nullptr)
        : QObject(parent), m_isFolder(isFolder), m_folderId(folderId),
          m_folderName(folderName), m_fileUrl(fileUrl),
          m_onFolder(onFolder), m_onFile(onFile) {}
protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (m_isFolder) m_onFolder();
            else if (!m_fileUrl.isEmpty()) m_onFile(m_fileUrl);
            return true;
        }
        return QObject::eventFilter(watched, event);
    }
private:
    bool m_isFolder;
    QString m_folderId, m_folderName, m_fileUrl;
    std::function<void()> m_onFolder;
    std::function<void(const QString &)> m_onFile;
};

class MaterialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MaterialWidget(const QString &classId, const QString &uploaderEmail, QWidget *parent = nullptr);

signals:
    void backRequested();

private slots:
    void onCreateFolderClicked();
    void onUploadFileClicked();

private:
    void setupUI();
    void loadFolder(const QString &folderId, const QString &folderName = QString());
    void navigateToBreadcrumb(int index);
    void refreshCurrentFolder();
    void updateBreadcrumbUI();
    QWidget* createMaterialCard(const MaterialManager::MaterialInfo &info);
    void clearGrid();
    QString formatFileSize(qint64 bytes) const;
    QString getFileIcon(const QString &mimeType, const QString &type) const;

    QString m_classId;
    QString m_uploaderEmail;
    QString m_currentFolderId;
    QList<QPair<QString, QString>> m_breadcrumb; // (folderId, folderName)

    QFrame *m_breadcrumbBar = nullptr;
    QHBoxLayout *m_breadcrumbLayout = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QWidget *m_gridContainer = nullptr;
    QScrollArea *m_scrollArea = nullptr;
};

#endif
