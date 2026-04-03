#include "SimpleZipWriter.h"

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QtEndian>
#include <QByteArray>
#include <QDebug>

// zlib 通过 Qt 内置依赖可用
#include <zlib.h>

QString SimpleZipWriter::s_lastError;

QString SimpleZipWriter::lastError()
{
    return s_lastError;
}

// ---------- ZIP 格式常量 ----------
static constexpr quint32 LOCAL_FILE_HEADER_SIG    = 0x04034b50;
static constexpr quint32 CENTRAL_DIR_HEADER_SIG   = 0x02014b50;
static constexpr quint32 END_OF_CENTRAL_DIR_SIG   = 0x06054b50;
static constexpr quint16 VERSION_MADE_BY          = 20;   // 2.0 — DEFLATE
static constexpr quint16 VERSION_NEEDED           = 20;
static constexpr quint16 METHOD_STORE             = 0;
static constexpr quint16 METHOD_DEFLATE           = 8;

// ---------- 辅助：DOS 日期时间 ----------
static void toDosDateTime(const QDateTime &dt, quint16 &dosDate, quint16 &dosTime)
{
    const QDate d = dt.date();
    const QTime t = dt.time();
    dosDate = static_cast<quint16>(((d.year() - 1980) << 9) | (d.month() << 5) | d.day());
    dosTime = static_cast<quint16>((t.hour() << 11) | (t.minute() << 5) | (t.second() / 2));
}

// ---------- 辅助：DEFLATE 压缩（raw deflate, 无 zlib/gzip 头） ----------
static QByteArray deflateData(const QByteArray &input, int level = Z_DEFAULT_COMPRESSION)
{
    if (input.isEmpty()) {
        return QByteArray();
    }

    z_stream strm;
    memset(&strm, 0, sizeof(strm));
    // -MAX_WBITS → raw deflate (no zlib header)
    if (deflateInit2(&strm, level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return QByteArray();
    }

    strm.avail_in = static_cast<uInt>(input.size());
    strm.next_in  = reinterpret_cast<Bytef*>(const_cast<char*>(input.constData()));

    QByteArray output;
    output.resize(deflateBound(&strm, strm.avail_in));

    strm.avail_out = static_cast<uInt>(output.size());
    strm.next_out  = reinterpret_cast<Bytef*>(output.data());

    int ret = deflate(&strm, Z_FINISH);
    deflateEnd(&strm);

    if (ret != Z_STREAM_END) {
        return QByteArray();
    }

    output.resize(static_cast<int>(strm.total_out));
    return output;
}

// ---------- 写入小端字节序 ----------
static void writeLE16(QByteArray &buf, quint16 v)
{
    char bytes[2];
    qToLittleEndian(v, bytes);
    buf.append(bytes, 2);
}

static void writeLE32(QByteArray &buf, quint32 v)
{
    char bytes[4];
    qToLittleEndian(v, bytes);
    buf.append(bytes, 4);
}

QStringList SimpleZipWriter::collectFiles(const QString &baseDir, const QString &subDir)
{
    QStringList result;
    const QString dirPath = subDir.isEmpty() ? baseDir : (baseDir + '/' + subDir);
    QDir dir(dirPath);

    const QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &fi : entries) {
        QString relativePath = subDir.isEmpty() ? fi.fileName() : (subDir + '/' + fi.fileName());
        if (fi.isDir()) {
            result += collectFiles(baseDir, relativePath);
        } else {
            result.append(relativePath);
        }
    }
    return result;
}

