#include "StudentClassDetailWidget.h"
#include "StudentMaterialWidget.h"
#include "HomeworkManager.h"
#include "MaterialManager.h"
#include "../shared/StyleConfig.h"
#include "../settings/UserSettingsManager.h"
#include "../auth/supabase/supabaseconfig.h"
#include "../utils/NetworkRequestFactory.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>
#include <QDialog>
#include <QTextEdit>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QDesktopServices>
#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QProgressBar>
#include <QRandomGenerator>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QTextStream>

StudentClassDetailWidget::StudentClassDetailWidget(const ClassInfo &info, QWidget *parent)
    : QWidget(parent), m_classInfo(info)
    , m_studentEmail(UserSettingsManager::instance()->email())
    , m_studentName(UserSettingsManager::instance()->nickname())
    , m_networkManagerForUpload(new QNetworkAccessManager(this))
{
    setupUI();

    // ── 成员加载 ──
    connect(ClassManager::instance(), &ClassManager::membersLoaded, this,
        [this](const QString &classId, const QList<ClassManager::MemberInfo> &members) {
        if (classId != m_classInfo.id) return;

        while (m_memberLayout->count() > 1) {
            auto *item = m_memberLayout->takeAt(m_memberLayout->count() - 1);
            delete item->widget(); delete item;
        }

        // 教师行
        {
            auto *rowFrame = new QFrame();
            rowFrame->setObjectName("teacherRow");
            rowFrame->setStyleSheet(
                "#teacherRow { background: transparent; border: none; border-bottom: 1px solid #F3F4F6; border-radius: 0; }");
            auto *row = new QHBoxLayout(rowFrame); row->setSpacing(12);
            row->setContentsMargins(8, 10, 8, 10);

            auto *avatar = new QLabel(m_classInfo.teacher.left(1));
            avatar->setFixedSize(28, 28); avatar->setAlignment(Qt::AlignCenter);
            avatar->setStyleSheet("background: #F3F4F6; color: #4B5563; font-size: 13px; font-weight: 600; border-radius: 14px; border: none;");
            auto *nameL = new QLabel(m_classInfo.teacher);
            nameL->setStyleSheet("font-size: 13px; font-weight: 500; color: #111827; background: transparent; border: none;");
            auto *tag = new QLabel("教师");
            tag->setStyleSheet("font-size: 11px; color: #4B5563; background: transparent; padding: 2px 6px; border: 1px solid #D1D5DB; border-radius: 4px; font-weight: 500;");
            row->addWidget(avatar); row->addWidget(nameL); row->addStretch(); row->addWidget(tag);
            m_memberLayout->addWidget(rowFrame);
        }

        // 学生列表
        for (int i = 0; i < members.size(); ++i) {
            const auto &m = members[i];
            if (m.email == m_classInfo.teacherEmail) continue;

            auto *rowFrame = new QFrame();
            rowFrame->setObjectName(QString("stuRow_%1").arg(i));
            rowFrame->setStyleSheet(QString(
                "#stuRow_%1 { background: transparent; border: none; border-bottom: 1px solid #F3F4F6; border-radius: 0; }"
                "#stuRow_%1:hover { background: #F8FAFC; }").arg(i));
            auto *row = new QHBoxLayout(rowFrame); row->setSpacing(12);
            row->setContentsMargins(8, 10, 8, 10);

            auto *avatar = new QLabel(QString::number(i + 1));
            avatar->setFixedSize(28, 28); avatar->setAlignment(Qt::AlignCenter);
            avatar->setStyleSheet("background: #F3F4F6; color: #4B5563; font-size: 12px; font-weight: 600; border-radius: 14px; border: none;");
            QString display = m.name.isEmpty() ? m.email.split('@')[0] : m.name;
            if (!m.number.isEmpty()) display += "  (" + m.number + ")";
            auto *nameL = new QLabel(display);
            nameL->setStyleSheet("font-size: 13px; font-weight: 500; color: #111827; background: transparent; border: none;");
            row->addWidget(avatar); row->addWidget(nameL); row->addStretch();
            m_memberLayout->addWidget(rowFrame);
        }
        m_memberLayout->addStretch();
    });

    // ── 考勤加载 ──
    connect(AttendanceManager::instance(), &AttendanceManager::studentAttendanceLoaded, this,
        [this](const QString &classId, const QList<QPair<AttendanceManager::SessionInfo, QString>> &records) {
        if (classId != m_classInfo.id) return;

        while (m_attendanceLayout->count() > 1) {
            auto *item = m_attendanceLayout->takeAt(m_attendanceLayout->count() - 1);
            delete item->widget(); delete item;
        }

        if (records.isEmpty()) {
            auto *empty = new QLabel("暂无考勤记录");
            empty->setAlignment(Qt::AlignCenter);
            empty->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 40px; background: transparent; border: none;");
            m_attendanceLayout->addWidget(empty);
        } else {
            for (const auto &pair : records) {
                const auto &session = pair.first;
                QString status = pair.second;

                auto *card = new QFrame();
                card->setObjectName("attCard");
                card->setStyleSheet(
                    "#attCard { background: transparent; border: none; border-bottom: 1px solid #F3F4F6; }"
                    "#attCard:hover { background: #FAFAFA; }");
                auto *row = new QHBoxLayout(card); row->setSpacing(12);
                row->setContentsMargins(4, 10, 4, 10);

                QString name = session.name.isEmpty() ? "签到码 " + session.code : session.name;
                auto *nameL = new QLabel(name);
                nameL->setStyleSheet(QString("font-size: 14px; font-weight: 500; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));

                QString statusText, statusColor, statusBg;
                if (status == "present") { statusText = "已签到"; statusColor = "#059669"; statusBg = "#D1FAE5"; }
                else if (status == "late") { statusText = "迟到"; statusColor = "#D97706"; statusBg = "#FEF3C7"; }
                else if (status == "absent") { statusText = "缺勤"; statusColor = "#DC2626"; statusBg = "#FEE2E2"; }
                else if (status == "leave") { statusText = "请假"; statusColor = "#2563EB"; statusBg = "#DBEAFE"; }
                else { statusText = "未记录"; statusColor = "#9CA3AF"; statusBg = "#F3F4F6"; }

                auto *tag = new QLabel(statusText);
                tag->setStyleSheet(QString("font-size: 12px; font-weight: 600; color: %1; background: %2; padding: 3px 12px; border-radius: 10px; border: none;").arg(statusColor, statusBg));

                QString date = session.createdAt.toString("MM-dd HH:mm");
                auto *dateL = new QLabel(date);
                dateL->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");

                row->addWidget(nameL); row->addStretch(); row->addWidget(dateL); row->addWidget(tag);
                m_attendanceLayout->addWidget(card);
            }
        }
        m_attendanceLayout->addStretch();
    });

    // ── 作业加载 ──
    connect(HomeworkManager::instance(), &HomeworkManager::assignmentsLoaded, this,
        [this](const QList<HomeworkManager::AssignmentInfo> &list) {
        while (m_homeworkLayout->count() > 1) {
            auto *item = m_homeworkLayout->takeAt(m_homeworkLayout->count() - 1);
            delete item->widget(); delete item;
        }

        if (list.isEmpty()) {
            auto *empty = new QLabel("暂无作业");
            empty->setAlignment(Qt::AlignCenter);
            empty->setStyleSheet("font-size: 14px; color: #9CA3AF; padding: 40px; background: transparent; border: none;");
            m_homeworkLayout->addWidget(empty);
        } else {
            for (const auto &a : list) {
                connect(HomeworkManager::instance(), &HomeworkManager::submissionsLoaded, this,
                    [this, a](const QString &assignmentId, const QList<HomeworkManager::SubmissionInfo> &subs) {
                    if (assignmentId != a.id) return;

                    const HomeworkManager::SubmissionInfo *mySub = nullptr;
                    for (const auto &s : subs) {
                        if (s.studentEmail == m_studentEmail) { mySub = &s; break; }
                    }

                    bool exists = false;
                    for (int i = 0; i < m_homeworkLayout->count(); ++i) {
                        auto *w = m_homeworkLayout->itemAt(i)->widget();
                        if (w && w->property("hwId").toString() == a.id) { exists = true; break; }
                    }
                    if (exists) return;

                    auto *card = new QFrame();
                    card->setProperty("hwId", a.id);
                    card->setObjectName("hwCard");
                    card->setStyleSheet(
                        "#hwCard { background: white; border: 1px solid #E5E7EB; border-radius: 10px; }"
                        "#hwCard:hover { border-color: #E53935; }");
                    auto *layout = new QVBoxLayout(card); layout->setContentsMargins(16, 12, 16, 12); layout->setSpacing(6);

                    auto *titleRow = new QHBoxLayout();
                    auto *title = new QLabel(a.title);
                    title->setStyleSheet(QString("font-size: 15px; font-weight: 700; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));

                    QString timeText = a.endTime.isValid() ? "截止: " + a.endTime.toString("MM-dd HH:mm") : "无截止时间";
                    auto *timeL = new QLabel(timeText);
                    timeL->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
                    titleRow->addWidget(title); titleRow->addStretch(); titleRow->addWidget(timeL);
                    layout->addLayout(titleRow);

                    if (!a.description.isEmpty()) {
                        auto *desc = new QLabel(a.description.length() > 60 ? a.description.left(60) + "..." : a.description);
                        desc->setWordWrap(true);
                        desc->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent; border: none;");
                        layout->addWidget(desc);
                    }

                    auto *bottomRow = new QHBoxLayout();
                    if (mySub) {
                        // 已提交 — 显示状态
                        if (mySub->status == 2) {
                            auto *scoreL = new QLabel(QString("得分: %1 / %2").arg(mySub->score).arg(a.totalScore));
                            scoreL->setStyleSheet("font-size: 13px; font-weight: 600; color: #059669; background: transparent; border: none;");
                            bottomRow->addWidget(scoreL);
                            if (!mySub->feedback.isEmpty()) {
                                bottomRow->addSpacing(12);
                                auto *fb = new QLabel("反馈: " + mySub->feedback.left(30));
                                fb->setStyleSheet("font-size: 12px; color: #6B7280; background: transparent; border: none;");
                                bottomRow->addWidget(fb);
                            }
                        } else {
                            auto *tag = new QLabel("已提交 · 待批改");
                            tag->setStyleSheet("font-size: 12px; font-weight: 600; color: #D97706; background: #FEF3C7; padding: 3px 10px; border-radius: 10px; border: none;");
                            bottomRow->addWidget(tag);
                        }
                        bottomRow->addStretch();

                        // 查看提交内容按钮
                        auto *viewBtn = new QPushButton("查看");
                        viewBtn->setCursor(Qt::PointingHandCursor);
                        viewBtn->setFixedSize(50, 24);
                        viewBtn->setStyleSheet(
                            "QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px;"
                            "  color: #6B7280; font-size: 11px; }"
                            "QPushButton:hover { border-color: #E53935; color: #E53935; }");
                        HomeworkManager::SubmissionInfo viewSub = *mySub;
                        connect(viewBtn, &QPushButton::clicked, this, [this, viewSub]() {
                            showSubmissionDetailDialog(viewSub);
                        });
                        bottomRow->addWidget(viewBtn);
                    } else {
                        bool expired = a.endTime.isValid() && a.endTime < QDateTime::currentDateTime();
                        auto *submitBtn = new QPushButton(expired ? "已截止" : "提交作业");
                        submitBtn->setCursor(Qt::PointingHandCursor);
                        submitBtn->setFixedSize(90, 28);
                        submitBtn->setEnabled(!expired);
                        submitBtn->setStyleSheet(QString(
                            "QPushButton { background: %1; color: white; border: none; border-radius: 6px; font-size: 12px; font-weight: 600; }"
                            "QPushButton:hover { background: #C62828; }"
                            "QPushButton:disabled { background: #D1D5DB; color: #9CA3AF; }"
                        ).arg(StyleConfig::PATRIOTIC_RED));

                        QString assignmentId = a.id;
                        connect(submitBtn, &QPushButton::clicked, this, [this, assignmentId]() {
                            QDialog dlg(this);
                            dlg.setWindowTitle("提交作业");
                            dlg.setMinimumSize(480, 420);
                            dlg.setStyleSheet(QString("QDialog { background: %1; }").arg(StyleConfig::BG_APP));

                            auto *dlgLayout = new QVBoxLayout(&dlg);
                            dlgLayout->setSpacing(12);

                            auto *edit = new QTextEdit();
                            edit->setPlaceholderText("在此输入作业内容（可选）...");
                            edit->setStyleSheet(
                                "QTextEdit { border: 1px solid #E5E7EB; border-radius: 8px; padding: 8px; font-size: 14px; background: white; }"
                                "QTextEdit:focus { border-color: #E53935; }");
                            dlgLayout->addWidget(edit, 1);

                            // 文件选择行
                            auto *fileRow = new QHBoxLayout();
                            auto *fileBtn = new QPushButton("选择文件");
                            fileBtn->setCursor(Qt::PointingHandCursor);
                            fileBtn->setStyleSheet(
                                "QPushButton { background: white; border: 1px solid #E5E7EB; border-radius: 6px; padding: 6px 14px; font-size: 13px; color: #374151; }"
                                "QPushButton:hover { border-color: #E53935; color: #E53935; }");
                            auto *fileCountLabel = new QLabel("未选择文件");
                            fileCountLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
                            fileRow->addWidget(fileBtn);
                            fileRow->addWidget(fileCountLabel, 1);
                            dlgLayout->addLayout(fileRow);

                            // 已选文件列表区
                            auto *fileListWidget = new QWidget();
                            fileListWidget->setStyleSheet("background: transparent; border: none;");
                            auto *fileListLayout = new QVBoxLayout(fileListWidget);
                            fileListLayout->setContentsMargins(0, 0, 0, 0);
                            fileListLayout->setSpacing(4);
                            dlgLayout->addWidget(fileListWidget);

                            QStringList *selectedFiles = new QStringList();
                            connect(fileBtn, &QPushButton::clicked, &dlg,
                                [fileCountLabel, selectedFiles, fileListLayout]() {
                                QStringList paths = QFileDialog::getOpenFileNames(nullptr, "选择文件");
                                for (const auto &path : paths) {
                                    if (selectedFiles->contains(path)) continue;
                                    selectedFiles->append(path);
                                    QFileInfo fi(path);
                                    auto *row = new QHBoxLayout();
                                    auto *nameL = new QLabel(fi.fileName());
                                    nameL->setStyleSheet("font-size: 12px; color: #374151; background: transparent; border: none;");
                                    auto *removeBtn = new QPushButton("x");
                                    removeBtn->setFixedSize(20, 20);
                                    removeBtn->setCursor(Qt::PointingHandCursor);
                                    removeBtn->setStyleSheet(
                                        "QPushButton { background: transparent; border: none; color: #9CA3AF; font-size: 14px; }"
                                        "QPushButton:hover { color: #E53935; }");
                                    QString fp = path;
                                    connect(removeBtn, &QPushButton::clicked, nameL,
                                        [selectedFiles, fp, fileCountLabel, nameL, removeBtn]() {
                                        selectedFiles->removeOne(fp);
                                        nameL->deleteLater();
                                        removeBtn->deleteLater();
                                        fileCountLabel->setText(selectedFiles->isEmpty()
                                            ? "未选择文件" : QString("已选 %1 个文件").arg(selectedFiles->size()));
                                        fileCountLabel->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent; border: none;")
                                            .arg(selectedFiles->isEmpty() ? "#9CA3AF" : "#374151"));
                                    });
                                    row->addWidget(nameL); row->addStretch(); row->addWidget(removeBtn);
                                    fileListLayout->addLayout(row);
                                }
                                fileCountLabel->setText(selectedFiles->isEmpty()
                                    ? "未选择文件" : QString("已选 %1 个文件").arg(selectedFiles->size()));
                                fileCountLabel->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent; border: none;")
                                    .arg(selectedFiles->isEmpty() ? "#9CA3AF" : "#374151"));
                            });

                            auto *btnRow = new QHBoxLayout();
                            auto *cancelBtn = new QPushButton("取消");
                            cancelBtn->setStyleSheet(
                                "QPushButton { padding: 8px 24px; border: 1px solid #E5E7EB; border-radius: 8px;"
                                "  background: white; color: #6B7280; font-size: 14px; }"
                                "QPushButton:hover { background: #F9FAFB; }");
                            connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

                            auto *okBtn = new QPushButton("提交");
                            okBtn->setStyleSheet(QString(
                                "QPushButton { padding: 8px 24px; border: none; border-radius: 8px;"
                                "  background: %1; color: white; font-size: 14px; font-weight: 600; }"
                                "QPushButton:hover { background: #C62828; }"
                            ).arg(StyleConfig::PATRIOTIC_RED));
                            connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

                            btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
                            dlgLayout->addLayout(btnRow);

                            if (dlg.exec() == QDialog::Accepted) {
                                QString text = edit->toPlainText().trimmed();
                                if (text.isEmpty() && selectedFiles->isEmpty()) { delete selectedFiles; return; }

                                if (!selectedFiles->isEmpty()) {
                                    // 多文件逐个上传，全部完成后提交
                                    QStringList filesToUpload = *selectedFiles;
                                    delete selectedFiles;
                                    auto *uploadedUrls = new QStringList();
                                    auto *remaining = new int(filesToUpload.size());

                                    for (const QString &filePath : filesToUpload) {
                                        QFile file(filePath);
                                        if (!file.open(QIODevice::ReadOnly)) {
                                            (*remaining)--;
                                            if (*remaining == 0) {
                                                QString urls = uploadedUrls->join(",");
                                                delete uploadedUrls; delete remaining;
                                                {
                                                    QFile lf(QDir::homePath() + "/homework_upload.log");
                                                    if (lf.open(QIODevice::Append | QIODevice::Text)) {
                                                        QTextStream ts(&lf);
                                                        ts << QDateTime::currentDateTime().toString("HH:mm:ss")
                                                           << " SUBMIT urls=[" << urls << "] textLen=" << text.size() << "\n";
                                                    }
                                                }
                                                HomeworkManager::instance()->submitHomework(
                                                    assignmentId, m_studentEmail, m_studentName, text, urls);
                                            }
                                            continue;
                                        }
                                        QByteArray fileData = file.readAll();
                                        file.close();

                                        QFileInfo fi(filePath);
                                        QString storageName = QDateTime::currentDateTime().toString("yyyyMMddHHmmss_")
                                            + QString::number(qHash(fi.fileName()
                                                + QString::number(QRandomGenerator::global()->generate())), 16);
                                        QString ext = fi.suffix().isEmpty() ? "" : "." + fi.suffix();
                                        storageName += ext;

                                        QUrl storageUrl(SupabaseConfig::supabaseUrl()
                                            + "/storage/v1/object/homework/" + storageName);
                                        QNetworkRequest storageReq(storageUrl);
                                        storageReq.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
                                        storageReq.setRawHeader("Authorization",
                                            ("Bearer " + SupabaseConfig::supabaseAnonKey()).toUtf8());

                                        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
                                        QHttpPart filePart;
                                        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                            QVariant("form-data; name=\"file\"; filename=\"" + storageName + "\""));
                                        filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                                            QVariant("application/octet-stream"));
                                        filePart.setBody(fileData);
                                        multiPart->append(filePart);

                                        QNetworkReply *storageReply = m_networkManagerForUpload->post(storageReq, multiPart);
                                        multiPart->setParent(storageReply);

                                        connect(storageReply, &QNetworkReply::finished, this,
                                            [this, assignmentId, text, storageName, storageReply, uploadedUrls, remaining]() {
                                            storageReply->deleteLater();
                                            // 写日志
                                            {
                                                QFile logFile(QDir::homePath() + "/homework_upload.log");
                                                if (logFile.open(QIODevice::Append | QIODevice::Text)) {
                                                    QTextStream ts(&logFile);
                                                    ts << QDateTime::currentDateTime().toString("HH:mm:ss")
                                                       << " file:" << storageName
                                                       << " error:" << storageReply->error()
                                                       << " msg:" << storageReply->errorString()
                                                       << " status:" << storageReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                                                       << " body:" << storageReply->readAll().left(200)
                                                       << "\n";
                                                }
                                            }
                                            if (storageReply->error() == QNetworkReply::NoError) {
                                                uploadedUrls->append(SupabaseConfig::supabaseUrl()
                                                    + "/storage/v1/object/public/homework/" + storageName);
                                            }
                                            (*remaining)--;
                                            if (*remaining == 0) {
                                                QString urls = uploadedUrls->join(",");
                                                delete uploadedUrls; delete remaining;
                                                {
                                                    QFile lf(QDir::homePath() + "/homework_upload.log");
                                                    if (lf.open(QIODevice::Append | QIODevice::Text)) {
                                                        QTextStream ts(&lf);
                                                        ts << QDateTime::currentDateTime().toString("HH:mm:ss")
                                                           << " SUBMIT urls=[" << urls << "] textLen=" << text.size() << "\n";
                                                    }
                                                }
                                                HomeworkManager::instance()->submitHomework(
                                                    assignmentId, m_studentEmail, m_studentName, text, urls);
                                            }
                                        });
                                    }
                                } else {
                                    delete selectedFiles;
                                    HomeworkManager::instance()->submitHomework(
                                        assignmentId, m_studentEmail, m_studentName, text);
                                }
                            } else {
                                delete selectedFiles;
                            }
                        });
                        bottomRow->addStretch(); bottomRow->addWidget(submitBtn);
                    }
                    layout->addLayout(bottomRow);
                    m_homeworkLayout->addWidget(card);
                });

                HomeworkManager::instance()->loadSubmissions(a.id);
            }
        }
    });

    // 提交成功后刷新
    connect(HomeworkManager::instance(), &HomeworkManager::homeworkSubmitted, this, [this]() {
        loadHomework();
    });

    // 初始加载成员列表（默认显示成员页）
    loadMembers();
}

void StudentClassDetailWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(32, 28, 32, 28);
    mainLayout->setSpacing(20);

    // 头部
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("< 返回");
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet("QPushButton { background: transparent; border: none; color: #6B7280; font-size: 14px; padding: 4px 8px; } QPushButton:hover { color: #E53935; }");
    connect(backBtn, &QPushButton::clicked, this, &StudentClassDetailWidget::backRequested);

    auto *nameLabel = new QLabel(m_classInfo.name);
    nameLabel->setStyleSheet(QString("font-size: 24px; font-weight: 700; color: %1;").arg(StyleConfig::TEXT_PRIMARY));

    auto *teacherLabel = new QLabel("教师: " + m_classInfo.teacher);
    teacherLabel->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent; border: none;");

    headerLayout->addWidget(backBtn);
    headerLayout->addWidget(nameLabel);
    headerLayout->addWidget(teacherLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(QString("background: %1; max-height: 1px; border: none;").arg(StyleConfig::BORDER_LIGHT));
    mainLayout->addWidget(line);

    // 内容区
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(24);
    contentLayout->addWidget(createActionButtons());

    m_rightStack = new QStackedWidget();
    m_rightStack->addWidget(createMemberPanel());       // 0
    m_rightStack->addWidget(createAttendancePanel());    // 1
    m_rightStack->addWidget(createHomeworkPanel());      // 2
    contentLayout->addWidget(m_rightStack, 1);

    mainLayout->addLayout(contentLayout, 1);
}

