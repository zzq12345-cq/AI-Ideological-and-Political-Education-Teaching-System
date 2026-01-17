#include "DocumentReaderService.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QDebug>
#include <QFileInfo>

DocumentReaderService::DocumentReaderService(QObject *parent)
    : QObject(parent)
{
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
        qDebug() << "[DocumentReaderService] XML 解析错误:" << xml.errorString();
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
