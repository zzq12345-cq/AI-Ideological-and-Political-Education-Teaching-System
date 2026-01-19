#include "DocumentReaderService.h"
#include "SupabaseStorageService.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDateTime>

DocumentReaderService::DocumentReaderService(QObject *parent)
    : QObject(parent)
    , m_useCloudStorage(true)  // 默认使用云存储
    , m_storageService(new SupabaseStorageService(this))
{
    // 转发上传进度信号
    connect(m_storageService, &SupabaseStorageService::uploadProgress,
            this, &DocumentReaderService::imageUploadProgress);
}

void DocumentReaderService::setUseCloudStorage(bool useCloud)
{
    m_useCloudStorage = useCloud;
}

QString DocumentReaderService::lastError() const
{
    return m_lastError;
}

bool DocumentReaderService::isSupportedFormat(const QString &filePath) const
{
    QString suffix = QFileInfo(filePath).suffix().toLower();
    return suffix == "docx";
}

QString DocumentReaderService::readDocx(const QString &filePath)
{
    m_lastError.clear();
    
    if (!QFile::exists(filePath)) {
        m_lastError = QString("文件不存在: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return QString();
    }
    
    if (!isSupportedFormat(filePath)) {
        m_lastError = QString("不支持的文件格式: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return QString();
    }
    
    // 提取 document.xml
    QByteArray xmlData = extractDocumentXml(filePath);
    if (xmlData.isEmpty()) {
        return QString();
    }
    
    // 解析 XML 提取文本
    QString text = parseDocumentXml(xmlData);
    
    qDebug() << "[DocumentReaderService] 成功读取文档:" << filePath 
             << "文本长度:" << text.length();
    
    return text;
}

QByteArray DocumentReaderService::extractDocumentXml(const QString &zipPath)
{
    // 使用临时目录解压
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_lastError = "无法创建临时目录";
        emit errorOccurred(m_lastError);
        return QByteArray();
    }
    
    // 使用 unzip 命令解压 document.xml
    QProcess unzip;
    unzip.setWorkingDirectory(tempDir.path());
    
    // macOS/Linux 使用 unzip 命令
    QStringList args;
    args << "-o" << zipPath << "word/document.xml" << "-d" << tempDir.path();
    
    unzip.start("unzip", args);
    if (!unzip.waitForFinished(10000)) {
        m_lastError = "解压超时";
        emit errorOccurred(m_lastError);
        return QByteArray();
    }
    
    if (unzip.exitCode() != 0) {
        // 尝试检查是否有部分内容被解压
        QString errorOutput = QString::fromUtf8(unzip.readAllStandardError());
        qDebug() << "[DocumentReaderService] unzip 输出:" << errorOutput;
    }
    
    // 读取解压后的 document.xml
    QString xmlPath = tempDir.path() + "/word/document.xml";
    QFile xmlFile(xmlPath);
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        m_lastError = QString("无法读取 document.xml: %1").arg(xmlPath);
        emit errorOccurred(m_lastError);
        return QByteArray();
    }
    
    QByteArray data = xmlFile.readAll();
    xmlFile.close();
    
    return data;
}

QString DocumentReaderService::parseDocumentXml(const QByteArray &xmlData)
{
    QString result;
    QXmlStreamReader xml(xmlData);

    bool inParagraph = false;
    QString currentParagraph;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            // w:p 是段落
            if (name == "p") {
                inParagraph = true;
                currentParagraph.clear();
            }
            // w:t 是文本
            else if (name == "t" && inParagraph) {
                QString text = xml.readElementText();
                currentParagraph += text;
            }
            // w:tab 是制表符
            else if (name == "tab") {
                currentParagraph += "\t";
            }
            // w:br 是换行
            else if (name == "br") {
                currentParagraph += "\n";
            }
        }
        else if (token == QXmlStreamReader::EndElement) {
            QString name = xml.name().toString();

            if (name == "p" && inParagraph) {
                // 段落结束，添加到结果
                if (!currentParagraph.trimmed().isEmpty()) {
                    if (!result.isEmpty()) {
                        result += "\n";
                    }
                    result += currentParagraph;
                }
                inParagraph = false;
            }
        }
    }

    if (xml.hasError()) {
        qDebug() << "[DocumentReaderService] parseDocumentXmlWithTables XML 解析错误:" << xml.errorString();
        m_lastError = QString("XML 解析错误: %1").arg(xml.errorString());
    }

    return result;
}