QWidget* StudentClassDetailWidget::createActionButtons()
{
    auto *container = new QWidget();
    container->setFixedWidth(200);
    container->setStyleSheet("#navPanel { background: transparent; }");
    auto *layout = new QVBoxLayout(container);
    layout->setSpacing(2);
    layout->setContentsMargins(0, 0, 0, 0);

    struct Action { QString name; QString svgPath; };
    QList<Action> actions = {
        {"班级成员", ":/icons/resources/icons/users.svg"},
        {"考勤明细", ":/icons/resources/icons/attendance.svg"},
        {"我的作业", ":/icons/resources/icons/document.svg"},
        {"课程资料", ":/icons/resources/icons/folder.svg"},
    };

    bool isFirst = true;
    for (int i = 0; i < actions.size(); ++i) {
        const auto &act = actions[i];
        auto *btn = new QPushButton();
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(36);

        if (isFirst) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: #E5E7EB; border: none;"
                "  border-radius: 6px; padding: 4px 12px; text-align: left;"
                "}");
            isFirst = false;
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background: transparent; border: none;"
                "  border-radius: 6px; padding: 4px 12px; text-align: left;"
                "}"
                "QPushButton:hover { background: #F3F4F6; }");
        }

        auto *row = new QHBoxLayout(btn);
        row->setSpacing(8);
        row->setContentsMargins(8, 0, 8, 0);

        auto *iconLabel = new QLabel();
        QIcon svgIcon(act.svgPath);
        iconLabel->setPixmap(svgIcon.pixmap(16, 16));
        iconLabel->setFixedSize(16, 16);
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setStyleSheet("background: transparent; border: none;");

        auto *nameL = new QLabel(act.name);
        nameL->setStyleSheet(QString(
            "font-size: 13px; font-weight: %1; color: %2; background: transparent; border: none;"
        ).arg(i == 0 ? "600" : "500", i == 0 ? "#111827" : "#4B5563"));

        row->addWidget(iconLabel);
        row->addWidget(nameL);
        row->addStretch();

        int pageIdx = i;
        QString actName = act.name;
        connect(btn, &QPushButton::clicked, this, [this, pageIdx, actName]() {
            if (actName == "课程资料") {
                showMaterials();
            } else {
                while (m_rightStack->count() > 3) {
                    auto *w = m_rightStack->widget(m_rightStack->count() - 1);
                    m_rightStack->removeWidget(w); w->deleteLater();
                }
                m_rightStack->setCurrentIndex(pageIdx);
                if (actName == "班级成员") loadMembers();
                else if (actName == "考勤明细") loadAttendance();
                else if (actName == "我的作业") loadHomework();
            }
        });

        layout->addWidget(btn);
    }

    layout->addStretch();
    return container;
}

