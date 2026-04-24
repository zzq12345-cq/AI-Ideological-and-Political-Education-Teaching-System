#ifndef MODERNDIALOGHELPER_H
#define MODERNDIALOGHELPER_H

#include <QWidget>
#include <QString>

/**
 * ModernDialogHelper — 全局现代风格弹窗工具类
 *
 * 替代 QMessageBox / QInputDialog，提供无边框 + 圆角卡片 + 柔和阴影的弹窗体验。
 * 所有弹窗均为模态（exec），使用方法与 QMessageBox::question / information 类似。
 */
class ModernDialogHelper
{
public:
    /// 确认弹窗（⚠️ 图标 + 红色"确定"按钮）。返回 true 表示用户确认。
    static bool confirm(QWidget *parent, const QString &title, const QString &message);

    /// 信息提示弹窗（Indigo"知道了"按钮）
    static void info(QWidget *parent, const QString &title, const QString &message);

    /// 警告提示弹窗（橙色"知道了"按钮）
    static void warning(QWidget *parent, const QString &title, const QString &message);

    /// 输入弹窗（Indigo"确定"按钮）。返回用户输入的文本，取消则返回空字符串。
    static QString input(QWidget *parent, const QString &title, const QString &hint,
                         const QString &defaultText = QString());
};

#endif // MODERNDIALOGHELPER_H