bool SimpleZipWriter::packDirectory(const QString &sourceDir, const QString &outputZipPath)
{
    s_lastError.clear();

    const QStringList files = collectFiles(sourceDir);
    if (files.isEmpty()) {
        s_lastError = QStringLiteral("源目录为空，没有可打包的文件");
        return false;
    }

    // 确保输出目录存在
    QDir().mkpath(QFileInfo(outputZipPath).absolutePath());

    QFile zipFile(outputZipPath);
    if (!zipFile.open(QIODevice::WriteOnly)) {
        s_lastError = QStringLiteral("无法创建输出文件: %1").arg(outputZipPath);
        return false;
    }

    struct CentralEntry {
        QByteArray header;
        quint32 localOffset;
    };
    QVector<CentralEntry> centralEntries;

    const QDateTime now = QDateTime::currentDateTime();
    quint16 dosDate, dosTime;
    toDosDateTime(now, dosDate, dosTime);

    for (const QString &relPath : files) {
        const QString absPath = sourceDir + '/' + relPath;
        QFile inputFile(absPath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            qWarning() << "[SimpleZipWriter] 无法读取文件:" << absPath;
            continue;
        }
        const QByteArray rawData = inputFile.readAll();
        inputFile.close();

        // ZIP 内部路径使用 '/'
        const QByteArray entryName = relPath.toUtf8();
        const quint32 crc = static_cast<quint32>(crc32(0L, reinterpret_cast<const Bytef*>(rawData.constData()),
                                                        static_cast<uInt>(rawData.size())));
        const quint32 uncompressedSize = static_cast<quint32>(rawData.size());

        // 尝试 DEFLATE 压缩
        QByteArray compressedData = deflateData(rawData);
        quint16 method = METHOD_DEFLATE;
        if (compressedData.isEmpty() || compressedData.size() >= rawData.size()) {
            // 压缩无效，使用 STORE
            compressedData = rawData;
            method = METHOD_STORE;
        }
        const quint32 compressedSize = static_cast<quint32>(compressedData.size());

        // ---------- Local File Header ----------
        const quint32 localOffset = static_cast<quint32>(zipFile.pos());

        QByteArray localHeader;
        localHeader.reserve(30 + entryName.size());
        writeLE32(localHeader, LOCAL_FILE_HEADER_SIG);
        writeLE16(localHeader, VERSION_NEEDED);
        writeLE16(localHeader, 0);              // general purpose bit flag
        writeLE16(localHeader, method);
        writeLE16(localHeader, dosTime);
        writeLE16(localHeader, dosDate);
        writeLE32(localHeader, crc);
        writeLE32(localHeader, compressedSize);
        writeLE32(localHeader, uncompressedSize);
        writeLE16(localHeader, static_cast<quint16>(entryName.size()));
        writeLE16(localHeader, 0);              // extra field length
        localHeader.append(entryName);

        zipFile.write(localHeader);
        zipFile.write(compressedData);

        // ---------- Central Directory Header ----------
        QByteArray centralHeader;
        centralHeader.reserve(46 + entryName.size());
        writeLE32(centralHeader, CENTRAL_DIR_HEADER_SIG);
        writeLE16(centralHeader, VERSION_MADE_BY);
        writeLE16(centralHeader, VERSION_NEEDED);
        writeLE16(centralHeader, 0);            // general purpose bit flag
        writeLE16(centralHeader, method);
        writeLE16(centralHeader, dosTime);
        writeLE16(centralHeader, dosDate);
        writeLE32(centralHeader, crc);
        writeLE32(centralHeader, compressedSize);
        writeLE32(centralHeader, uncompressedSize);
        writeLE16(centralHeader, static_cast<quint16>(entryName.size()));
        writeLE16(centralHeader, 0);            // extra field length
        writeLE16(centralHeader, 0);            // file comment length
        writeLE16(centralHeader, 0);            // disk number start
        writeLE16(centralHeader, 0);            // internal file attributes
        writeLE32(centralHeader, 0);            // external file attributes
        writeLE32(centralHeader, localOffset);
        centralHeader.append(entryName);

        centralEntries.append({centralHeader, localOffset});
    }

    // ---------- Central Directory ----------
    const quint32 centralDirOffset = static_cast<quint32>(zipFile.pos());
    quint32 centralDirSize = 0;

    for (const CentralEntry &entry : centralEntries) {
        zipFile.write(entry.header);
        centralDirSize += static_cast<quint32>(entry.header.size());
    }

    // ---------- End of Central Directory ----------
    QByteArray eocd;
    eocd.reserve(22);
    writeLE32(eocd, END_OF_CENTRAL_DIR_SIG);
    writeLE16(eocd, 0);    // number of this disk
    writeLE16(eocd, 0);    // disk where central directory starts
    writeLE16(eocd, static_cast<quint16>(centralEntries.size()));
    writeLE16(eocd, static_cast<quint16>(centralEntries.size()));
    writeLE32(eocd, centralDirSize);
    writeLE32(eocd, centralDirOffset);
    writeLE16(eocd, 0);    // ZIP file comment length

    zipFile.write(eocd);
    zipFile.close();

    qDebug() << "[SimpleZipWriter] ZIP 打包完成:" << outputZipPath
             << "文件数:" << centralEntries.size();
    return true;
}