QWidget* StudentClassDetailWidget::createMemberPanel()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *panel = new QWidget();
    panel->setStyleSheet("background: transparent;");
    m_memberLayout = new QVBoxLayout(panel);
    m_memberLayout->setContentsMargins(0, 0, 0, 0);
    m_memberLayout->setSpacing(4);
    m_memberLayout->setAlignment(Qt::AlignTop);

    auto *titleRow = new QHBoxLayout();
    auto *title = new QLabel("班级成员");
    title->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));
    titleRow->addWidget(title); titleRow->addStretch();
    m_memberLayout->addLayout(titleRow);

    auto *placeholder = new QLabel("加载中...");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 32px; background: transparent; border: none;");
    m_memberLayout->addWidget(placeholder);

    scroll->setWidget(panel);
    return scroll;
}

QWidget* StudentClassDetailWidget::createAttendancePanel()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *panel = new QWidget();
    panel->setStyleSheet("background: transparent;");
    m_attendanceLayout = new QVBoxLayout(panel);
    m_attendanceLayout->setContentsMargins(0, 0, 0, 0);
    m_attendanceLayout->setSpacing(4);
    m_attendanceLayout->setAlignment(Qt::AlignTop);

    auto *titleRow = new QHBoxLayout();
    auto *title = new QLabel("考勤明细");
    title->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));
    titleRow->addWidget(title); titleRow->addStretch();
    m_attendanceLayout->addLayout(titleRow);

    auto *placeholder = new QLabel("加载中...");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("font-size: 13px; color: #9CA3AF; padding: 32px; background: transparent; border: none;");
    m_attendanceLayout->addWidget(placeholder);

    scroll->setWidget(panel);
    return scroll;
}

