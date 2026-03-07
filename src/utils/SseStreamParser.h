#ifndef SSESTREAMPARSER_H
#define SSESTREAMPARSER_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <functional>

/**
 * @brief SSE (Server-Sent Events) 流式协议解析器
 *
 * 纯解析工具，不持有网络连接。负责将 SSE 字节流解析为 (event, QJsonObject) 对。
 * 业务逻辑通过回调 (EventHandler) 处理。
 *
 * 用法：
 *   SseStreamParser parser;
 *   parser.setEventHandler([](const QString &event, const QJsonObject &data) {
 *       // 处理业务逻辑
 *   });
 *   // 在 onReadyRead 中调用：
 *   parser.feed(reply->readAll());
 *   // 在 onFinished 中调用：
 *   parser.flush();
 */
class SseStreamParser
{
public:
    using EventHandler = std::function<void(const QString &event, const QJsonObject &data)>;

    SseStreamParser() = default;

    void setEventHandler(EventHandler handler) { m_handler = std::move(handler); }

    /**
     * @brief 喂入新的数据块
     */
    void feed(const QByteArray &data)
    {
        QString buffer = m_streamBuffer + QString::fromUtf8(data);
        QStringList lines = buffer.split('\n');
        const bool endedWithNewline = buffer.endsWith('\n');
        m_streamBuffer.clear();

        // 保留不完整的最后一行
        if (!endedWithNewline && !lines.isEmpty()) {
            QString lastLine = lines.takeLast();
            if (!lastLine.trimmed().isEmpty()) {
                m_streamBuffer = lastLine;
            }
        }

        for (const QString &line : lines) {
            processLine(line.trimmed());
        }

        // 如果以双换行结尾，立即 flush 当前事件
        if (endedWithNewline && buffer.endsWith("\n\n")) {
            flushEvent();
        }
    }

    /**
     * @brief 强制刷新残留缓冲（连接结束时调用）
     */
    void flush()
    {
        if (!m_streamBuffer.isEmpty()) {
            processLine(m_streamBuffer.trimmed());
            m_streamBuffer.clear();
        }
        flushEvent();
    }

    /**
     * @brief 重置解析器状态
     */
    void reset()
    {
        m_streamBuffer.clear();
        m_sseEvent.clear();
        m_sseDataLines.clear();
    }

    /**
     * @brief 检查是否有未刷新的缓冲数据
     */
    bool hasPendingData() const
    {
        return !m_streamBuffer.isEmpty() || !m_sseEvent.isEmpty() || !m_sseDataLines.isEmpty();
    }

private:
    void processLine(const QString &trimmed)
    {
        if (trimmed.isEmpty()) {
            flushEvent();
            return;
        }

        if (trimmed.startsWith("event:")) {
            QString eventValue = trimmed.mid(6).trimmed();
            if (!eventValue.isEmpty()) {
                m_sseEvent = eventValue;
            }
            return;
        }

        if (trimmed.startsWith("data:")) {
            QString dataValue = trimmed.mid(5);
            if (dataValue.startsWith(' ')) {
                dataValue = dataValue.mid(1);
            }
            m_sseDataLines.append(dataValue);
            return;
        }

        // SSE 注释行
        if (trimmed.startsWith(':')) {
            return;
        }

        // 非标准 SSE：裸 JSON 行
        if (trimmed.startsWith('{')) {
            QJsonDocument doc = QJsonDocument::fromJson(trimmed.toUtf8());
            if (!doc.isNull() && doc.isObject()) {
                dispatchEvent(m_sseEvent, doc.object());
                m_sseEvent.clear();
            }
            return;
        }
    }

    void flushEvent()
    {
        if (m_sseDataLines.isEmpty()) {
            m_sseEvent.clear();
            return;
        }

        const QString jsonStr = m_sseDataLines.join("\n");
        m_sseDataLines.clear();

        if (jsonStr.isEmpty() || jsonStr == "[DONE]") {
            m_sseEvent.clear();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            m_sseEvent.clear();
            return;
        }

        dispatchEvent(m_sseEvent, doc.object());
        m_sseEvent.clear();
    }

    void dispatchEvent(const QString &eventHint, const QJsonObject &obj)
    {
        if (!m_handler) return;

        // 事件类型可能在 data JSON 中，也可能通过 event: 行传递
        QString event = obj["event"].toString();
        if (event.isEmpty()) {
            event = eventHint;
        }

        m_handler(event, obj);
    }

    EventHandler m_handler;
    QString m_streamBuffer;
    QString m_sseEvent;
    QStringList m_sseDataLines;
};

#endif // SSESTREAMPARSER_H