QString DocumentReaderService::extractDocxToTemp(const QString &zipPath)
{
    // 创建临时目录
    static QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_lastError = "无法创建临时目录";
        emit errorOccurred(m_lastError);
        return QString();
    }

    // 为每个文件创建子目录
    QString subDir = tempDir.path() + "/" + QFileInfo(zipPath).baseName() + "_" +
                     QString::number(QDateTime::currentMSecsSinceEpoch());
    QDir().mkpath(subDir);

    // 使用 unzip 命令解压所有文件
    QProcess unzip;
    QStringList args;
    args << "-o" << zipPath << "-d" << subDir;

    unzip.start("unzip", args);
    if (!unzip.waitForFinished(30000)) {
        m_lastError = "解压超时";
        emit errorOccurred(m_lastError);
        return QString();
    }

    m_currentTempDir = subDir;
    return subDir;
}

QMap<QString, QString> DocumentReaderService::parseRelationships(const QString &tempDir)
{
    QMap<QString, QString> relationships;

    QString relsPath = tempDir + "/word/_rels/document.xml.rels";
    QFile relsFile(relsPath);
    if (!relsFile.open(QIODevice::ReadOnly)) {
        qDebug() << "[DocumentReaderService] 无法读取关系文件:" << relsPath;
        return relationships;
    }

    QXmlStreamReader xml(&relsFile);
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement && xml.name().toString() == "Relationship") {
            QString id = xml.attributes().value("Id").toString();
            QString target = xml.attributes().value("Target").toString();
            QString type = xml.attributes().value("Type").toString();

            // 只关注图片关系
            if (type.contains("image")) {
                relationships[id] = target;
            }
        }
    }

    relsFile.close();
    return relationships;
}

QMap<QString, QString> DocumentReaderService::extractImages(const QString &tempDir)
{
    QMap<QString, QString> imageMap;

    // 解析关系文件获取 rId -> 图片路径映射
    QMap<QString, QString> rels = parseRelationships(tempDir);

    QMimeDatabase mimeDb;

    for (auto it = rels.begin(); it != rels.end(); ++it) {
        QString rId = it.key();
        QString target = it.value();

        // 构建完整路径
        QString imagePath = tempDir + "/word/" + target;
        QFile imageFile(imagePath);
        if (!imageFile.open(QIODevice::ReadOnly)) {
            qDebug() << "[DocumentReaderService] 无法读取图片:" << imagePath;
            continue;
        }

        QByteArray imageData = imageFile.readAll();
        imageFile.close();

        // 检测 MIME 类型
        QMimeType mimeType = mimeDb.mimeTypeForFile(imagePath);
        QString mimeString = mimeType.name();
        if (mimeString.isEmpty() || mimeString == "application/octet-stream") {
            // 根据扩展名推断
            QString ext = QFileInfo(imagePath).suffix().toLower();
            if (ext == "png") mimeString = "image/png";
            else if (ext == "jpg" || ext == "jpeg") mimeString = "image/jpeg";
            else if (ext == "gif") mimeString = "image/gif";
            else if (ext == "bmp") mimeString = "image/bmp";
            else mimeString = "image/png";
        }

        QString imageUrl;

        if (m_useCloudStorage) {
            // 上传到 Supabase Storage
            QString fileName = QFileInfo(target).fileName();
            imageUrl = m_storageService->uploadImage(imageData, fileName, mimeString);
            if (imageUrl.isEmpty()) {
                qDebug() << "[DocumentReaderService] 上传失败，回退到 base64:" << m_storageService->lastError();
                // 回退到 base64
                imageUrl = QString("data:%1;base64,%2")
                              .arg(mimeString)
                              .arg(QString::fromLatin1(imageData.toBase64()));
            }
        } else {
            // 转换为 base64 data URI
            imageUrl = QString("data:%1;base64,%2")
                          .arg(mimeString)
                          .arg(QString::fromLatin1(imageData.toBase64()));
        }

        imageMap[rId] = imageUrl;
        qDebug() << "[DocumentReaderService] 提取图片:" << rId << "->" << target
                 << "大小:" << imageData.size() << "bytes"
                 << (m_useCloudStorage ? "(云存储)" : "(base64)");
    }

    return imageMap;
}

