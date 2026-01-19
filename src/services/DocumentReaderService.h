#ifndef DOCUMENTREADERSERVICE_H
#define DOCUMENTREADERSERVICE_H

#include <QObject>
#include <QString>
#include <QMap>

class SupabaseStorageService;

/**
 * @brief 文档读取服务
 *
 * 提供 DOCX 文档的文本内容提取功能，支持图片上传到 Supabase Storage
 * DOCX 本质是 ZIP 压缩包，内含 word/document.xml
 */
class DocumentReaderService : public QObject
{
    Q_OBJECT

public:
    explicit DocumentReaderService(QObject *parent = nullptr);

    /**
     * @brief 读取 DOCX 文件的纯文本内容
     * @param filePath DOCX 文件路径
     * @return 文档的纯文本内容，失败返回空字符串
     */
    QString readDocx(const QString &filePath);

    /**
     * @brief 读取 DOCX 文件内容，保留表格为 HTML 格式
     * @param filePath DOCX 文件路径
     * @return 文档内容（表格以 HTML 格式保留），失败返回空字符串
     */
    QString readDocxWithTables(const QString &filePath);

    /**
     * @brief 读取 DOCX 文件内容，保留表格和图片为 HTML 格式
     * @param filePath DOCX 文件路径
     * @return 文档内容（表格和图片以 HTML 格式保留），失败返回空字符串
     */
    QString readDocxWithImages(const QString &filePath);

    /**
     * @brief 检查文件是否为支持的文档格式
     * @param filePath 文件路径
     * @return true 如果是支持的格式
     */
    bool isSupportedFormat(const QString &filePath) const;

    /**
     * @brief 获取最后一次操作的错误信息
     */
    QString lastError() const;

    /**
     * @brief 设置是否使用云存储（Supabase Storage）存储图片
     * @param useCloud true 使用云存储，false 使用 base64 嵌入
     */
    void setUseCloudStorage(bool useCloud);

signals:
    void errorOccurred(const QString &error);
    void imageUploadProgress(int current, int total);

private:
    /**
     * @brief 从 DOCX ZIP 包中提取 document.xml
     */
    QByteArray extractDocumentXml(const QString &zipPath);

    /**
     * @brief 从 DOCX ZIP 包中提取所有内容到临时目录
     * @return 临时目录路径，失败返回空字符串
     */
    QString extractDocxToTemp(const QString &zipPath);

    /**
     * @brief 提取图片并转换为 base64 映射或上传到云存储
     * @param tempDir 解压后的临时目录
     * @return rId 到图片 URL/data URI 的映射
     */
    QMap<QString, QString> extractImages(const QString &tempDir);

    /**
     * @brief 解析 document.xml.rels 获取图片关系
     * @param tempDir 解压后的临时目录
     * @return rId 到图片文件路径的映射
     */
    QMap<QString, QString> parseRelationships(const QString &tempDir);

    /**
     * @brief 解析 document.xml 提取纯文本
     */
    QString parseDocumentXml(const QByteArray &xmlData);

    /**
     * @brief 解析 document.xml 提取内容（保留表格为 HTML）
     */
    QString parseDocumentXmlWithTables(const QByteArray &xmlData);

    /**
     * @brief 解析 document.xml 提取内容（保留表格和图片为 HTML）
     */
    QString parseDocumentXmlWithImages(const QByteArray &xmlData, const QMap<QString, QString> &imageMap);

    QString m_lastError;
    QString m_currentTempDir;  // 当前解压的临时目录
    bool m_useCloudStorage;    // 是否使用云存储
    SupabaseStorageService *m_storageService;
};

#endif // DOCUMENTREADERSERVICE_H
