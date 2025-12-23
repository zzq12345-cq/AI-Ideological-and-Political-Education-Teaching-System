#include "PPTXGenerator.h"
#include <QJsonDocument>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <QCoreApplication>

PPTXGenerator::PPTXGenerator(QObject *parent)
    : QObject(parent)
{
    // 默认使用资源目录中的模板
    QString appDir = QCoreApplication::applicationDirPath();
    // macOS 应用包结构: .app/Contents/MacOS/
    QString templatePath = appDir + "/../../../resources/templates/party_red_template.pptx";
    if (QFile::exists(templatePath)) {
        m_templatePath = templatePath;
        qDebug() << "[PPTXGenerator] Template found:" << m_templatePath;
    } else {
        // 尝试开发模式路径
        templatePath = "/Users/zhouzhiqi/QtProjects/AItechnology/resources/templates/party_red_template.pptx";
        if (QFile::exists(templatePath)) {
            m_templatePath = templatePath;
            qDebug() << "[PPTXGenerator] Template found (dev):" << m_templatePath;
        } else {
            qDebug() << "[PPTXGenerator] No template found, using basic mode";
        }
    }
}

void PPTXGenerator::setTemplatePath(const QString &templatePath)
{
    if (QFile::exists(templatePath)) {
        m_templatePath = templatePath;
        qDebug() << "[PPTXGenerator] Template set to:" << m_templatePath;
    } else {
        qDebug() << "[PPTXGenerator] Template not found:" << templatePath;
    }
}

QJsonObject PPTXGenerator::parseJsonFromResponse(const QString &response)
{
    QString jsonStr = response;
    
    // 尝试提取 JSON 代码块中的内容
    QRegularExpression jsonBlockRegex("```(?:json)?\\s*([\\s\\S]*?)```");
    QRegularExpressionMatch match = jsonBlockRegex.match(response);
    if (match.hasMatch()) {
        jsonStr = match.captured(1).trimmed();
    }
    
    // 尝试直接解析
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        // 尝试找到 { 开头的 JSON
        int start = response.indexOf('{');
        int end = response.lastIndexOf('}');
        if (start >= 0 && end > start) {
            jsonStr = response.mid(start, end - start + 1);
            doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
        }
    }
    
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        return doc.object();
    }
    
    qDebug() << "[PPTXGenerator] JSON parse error:" << error.errorString();
    return QJsonObject();
}

bool PPTXGenerator::generateFromJsonString(const QString &jsonStr, const QString &outputPath)
{
    QJsonObject outline = parseJsonFromResponse(jsonStr);
    if (outline.isEmpty()) {
        m_lastError = "无法解析 JSON 大纲";
        return false;
    }
    return generateFromJson(outline, outputPath);
}

bool PPTXGenerator::generateFromJson(const QJsonObject &outline, const QString &outputPath)
{
    emit generationStarted();
    
    QString title = outline["title"].toString();
    QJsonArray slides = outline["slides"].toArray();
    
    if (title.isEmpty() || slides.isEmpty()) {
        m_lastError = "PPT 大纲缺少标题或幻灯片内容";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }
    
    qDebug() << "[PPTXGenerator] Generating PPT:" << title << "with" << slides.size() << "slides";
    
    // 如果有模板，使用模板模式
    if (!m_templatePath.isEmpty() && QFile::exists(m_templatePath)) {
        qDebug() << "[PPTXGenerator] Using template mode";
        return generateFromTemplate(outline, outputPath);
    }
    
    // 否则使用基础模式
    qDebug() << "[PPTXGenerator] Using basic mode (no template)";
    
    // 创建临时目录
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_lastError = "无法创建临时目录";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }
    
    QString basePath = tempDir.path();
    
    // 创建 PPTX 目录结构
    QDir().mkpath(basePath + "/_rels");
    QDir().mkpath(basePath + "/ppt/_rels");
    QDir().mkpath(basePath + "/ppt/slides/_rels");
    QDir().mkpath(basePath + "/ppt/slideLayouts/_rels");
    QDir().mkpath(basePath + "/ppt/slideMasters/_rels");
    QDir().mkpath(basePath + "/ppt/theme");
    QDir().mkpath(basePath + "/docProps");
    
    // 创建必要的 XML 文件
    int slideCount = slides.size() + 1; // +1 for title slide
    
    if (!createContentTypes(basePath, slideCount) ||
        !createRels(basePath) ||
        !createPresentation(basePath, slideCount) ||
        !createTheme(basePath) ||
        !createSlideMaster(basePath) ||
        !createSlideLayout(basePath)) {
        emit generationFinished(false, "");
        return false;
    }
    
    // 创建标题幻灯片
    QStringList titleContent;
    titleContent << outline["author"].toString("思政课堂");
    if (!createSlide(basePath, 1, title, titleContent, true)) {
        emit generationFinished(false, "");
        return false;
    }
    
    // 创建内容幻灯片
    for (int i = 0; i < slides.size(); ++i) {
        QJsonObject slide = slides[i].toObject();
        QString slideTitle = slide["title"].toString();
        QJsonArray contentArray = slide["content"].toArray();
        QStringList content;
        for (const QJsonValue &val : contentArray) {
            content << val.toString();
        }
        
        if (!createSlide(basePath, i + 2, slideTitle, content, false)) {
            emit generationFinished(false, "");
            return false;
        }
    }
    
    // 打包为 ZIP（PPTX）
    if (!packToZip(basePath, outputPath)) {
        emit generationFinished(false, "");
        return false;
    }
    
    qDebug() << "[PPTXGenerator] PPTX generated successfully:" << outputPath;
    emit generationFinished(true, outputPath);
    return true;
}

