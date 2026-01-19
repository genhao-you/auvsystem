
#pragma once
#include <QString>
#include<QWidget>
#include<QFile>
#include<qDebug>
// 样式加载工具类
class ModuleStyleHelper
{
public:
    // 静态方法：直接调用，不需要继承
    static void loadStyleSheet(const QString& fileName, QWidget* targetWidget) {
        if (!targetWidget) return;

        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QString styleSheet = QLatin1String(file.readAll());
            targetWidget->setStyleSheet(styleSheet);
            file.close();
        }
        else {
            qWarning() << "样式表加载失败：" << fileName << "，错误：" << file.errorString();
        }
    }
};