QString DocumentReaderService::readDocxWithImages(const QString &filePath)
{
    m_lastError.clear();

    if (!QFile::exists(filePath)) {
        m_lastError = QString("文件不存在: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return QString();
    }

    if (!isSupportedFormat(filePath)) {
        m_lastError = QString("不支持的文件格式: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return QString();
    }

    // 解压整个 DOCX
    QString tempDir = extractDocxToTemp(filePath);
    if (tempDir.isEmpty()) {
        return QString();
    }

    // 提取图片（上传到云存储或转为 base64）
    QMap<QString, QString> imageMap = extractImages(tempDir);

    // 读取 document.xml
    QString xmlPath = tempDir + "/word/document.xml";
    QFile xmlFile(xmlPath);
    if (!xmlFile.open(QIODevice::ReadOnly)) {
        m_lastError = QString("无法读取 document.xml: %1").arg(xmlPath);
        emit errorOccurred(m_lastError);
        return QString();
    }

    QByteArray xmlData = xmlFile.readAll();
    xmlFile.close();

    // 解析 XML（支持图片）
    QString text = parseDocumentXmlWithImages(xmlData, imageMap);

    qDebug() << "[DocumentReaderService] 成功读取文档(含图片):" << filePath
             << "文本长度:" << text.length() << "图片数:" << imageMap.size();

    return text;
}

QString DocumentReaderService::parseDocumentXmlWithImages(const QByteArray &xmlData, const QMap<QString, QString> &imageMap)
{
    QString result;
    QXmlStreamReader xml(xmlData);

    bool inParagraph = false;
    bool inTable = false;
    bool inTableRow = false;
    bool inTableCell = false;
    bool inDrawing = false;
    QString currentParagraph;
    QString currentCell;
    QString currentImageRId;
    QStringList currentRow;
    QList<QStringList> tableRows;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            // w:tbl 是表格开始
            if (name == "tbl") {
                inTable = true;
                tableRows.clear();
            }
            // w:tr 是表格行
            else if (name == "tr" && inTable) {
                inTableRow = true;
                currentRow.clear();
            }
            // w:tc 是表格单元格
            else if (name == "tc" && inTableRow) {
                inTableCell = true;
                currentCell.clear();
            }
            // w:p 是段落
            else if (name == "p") {
                inParagraph = true;
                currentParagraph.clear();
            }
            // w:drawing 或 w:pict 是图片容器
            else if (name == "drawing" || name == "pict") {
                inDrawing = true;
                currentImageRId.clear();
            }
            // a:blip 包含图片引用 (r:embed 属性)
            else if (name == "blip" && inDrawing) {
                // 尝试获取 r:embed 属性
                for (const QXmlStreamAttribute &attr : xml.attributes()) {
                    if (attr.name().toString() == "embed") {
                        currentImageRId = attr.value().toString();
                        break;
                    }
                }
            }
            // v:imagedata 也可能包含图片引用 (r:id 属性)
            else if (name == "imagedata" && inDrawing) {
                for (const QXmlStreamAttribute &attr : xml.attributes()) {
                    if (attr.name().toString() == "id") {
                        currentImageRId = attr.value().toString();
                        break;
                    }
                }
            }
            // w:t 是文本
            else if (name == "t") {
                QString text = xml.readElementText();
                if (inTableCell) {
                    currentCell += text;
                } else if (inParagraph) {
                    currentParagraph += text;
                }
            }
            // w:tab 是制表符
            else if (name == "tab") {
                if (inTableCell) {
                    currentCell += "\t";
                } else if (inParagraph) {
                    currentParagraph += "\t";
                }
            }
            // w:br 是换行
            else if (name == "br") {
                if (inTableCell) {
                    currentCell += "<br>";
                } else if (inParagraph) {
                    currentParagraph += "\n";
                }
            }
        }
        else if (token == QXmlStreamReader::EndElement) {
            QString name = xml.name().toString();

            // 图片容器结束 - 插入图片
            if ((name == "drawing" || name == "pict") && inDrawing) {
                if (!currentImageRId.isEmpty() && imageMap.contains(currentImageRId)) {
                    QString imgTag = QString("<img src=\"%1\" style=\"max-width: 100%; height: auto;\">")
                                        .arg(imageMap[currentImageRId]);
                    if (inTableCell) {
                        currentCell += imgTag;
                    } else if (inParagraph) {
                        currentParagraph += imgTag;
                    }
                }
                inDrawing = false;
                currentImageRId.clear();
            }
            // 表格单元格结束
            else if (name == "tc" && inTableCell) {
                currentRow.append(currentCell.trimmed());
                inTableCell = false;
            }
            // 表格行结束
            else if (name == "tr" && inTableRow) {
                if (!currentRow.isEmpty()) {
                    tableRows.append(currentRow);
                }
                inTableRow = false;
            }
            // 表格结束 - 转换为 HTML
            else if (name == "tbl" && inTable) {
                if (!tableRows.isEmpty()) {
                    QString tableHtml = "<table border=\"1\" style=\"border-collapse: collapse;\">\n";
                    for (int i = 0; i < tableRows.size(); i++) {
                        const QStringList &row = tableRows.at(i);
                        tableHtml += "<tr>";
                        QString cellTag = (i == 0) ? "th" : "td";
                        for (const QString &cell : row) {
                            tableHtml += QString("<%1 style=\"padding: 8px; border: 1px solid #ccc;\">%2</%1>")
                                        .arg(cellTag)
                                        .arg(cell);
                        }
                        tableHtml += "</tr>\n";
                    }
                    tableHtml += "</table>\n";

                    if (!result.isEmpty() && !result.endsWith('\n')) {
                        result += "\n";
                    }
                    result += tableHtml;
                }
                tableRows.clear();
                inTable = false;
            }
            // 段落结束（非表格内）
            else if (name == "p" && inParagraph && !inTableCell) {
                if (!currentParagraph.trimmed().isEmpty()) {
                    if (!result.isEmpty() && !result.endsWith('\n')) {
                        result += "\n";
                    }
                    result += currentParagraph;
                }
                inParagraph = false;
            }
            // 表格内段落结束
            else if (name == "p" && inParagraph && inTableCell) {
                if (!currentParagraph.isEmpty()) {
                    if (!currentCell.isEmpty()) {
                        currentCell += "<br>";
                    }
                    currentCell += currentParagraph;
                }
                inParagraph = false;
            }
        }
    }

    if (xml.hasError()) {
        qDebug() << "[DocumentReaderService] parseDocumentXmlWithImages XML 解析错误:" << xml.errorString();
        m_lastError = QString("XML 解析错误: %1").arg(xml.errorString());
    }

    return result;
}