bool PPTXGenerator::createContentTypes(const QString &tempDir, int slideCount)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/ppt/presentation.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"/>
<Override PartName="/ppt/slideMasters/slideMaster1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"/>
<Override PartName="/ppt/slideLayouts/slideLayout1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"/>
<Override PartName="/ppt/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
)";
    
    for (int i = 1; i <= slideCount; ++i) {
        content += QString("<Override PartName=\"/ppt/slides/slide%1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.presentationml.slide+xml\"/>\n").arg(i);
    }
    
    content += "</Types>";
    
    QFile file(tempDir + "/[Content_Types].xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 [Content_Types].xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool PPTXGenerator::createRels(const QString &tempDir)
{
    QString content = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="ppt/presentation.xml"/>
</Relationships>)";
    
    QFile file(tempDir + "/_rels/.rels");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 .rels";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    file.close();
    return true;
}

bool PPTXGenerator::createPresentation(const QString &tempDir, int slideCount)
{
    QString slideList;
    QString relsList;
    
    for (int i = 1; i <= slideCount; ++i) {
        slideList += QString("<p:sldId id=\"%1\" r:id=\"rId%2\"/>").arg(255 + i).arg(i);
        relsList += QString("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide\" Target=\"slides/slide%1.xml\"/>\n").arg(i);
    }
    
    // presentation.xml
    QString presentation = QString(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:presentation xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main" saveSubsetFonts="1">
<p:sldMasterIdLst><p:sldMasterId id="2147483648" r:id="rId%1"/></p:sldMasterIdLst>
<p:sldIdLst>%2</p:sldIdLst>
<p:sldSz cx="9144000" cy="6858000" type="screen4x3"/>
<p:notesSz cx="6858000" cy="9144000"/>
</p:presentation>)").arg(slideCount + 1).arg(slideList);

    QFile presFile(tempDir + "/ppt/presentation.xml");
    if (!presFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 presentation.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream presOut(&presFile);
    presOut.setEncoding(QStringConverter::Utf8);
    presOut << presentation;
    presFile.close();
    
    // presentation.xml.rels
    QString rels = QString(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
%1<Relationship Id="rId%2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>
<Relationship Id="rId%3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="theme/theme1.xml"/>
</Relationships>)").arg(relsList).arg(slideCount + 1).arg(slideCount + 2);

    QFile relsFile(tempDir + "/ppt/_rels/presentation.xml.rels");
    if (!relsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 presentation.xml.rels";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream relsOut(&relsFile);
    relsOut.setEncoding(QStringConverter::Utf8);
    relsOut << rels;
    relsFile.close();
    
    return true;
}

bool PPTXGenerator::createSlide(const QString &tempDir, int index, const QString &title, const QStringList &content, bool isTitleSlide)
{
    QString escapedTitle = title.toHtmlEscaped();
    QString bodyContent;
    
    int yOffset = 1600000; // 起始 Y 位置
    for (const QString &item : content) {
        bodyContent += QString(R"(<a:p>
<a:pPr marL="342900" indent="-342900"><a:buChar char="•"/></a:pPr>
<a:r><a:rPr lang="zh-CN" sz="2400" dirty="0"/><a:t>%1</a:t></a:r>
</a:p>)").arg(item.toHtmlEscaped());
    }
    
    QString slideXml;
    if (isTitleSlide) {
        slideXml = QString(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld>
<p:spTree>
<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/></a:xfrm></p:grpSpPr>
<p:sp>
<p:nvSpPr><p:cNvPr id="2" name="Title"/><p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr><p:ph type="ctrTitle"/></p:nvPr></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="685800" y="2130425"/><a:ext cx="7772400" cy="1470025"/></a:xfrm></p:spPr>
<p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang="zh-CN" sz="4400" b="1" dirty="0"/><a:t>%1</a:t></a:r></a:p></p:txBody>
</p:sp>
<p:sp>
<p:nvSpPr><p:cNvPr id="3" name="Subtitle"/><p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr><p:ph type="subTitle" idx="1"/></p:nvPr></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="1371600" y="3886200"/><a:ext cx="6400800" cy="1752600"/></a:xfrm></p:spPr>
<p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang="zh-CN" sz="2000" dirty="0"/><a:t>%2</a:t></a:r></a:p></p:txBody>
</p:sp>
</p:spTree>
</p:cSld>
</p:sld>)").arg(escapedTitle).arg(content.isEmpty() ? "" : content.first().toHtmlEscaped());
    } else {
        slideXml = QString(R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sld xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld>
<p:spTree>
<p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>
<p:grpSpPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/></a:xfrm></p:grpSpPr>
<p:sp>
<p:nvSpPr><p:cNvPr id="2" name="Title"/><p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr><p:ph type="title"/></p:nvPr></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="457200" y="274638"/><a:ext cx="8229600" cy="1143000"/></a:xfrm></p:spPr>
<p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang="zh-CN" sz="3600" b="1" dirty="0"/><a:t>%1</a:t></a:r></a:p></p:txBody>
</p:sp>
<p:sp>
<p:nvSpPr><p:cNvPr id="3" name="Content"/><p:cNvSpPr><a:spLocks noGrp="1"/></p:cNvSpPr><p:nvPr><p:ph idx="1"/></p:nvPr></p:nvSpPr>
<p:spPr><a:xfrm><a:off x="457200" y="1600200"/><a:ext cx="8229600" cy="4525963"/></a:xfrm></p:spPr>
<p:txBody><a:bodyPr/><a:lstStyle/>%2</p:txBody>
</p:sp>
</p:spTree>
</p:cSld>
</p:sld>)").arg(escapedTitle).arg(bodyContent);
    }
    
    QFile slideFile(tempDir + QString("/ppt/slides/slide%1.xml").arg(index));
    if (!slideFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("无法创建 slide%1.xml").arg(index);
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream slideOut(&slideFile);
    slideOut.setEncoding(QStringConverter::Utf8);
    slideOut << slideXml;
    slideFile.close();
    
    // slide rels
    QString slideRels = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
</Relationships>)";
    
    QFile relsFile(tempDir + QString("/ppt/slides/_rels/slide%1.xml.rels").arg(index));
    if (!relsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("无法创建 slide%1.xml.rels").arg(index);
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream relsOut(&relsFile);
    relsOut.setEncoding(QStringConverter::Utf8);
    relsOut << slideRels;
    relsFile.close();
    
    return true;
}

bool PPTXGenerator::createTheme(const QString &tempDir)
{
    QString theme = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" name="思政课堂主题">
<a:themeElements>
<a:clrScheme name="Office">
<a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>
<a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>
<a:dk2><a:srgbClr val="1F497D"/></a:dk2>
<a:lt2><a:srgbClr val="EEECE1"/></a:lt2>
<a:accent1><a:srgbClr val="4F81BD"/></a:accent1>
<a:accent2><a:srgbClr val="C0504D"/></a:accent2>
<a:accent3><a:srgbClr val="9BBB59"/></a:accent3>
<a:accent4><a:srgbClr val="8064A2"/></a:accent4>
<a:accent5><a:srgbClr val="4BACC6"/></a:accent5>
<a:accent6><a:srgbClr val="F79646"/></a:accent6>
<a:hlink><a:srgbClr val="0000FF"/></a:hlink>
<a:folHlink><a:srgbClr val="800080"/></a:folHlink>
</a:clrScheme>
<a:fontScheme name="Office">
<a:majorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>
<a:minorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>
</a:fontScheme>
<a:fmtScheme name="Office">
<a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
<a:lnStyleLst><a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>
<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>
</a:fmtScheme>
</a:themeElements>
</a:theme>)";

    QFile file(tempDir + "/ppt/theme/theme1.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 theme1.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << theme;
    file.close();
    return true;
}

bool PPTXGenerator::createSlideMaster(const QString &tempDir)
{
    QString slideMaster = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldMaster xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main">
<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/></p:spTree></p:cSld>
<p:clrMap bg1="lt1" tx1="dk1" bg2="lt2" tx2="dk2" accent1="accent1" accent2="accent2" accent3="accent3" accent4="accent4" accent5="accent5" accent6="accent6" hlink="hlink" folHlink="folHlink"/>
<p:sldLayoutIdLst><p:sldLayoutId id="2147483649" r:id="rId1"/></p:sldLayoutIdLst>
</p:sldMaster>)";

    QFile file(tempDir + "/ppt/slideMasters/slideMaster1.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 slideMaster1.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << slideMaster;
    file.close();
    
    // slideMaster rels
    QString rels = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme" Target="../theme/theme1.xml"/>
</Relationships>)";
    
    QFile relsFile(tempDir + "/ppt/slideMasters/_rels/slideMaster1.xml.rels");
    if (!relsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 slideMaster1.xml.rels";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream relsOut(&relsFile);
    relsOut.setEncoding(QStringConverter::Utf8);
    relsOut << rels;
    relsFile.close();
    
    return true;
}

bool PPTXGenerator::createSlideLayout(const QString &tempDir)
{
    QString slideLayout = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<p:sldLayout xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" xmlns:p="http://schemas.openxmlformats.org/presentationml/2006/main" type="blank">
<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id="1" name=""/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/></p:spTree></p:cSld>
</p:sldLayout>)";

    QFile file(tempDir + "/ppt/slideLayouts/slideLayout1.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 slideLayout1.xml";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << slideLayout;
    file.close();
    
    // slideLayout rels
    QString rels = R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="../slideMasters/slideMaster1.xml"/>
</Relationships>)";
    
    QFile relsFile(tempDir + "/ppt/slideLayouts/_rels/slideLayout1.xml.rels");
    if (!relsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "无法创建 slideLayout1.xml.rels";
        emit errorOccurred(m_lastError);
        return false;
    }
    QTextStream relsOut(&relsFile);
    relsOut.setEncoding(QStringConverter::Utf8);
    relsOut << rels;
    relsFile.close();
    
    return true;
}

bool PPTXGenerator::packToZip(const QString &tempDir, const QString &outputPath)
{
    // 使用 zip 命令打包
    QProcess zipProcess;
    zipProcess.setWorkingDirectory(tempDir);
    
    // macOS/Linux 使用 zip 命令
    QStringList args;
    args << "-r" << outputPath << ".";
    
    zipProcess.start("zip", args);
    if (!zipProcess.waitForFinished(30000)) {
        m_lastError = "ZIP 打包超时";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    if (zipProcess.exitCode() != 0) {
        m_lastError = QString("ZIP 打包失败: %1").arg(QString::fromUtf8(zipProcess.readAllStandardError()));
        emit errorOccurred(m_lastError);
        return false;
    }
    
    return true;
}

// ========== 模板模式核心方法 ==========

bool PPTXGenerator::generateFromTemplate(const QJsonObject &outline, const QString &outputPath)
{
    QString title = outline["title"].toString();
    QString subtitle = outline["subtitle"].toString();
    if (subtitle.isEmpty()) {
        subtitle = outline["author"].toString("思政课堂");
    }
    QJsonArray slides = outline["slides"].toArray();
    
    qDebug() << "[PPTXGenerator] Template mode: title =" << title << ", slides =" << slides.size();
    
    // 创建临时目录
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        m_lastError = "无法创建临时目录";
        emit errorOccurred(m_lastError);
        emit generationFinished(false, "");
        return false;
    }
    
    QString extractPath = tempDir.path();
    
    // 解压模板
    if (!extractTemplate(m_templatePath, extractPath)) {
        emit generationFinished(false, "");
        return false;
    }
    
    // 获取模板中的幻灯片数量
    QDir slidesDir(extractPath + "/ppt/slides");
    QStringList slideFiles = slidesDir.entryList(QStringList() << "slide*.xml", QDir::Files);
    int templateSlideCount = slideFiles.size();
    qDebug() << "[PPTXGenerator] Template has" << templateSlideCount << "slides";
    
    // 替换第1页（封面）的标题
    QString slide1Path = extractPath + "/ppt/slides/slide1.xml";
    if (QFile::exists(slide1Path)) {
        replaceSlideContent(slide1Path, title, QStringList() << subtitle);
    }
    
    // 替换后续页面内容
    int contentSlideIndex = 2;  // 从第2页开始是内容页
    for (int i = 0; i < slides.size() && contentSlideIndex <= templateSlideCount; ++i) {
        QJsonObject slide = slides[i].toObject();
        QString slideTitle = slide["title"].toString();
        QJsonArray contentArray = slide["content"].toArray();
        QStringList content;
        for (const QJsonValue &val : contentArray) {
            content << val.toString();
        }
        
        QString slidePath = extractPath + QString("/ppt/slides/slide%1.xml").arg(contentSlideIndex);
        if (QFile::exists(slidePath)) {
            replaceSlideContent(slidePath, slideTitle, content);
            qDebug() << "[PPTXGenerator] Replaced slide" << contentSlideIndex << ":" << slideTitle;
        }
        
        contentSlideIndex++;
    }
    
    // 重新打包
    if (!packToZip(extractPath, outputPath)) {
        emit generationFinished(false, "");
        return false;
    }
    
    qDebug() << "[PPTXGenerator] Template PPT generated:" << outputPath;
    emit generationFinished(true, outputPath);
    return true;
}

bool PPTXGenerator::extractTemplate(const QString &templatePath, const QString &extractDir)
{
    QProcess unzipProcess;
    unzipProcess.setWorkingDirectory(extractDir);
    
    QStringList args;
    args << "-o" << templatePath << "-d" << extractDir;
    
    unzipProcess.start("unzip", args);
    if (!unzipProcess.waitForFinished(30000)) {
        m_lastError = "解压模板超时";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    if (unzipProcess.exitCode() != 0) {
        m_lastError = QString("解压模板失败: %1").arg(QString::fromUtf8(unzipProcess.readAllStandardError()));
        emit errorOccurred(m_lastError);
        return false;
    }
    
    qDebug() << "[PPTXGenerator] Template extracted to:" << extractDir;
    return true;
}

bool PPTXGenerator::replaceSlideContent(const QString &slideXmlPath, const QString &title, const QStringList &content)
{
    QFile file(slideXmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[PPTXGenerator] Cannot open slide:" << slideXmlPath;
        return false;
    }
    
    QString xmlContent = QString::fromUtf8(file.readAll());
    file.close();
    
    // 使用正则表达式找到并替换文本框中的内容
    // OOXML 中文本在 <a:t>文本</a:t> 标签内
    
    // 查找所有文本内容
    QRegularExpression textRegex("<a:t>([^<]*)</a:t>");
    QRegularExpressionMatchIterator it = textRegex.globalMatch(xmlContent);
    
    QStringList foundTexts;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString text = match.captured(1).trimmed();
        if (!text.isEmpty() && text.length() > 1) {
            foundTexts << text;
        }
    }
    
    qDebug() << "[PPTXGenerator] Found texts in slide:" << foundTexts.size();
    
    // 策略：替换第一个较长的文本为标题，后续文本替换为内容
    bool titleReplaced = false;
    int contentIndex = 0;
    
    for (const QString &oldText : foundTexts) {
        // 跳过很短的文本（可能是标点或装饰）
        if (oldText.length() < 2) {
            continue;
        }
        
        QString newText;
        if (!titleReplaced && !title.isEmpty()) {
            // 第一个长文本替换为标题
            newText = title;
            titleReplaced = true;
        } else if (contentIndex < content.size()) {
            // 后续文本替换为内容
            newText = content[contentIndex];
            contentIndex++;
        } else {
            // 没有更多内容，保留原文本
            continue;
        }
        
        // 执行替换
        xmlContent = replaceTextInXml(xmlContent, oldText, newText);
    }
    
    // 写回文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug() << "[PPTXGenerator] Cannot write slide:" << slideXmlPath;
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << xmlContent;
    file.close();
    
    return true;
}

QString PPTXGenerator::findTextInXml(const QString &xml)
{
    QRegularExpression textRegex("<a:t>([^<]+)</a:t>");
    QRegularExpressionMatch match = textRegex.match(xml);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}

QString PPTXGenerator::replaceTextInXml(const QString &xml, const QString &oldText, const QString &newText)
{
    // 转义 HTML 特殊字符
    QString escapedNew = newText.toHtmlEscaped();
    QString escapedOld = oldText.toHtmlEscaped();
    
    // 替换文本
    QString result = xml;
    QString oldTag = QString("<a:t>%1</a:t>").arg(oldText);
    QString newTag = QString("<a:t>%1</a:t>").arg(escapedNew);
    result.replace(oldTag, newTag);
    
    // 也尝试替换已转义的版本
    QString oldTagEscaped = QString("<a:t>%1</a:t>").arg(escapedOld);
    result.replace(oldTagEscaped, newTag);
    
    return result;
}
