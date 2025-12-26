#ifndef DOCUMENTREADERSERVICE_H
#define DOCUMENTREADERSERVICE_H

#include <QObject>
#include <QString>

/**
 * @brief 文档读取服务
 * 
 * 提供 DOCX 文档的文本内容提取功能
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
     * @brief 检查文件是否为支持的文档格式
     * @param filePath 文件路径
     * @return true 如果是支持的格式
     */
    bool isSupportedFormat(const QString &filePath) const;
    
    /**
     * @brief 获取最后一次操作的错误信息
     */
    QString lastError() const;

signals:
    void errorOccurred(const QString &error);

private:
    /**
     * @brief 从 DOCX ZIP 包中提取 document.xml
     */
    QByteArray extractDocumentXml(const QString &zipPath);
    
    /**
     * @brief 解析 document.xml 提取纯文本
     */
    QString parseDocumentXml(const QByteArray &xmlData);
    
    QString m_lastError;
};

#endif // DOCUMENTREADERSERVICE_H