QWidget* StudentClassDetailWidget::createHomeworkPanel()
{
    auto *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto *panel = new QWidget();
    panel->setStyleSheet("background: transparent;");
    m_homeworkLayout = new QVBoxLayout(panel);
    m_homeworkLayout->setContentsMargins(0, 0, 0, 0);
    m_homeworkLayout->setSpacing(12);
    m_homeworkLayout->setAlignment(Qt::AlignTop);

    auto *titleRow = new QHBoxLayout();
    auto *title = new QLabel("我的作业");
    title->setStyleSheet(QString("font-size: 16px; font-weight: 700; color: %1; background: transparent; border: none;").arg(StyleConfig::TEXT_PRIMARY));
    titleRow->addWidget(title); titleRow->addStretch();
    m_homeworkLayout->addLayout(titleRow);

    scroll->setWidget(panel);
    return scroll;
}

void StudentClassDetailWidget::showMaterials()
{
    while (m_rightStack->count() > 3) {
        auto *w = m_rightStack->widget(m_rightStack->count() - 1);
        m_rightStack->removeWidget(w); w->deleteLater();
    }
    auto *widget = new StudentMaterialWidget(m_classInfo.id);
    connect(widget, &StudentMaterialWidget::backRequested, this, [this]() {
        while (m_rightStack->count() > 3) {
            auto *w = m_rightStack->widget(m_rightStack->count() - 1);
            m_rightStack->removeWidget(w); w->deleteLater();
        }
        m_rightStack->setCurrentIndex(0);
    });
    m_rightStack->addWidget(widget);
    m_rightStack->setCurrentWidget(widget);
}

