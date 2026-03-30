#ifndef PPTXGENERATOR_H
#define PPTXGENERATOR_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QImage>

/**
 * @brief PPTX 文件生成器
 * 
 * 根据 JSON 格式的 PPT 大纲生成真实的 PPTX 文件。
 * 当前统一走基础生成模式，不再依赖本地模板文件。
 */
class PPTXGenerator : public QObject
{
    Q_OBJECT

public:
    explicit PPTXGenerator(QObject *parent = nullptr);

    /**
     * @brief 从 JSON 生成 PPTX 文件（统一走基础生成模式）
     * @param outline JSON 格式的 PPT 大纲
     * @param outputPath 输出文件路径
     * @return 成功返回 true
     */
    bool generateFromJson(const QJsonObject &outline, const QString &outputPath);

    /**
     * @brief 从渲染后的页面图片生成 PPTX
     * @param title 演示文稿标题
     * @param images 每页对应一张渲染图
     * @param outputPath 输出文件路径
     * @return 成功返回 true
     */
    bool generateFromImages(const QString &title, const QVector<QImage> &images, const QString &outputPath);

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
    // 基础模式：创建 PPTX 所需的 XML 文件
    bool createContentTypes(const QString &tempDir, int slideCount);
    bool createRels(const QString &tempDir);
    bool createPresentation(const QString &tempDir, int slideCount);
    bool createSlide(const QString &tempDir, int index, const QString &title, const QStringList &content, bool isTitleSlide);
    bool createImageSlide(const QString &tempDir, int index, const QString &imageFileName);
    bool createTheme(const QString &tempDir);
    bool createSlideMaster(const QString &tempDir);
    bool createSlideLayout(const QString &tempDir);
    
    // 打包为 ZIP
    bool packToZip(const QString &tempDir, const QString &outputPath);

    QString m_lastError;
};

#endif // PPTXGENERATOR_H
