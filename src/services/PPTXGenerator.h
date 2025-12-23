#ifndef PPTXGENERATOR_H
#define PPTXGENERATOR_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief PPTX 文件生成器
 * 
 * 根据 JSON 格式的 PPT 大纲生成真实的 PPTX 文件
 * 支持两种模式：
 * 1. 基础模式：从头创建简单 PPT
 * 2. 模板模式：基于精美模板替换文字内容
 */
class PPTXGenerator : public QObject
{
    Q_OBJECT

public:
    explicit PPTXGenerator(QObject *parent = nullptr);

    /**
     * @brief 设置 PPTX 模板路径
     * @param templatePath 模板文件路径
     */
    void setTemplatePath(const QString &templatePath);

    /**
     * @brief 从 JSON 生成 PPTX 文件（自动选择模板或基础模式）
     * @param outline JSON 格式的 PPT 大纲
     * @param outputPath 输出文件路径
     * @return 成功返回 true
     */
    bool generateFromJson(const QJsonObject &outline, const QString &outputPath);

    /**
     * @brief 从 JSON 字符串生成 PPTX
     */
    bool generateFromJsonString(const QString &jsonStr, const QString &outputPath);

    /**
     * @brief 解析 AI 响应中的 JSON
     * @param response AI 完整响应文本
     * @return 解析出的 JSON 对象，失败返回空对象
     */
    static QJsonObject parseJsonFromResponse(const QString &response);

    /**
     * @brief 获取最后的错误信息
     */
    QString lastError() const { return m_lastError; }

signals:
    void generationStarted();
    void generationFinished(bool success, const QString &filePath);
    void errorOccurred(const QString &error);

private:
    // 基于模板生成 PPT
    bool generateFromTemplate(const QJsonObject &outline, const QString &outputPath);
    bool extractTemplate(const QString &templatePath, const QString &extractDir);
    bool replaceSlideContent(const QString &slideXmlPath, const QString &title, const QStringList &content);
    QString findTextInXml(const QString &xml);
    QString replaceTextInXml(const QString &xml, const QString &oldText, const QString &newText);
    
    // 基础模式：创建 PPTX 所需的 XML 文件
    bool createContentTypes(const QString &tempDir, int slideCount);
    bool createRels(const QString &tempDir);
    bool createPresentation(const QString &tempDir, int slideCount);
    bool createSlide(const QString &tempDir, int index, const QString &title, const QStringList &content, bool isTitleSlide);
    bool createTheme(const QString &tempDir);
    bool createSlideMaster(const QString &tempDir);
    bool createSlideLayout(const QString &tempDir);
    
    // 打包为 ZIP
    bool packToZip(const QString &tempDir, const QString &outputPath);

    QString m_lastError;
    QString m_templatePath;  // 模板文件路径
};

#endif // PPTXGENERATOR_H