QString DocumentReaderService::readDocxWithTables(const QString &filePath)
{
    m_lastError.clear();

    if (!QFile::exists(filePath)) {
        m_lastError = QString("文件不存在: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return QString();
    }

    if (!isSupportedFormat(filePath)) {
        m_lastError = QString("不支持的文件格式: %1").arg(filePath);
        emit errorOccurred(m_lastError);
        return QString();
    }

    // 提取 document.xml
    QByteArray xmlData = extractDocumentXml(filePath);
    if (xmlData.isEmpty()) {
        return QString();
    }

    // 解析 XML 提取文本（保留表格为 HTML）
    QString text = parseDocumentXmlWithTables(xmlData);

    qDebug() << "[DocumentReaderService] 成功读取文档(含表格):" << filePath
             << "文本长度:" << text.length();

    return text;
}

QString DocumentReaderService::parseDocumentXmlWithTables(const QByteArray &xmlData)
{
    QString result;
    QXmlStreamReader xml(xmlData);

    bool inParagraph = false;
    bool inTable = false;
    bool inTableRow = false;
    bool inTableCell = false;
    QString currentParagraph;
    QString currentCell;
    QStringList currentRow;
    QList<QStringList> tableRows;

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            QString name = xml.name().toString();

            // w:tbl 是表格开始
            if (name == "tbl") {
                inTable = true;
                tableRows.clear();
            }
            // w:tr 是表格行
            else if (name == "tr" && inTable) {
                inTableRow = true;
                currentRow.clear();
            }
            // w:tc 是表格单元格
            else if (name == "tc" && inTableRow) {
                inTableCell = true;
                currentCell.clear();
            }
            // w:p 是段落
            else if (name == "p") {
                inParagraph = true;
                currentParagraph.clear();
            }
            // w:t 是文本
            else if (name == "t") {
                QString text = xml.readElementText();
                if (inTableCell) {
                    currentCell += text;
                } else if (inParagraph) {
                    currentParagraph += text;
                }
            }
            // w:tab 是制表符
            else if (name == "tab") {
                if (inTableCell) {
                    currentCell += "\t";
                } else if (inParagraph) {
                    currentParagraph += "\t";
                }
            }
            // w:br 是换行
            else if (name == "br") {
                if (inTableCell) {
                    currentCell += "<br>";
                } else if (inParagraph) {
                    currentParagraph += "\n";
                }
            }
        }
        else if (token == QXmlStreamReader::EndElement) {
            QString name = xml.name().toString();

            // 表格单元格结束
            if (name == "tc" && inTableCell) {
                currentRow.append(currentCell.trimmed());
                inTableCell = false;
            }
            // 表格行结束
            else if (name == "tr" && inTableRow) {
                if (!currentRow.isEmpty()) {
                    tableRows.append(currentRow);
                }
                inTableRow = false;
            }
            // 表格结束 - 转换为 HTML
            else if (name == "tbl" && inTable) {
                if (!tableRows.isEmpty()) {
                    QString tableHtml = "<table border=\"1\" style=\"border-collapse: collapse;\">\n";
                    for (int i = 0; i < tableRows.size(); i++) {
                        const QStringList &row = tableRows.at(i);
                        tableHtml += "<tr>";
                        // 第一行作为表头
                        QString cellTag = (i == 0) ? "th" : "td";
                        for (const QString &cell : row) {
                            tableHtml += QString("<%1 style=\"padding: 8px; border: 1px solid #ccc;\">%2</%1>")
                                        .arg(cellTag)
                                        .arg(cell);
                        }
                        tableHtml += "</tr>\n";
                    }
                    tableHtml += "</table>\n";

                    if (!result.isEmpty() && !result.endsWith('\n')) {
                        result += "\n";
                    }
                    result += tableHtml;
                }
                tableRows.clear();
                inTable = false;
            }
            // 段落结束（非表格内）
            else if (name == "p" && inParagraph && !inTableCell) {
                if (!currentParagraph.trimmed().isEmpty()) {
                    if (!result.isEmpty() && !result.endsWith('\n')) {
                        result += "\n";
                    }
                    result += currentParagraph;
                }
                inParagraph = false;
            }
            // 表格内段落结束
            else if (name == "p" && inParagraph && inTableCell) {
                // 表格内段落用换行分隔
                if (!currentParagraph.isEmpty()) {
                    if (!currentCell.isEmpty()) {
                        currentCell += "<br>";
                    }
                    currentCell += currentParagraph;
                }
                inParagraph = false;
            }
        }
    }

    if (xml.hasError()) {
        qDebug() << "[DocumentReaderService] XML 解析错误:" << xml.errorString();
        m_lastError = QString("XML 解析错误: %1").arg(xml.errorString());
    }

    return result;
}