void StudentClassDetailWidget::loadMembers()
{
    ClassManager::instance()->loadClassMembers(m_classInfo.id);
}

void StudentClassDetailWidget::loadAttendance()
{
    AttendanceManager::instance()->loadStudentAttendance(m_classInfo.id, m_studentEmail);
}

void StudentClassDetailWidget::loadHomework()
{
    HomeworkManager::instance()->loadAssignments(m_classInfo.id);
}

void StudentClassDetailWidget::showSubmissionDetailDialog(const HomeworkManager::SubmissionInfo &sub)
{
    QDialog dlg(this);
    dlg.setWindowTitle("提交详情");
    dlg.setMinimumSize(450, 400);
    dlg.setStyleSheet(QString("QDialog { background: %1; }").arg(StyleConfig::BG_APP));

    auto *layout = new QVBoxLayout(&dlg);
    layout->setSpacing(8);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setAlignment(Qt::AlignTop);

    // 状态
    if (sub.status == 2) {
        auto *row = new QHBoxLayout();
        auto *scoreL = new QLabel(QString("得分: %1").arg(sub.score));
        scoreL->setStyleSheet("font-size: 16px; font-weight: 700; color: #059669; background: transparent; border: none;");
        row->addWidget(scoreL);
        if (!sub.feedback.isEmpty()) {
            auto *fb = new QLabel("  反馈: " + sub.feedback);
            fb->setStyleSheet("font-size: 13px; color: #6B7280; background: transparent; border: none;");
            fb->setWordWrap(true);
            row->addWidget(fb, 1);
        }
        row->addStretch();
        layout->addLayout(row);
    } else {
        auto *tag = new QLabel("待批改");
        tag->setStyleSheet("font-size: 13px; font-weight: 600; color: #D97706; background: #FEF3C7; padding: 4px 12px; border-radius: 10px; border: none;");
        layout->addWidget(tag);
    }

    // 提交内容
    if (!sub.content.isEmpty()) {
        auto *titleLabel = new QLabel("提交内容:");
        titleLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #6B7280; background: transparent; border: none;");
        layout->addWidget(titleLabel);
        auto *contentLabel = new QLabel(sub.content);
        contentLabel->setWordWrap(true);
        contentLabel->setMaximumHeight(200);
        contentLabel->setStyleSheet("font-size: 14px; color: #1A1A1A; padding: 12px; background: #F9FAFB; border-radius: 8px; border: none;");
        layout->addWidget(contentLabel);
    }

    // 附件
    if (!sub.fileUrl.isEmpty()) {
        QStringList urls = sub.fileUrl.split(",", Qt::SkipEmptyParts);
        if (!urls.isEmpty()) {
            auto *fileTitle = new QLabel(QString("附件文件 (%1):").arg(urls.size()));
            fileTitle->setStyleSheet("font-size: 13px; font-weight: 600; color: #6B7280; background: transparent; border: none;");
            layout->addWidget(fileTitle);

            for (const QString &url : urls) {
                QString fileName = url.split("/").last();
                int usIdx = fileName.indexOf("_");
                if (usIdx > 0 && usIdx < 20) {
                    QString pn = fileName.mid(usIdx + 1);
                    if (!pn.isEmpty()) fileName = pn;
                }
                auto *row = new QHBoxLayout();
                auto *icon = new QLabel("📄");
                icon->setStyleSheet("font-size: 16px; background: transparent; border: none;");
                auto *nameL = new QLabel(fileName);
                nameL->setStyleSheet("font-size: 13px; color: #374151; background: transparent; border: none;");
                auto *dlBtn = new QPushButton("下载");
                dlBtn->setCursor(Qt::PointingHandCursor);
                dlBtn->setFixedHeight(24);
                dlBtn->setStyleSheet(
                    "QPushButton { background: transparent; border: 1px solid #E5E7EB; border-radius: 4px;"
                    "  color: #6B7280; font-size: 12px; padding: 2px 10px; }"
                    "QPushButton:hover { border-color: #E53935; color: #E53935; }");
                QString dlUrl = url;
                connect(dlBtn, &QPushButton::clicked, dlBtn, [dlUrl]() {
                    QDesktopServices::openUrl(QUrl(dlUrl));
                });
                row->addWidget(icon); row->addWidget(nameL, 1); row->addWidget(dlBtn);
                layout->addLayout(row);
            }
        }
    }

    auto *closeBtn = new QPushButton("关闭");
    closeBtn->setFixedHeight(36);
    closeBtn->setStyleSheet(
        "QPushButton { border: 1px solid #E5E7EB; border-radius: 8px; color: #6B7280; padding: 0 20px; }"
        "QPushButton:hover { background: #F9FAFB; }");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

void StudentClassDetailWidget::showResubmitDialog(const QString &submissionId)
{
    QDialog dlg(this);
    dlg.setWindowTitle("追加提交");
    dlg.setMinimumSize(480, 420);
    dlg.setStyleSheet(QString("QDialog { background: %1; }").arg(StyleConfig::BG_APP));

    auto *dlgLayout = new QVBoxLayout(&dlg);
    dlgLayout->setSpacing(12);

    auto *hint = new QLabel("追加内容将合并到原有提交中，不会覆盖原始内容。");
    hint->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
    dlgLayout->addWidget(hint);

    auto *edit = new QTextEdit();
    edit->setPlaceholderText("在此输入追加内容（可选）...");
    edit->setStyleSheet(
        "QTextEdit { border: 1px solid #E5E7EB; border-radius: 8px; padding: 8px; font-size: 14px; background: white; }"
        "QTextEdit:focus { border-color: #E53935; }");
    dlgLayout->addWidget(edit, 1);

    // 文件选择
    auto *fileRow = new QHBoxLayout();
    auto *fileBtn = new QPushButton("选择文件");
    fileBtn->setCursor(Qt::PointingHandCursor);
    fileBtn->setStyleSheet(
        "QPushButton { background: white; border: 1px solid #E5E7EB; border-radius: 6px; padding: 6px 14px; font-size: 13px; color: #374151; }"
        "QPushButton:hover { border-color: #E53935; color: #E53935; }");
    auto *fileCountLabel = new QLabel("未选择文件");
    fileCountLabel->setStyleSheet("font-size: 12px; color: #9CA3AF; background: transparent; border: none;");
    fileRow->addWidget(fileBtn);
    fileRow->addWidget(fileCountLabel, 1);
    dlgLayout->addLayout(fileRow);

    auto *fileListWidget = new QWidget();
    fileListWidget->setStyleSheet("background: transparent; border: none;");
    auto *fileListLayout = new QVBoxLayout(fileListWidget);
    fileListLayout->setContentsMargins(0, 0, 0, 0);
    fileListLayout->setSpacing(4);
    dlgLayout->addWidget(fileListWidget);

    QStringList *selectedFiles = new QStringList();
    connect(fileBtn, &QPushButton::clicked, &dlg,
        [fileCountLabel, selectedFiles, fileListLayout]() {
        QStringList paths = QFileDialog::getOpenFileNames(nullptr, "选择文件");
        for (const auto &path : paths) {
            if (selectedFiles->contains(path)) continue;
            selectedFiles->append(path);
            QFileInfo fi(path);
            auto *row = new QHBoxLayout();
            auto *nameL = new QLabel(fi.fileName());
            nameL->setStyleSheet("font-size: 12px; color: #374151; background: transparent; border: none;");
            auto *removeBtn = new QPushButton("x");
            removeBtn->setFixedSize(20, 20);
            removeBtn->setCursor(Qt::PointingHandCursor);
            removeBtn->setStyleSheet(
                "QPushButton { background: transparent; border: none; color: #9CA3AF; font-size: 14px; }"
                "QPushButton:hover { color: #E53935; }");
            QString fp = path;
            connect(removeBtn, &QPushButton::clicked, nameL,
                [selectedFiles, fp, fileCountLabel, nameL, removeBtn]() {
                selectedFiles->removeOne(fp);
                nameL->deleteLater();
                removeBtn->deleteLater();
                fileCountLabel->setText(selectedFiles->isEmpty()
                    ? "未选择文件" : QString("已选 %1 个文件").arg(selectedFiles->size()));
                fileCountLabel->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent; border: none;")
                    .arg(selectedFiles->isEmpty() ? "#9CA3AF" : "#374151"));
            });
            row->addWidget(nameL); row->addStretch(); row->addWidget(removeBtn);
            fileListLayout->addLayout(row);
        }
        fileCountLabel->setText(selectedFiles->isEmpty()
            ? "未选择文件" : QString("已选 %1 个文件").arg(selectedFiles->size()));
        fileCountLabel->setStyleSheet(QString("font-size: 12px; color: %1; background: transparent; border: none;")
            .arg(selectedFiles->isEmpty() ? "#9CA3AF" : "#374151"));
    });

    auto *btnRow = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("取消");
    cancelBtn->setStyleSheet(
        "QPushButton { padding: 8px 24px; border: 1px solid #E5E7EB; border-radius: 8px;"
        "  background: white; color: #6B7280; font-size: 14px; }"
        "QPushButton:hover { background: #F9FAFB; }");
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    auto *okBtn = new QPushButton("追加提交");
    okBtn->setStyleSheet(QString(
        "QPushButton { padding: 8px 24px; border: none; border-radius: 8px;"
        "  background: %1; color: white; font-size: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #C62828; }"
    ).arg(StyleConfig::PATRIOTIC_RED));
    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    btnRow->addStretch(); btnRow->addWidget(cancelBtn); btnRow->addWidget(okBtn);
    dlgLayout->addLayout(btnRow);

    if (dlg.exec() == QDialog::Accepted) {
        QString text = edit->toPlainText().trimmed();
        if (text.isEmpty() && selectedFiles->isEmpty()) { delete selectedFiles; return; }

        if (!selectedFiles->isEmpty()) {
            QStringList filesToUpload = *selectedFiles;
            delete selectedFiles;
            auto *uploadedUrls = new QStringList();
            auto *remaining = new int(filesToUpload.size());

            for (const QString &filePath : filesToUpload) {
                QFile file(filePath);
                if (!file.open(QIODevice::ReadOnly)) {
                    (*remaining)--;
                    if (*remaining == 0) {
                        QString urls = uploadedUrls->join(",");
                        delete uploadedUrls; delete remaining;
                        HomeworkManager::instance()->resubmitHomework(submissionId, text, urls);
                    }
                    continue;
                }
                QByteArray fileData = file.readAll();
                file.close();

                QFileInfo fi(filePath);
                QString storageName = QDateTime::currentDateTime().toString("yyyyMMddHHmmss_")
                    + QString::number(qHash(fi.fileName()
                        + QString::number(QRandomGenerator::global()->generate())), 16);
                QString ext = fi.suffix().isEmpty() ? "" : "." + fi.suffix();
                storageName += ext;

                QUrl storageUrl(SupabaseConfig::supabaseUrl()
                    + "/storage/v1/object/homework/" + storageName);
                QNetworkRequest storageReq(storageUrl);
                storageReq.setRawHeader("apikey", SupabaseConfig::supabaseAnonKey().toUtf8());
                storageReq.setRawHeader("Authorization",
                    ("Bearer " + SupabaseConfig::supabaseAnonKey()).toUtf8());

                QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
                QHttpPart filePart;
                filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                    QVariant("form-data; name=\"file\"; filename=\"" + storageName + "\""));
                filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                    QVariant("application/octet-stream"));
                filePart.setBody(fileData);
                multiPart->append(filePart);

                QNetworkReply *storageReply = m_networkManagerForUpload->post(storageReq, multiPart);
                multiPart->setParent(storageReply);

                connect(storageReply, &QNetworkReply::finished, this,
                    [this, submissionId, text, storageName, storageReply, uploadedUrls, remaining]() {
                    storageReply->deleteLater();
                    if (storageReply->error() == QNetworkReply::NoError) {
                        uploadedUrls->append(SupabaseConfig::supabaseUrl()
                            + "/storage/v1/object/public/homework/" + storageName);
                    }
                    (*remaining)--;
                    if (*remaining == 0) {
                        QString urls = uploadedUrls->join(",");
                        delete uploadedUrls; delete remaining;
                        HomeworkManager::instance()->resubmitHomework(submissionId, text, urls);
                    }
                });
            }
        } else {
            delete selectedFiles;
            HomeworkManager::instance()->resubmitHomework(submissionId, text, QString());
        }
    } else {
        delete selectedFiles;
    }
}
