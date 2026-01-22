#include "devicehandlerbase.h"
#include <QLabel>
#include <QLineEdit>
#include <QPalette>

const QString DeviceHandlerBase::ICON_SUCCESS = "./icon/selfcheck/success.png";
const QString DeviceHandlerBase::ICON_FAIL = "./icon/selfcheck/fail.png";
const QString DeviceHandlerBase::ICON_WAIT = "./icon/selfcheck/wait.png";

const QMap<ResultStatus, QPair<QString, QColor>> DeviceHandlerBase::STATUS_CONFIG = {
    {ResultStatus::Success,  {"成功", Qt::green}},
    {ResultStatus::Pending,  {"等待中", Qt::yellow}},
    {ResultStatus::Failed,   {"失败", Qt::red}},
    {ResultStatus::Timeout,  {"超时", QColor(255, 165, 0)}},
    {ResultStatus::Unknown,  {"未知状态", Qt::gray}}
};

DeviceHandlerBase::DeviceHandlerBase(SelfCheckModule* module, QWidget* parent)
    : QObject(parent)
    , m_module(module)
    , m_parent(parent)
{
}

void DeviceHandlerBase::updateWidgetText(QWidget* widget, const QString& text, const QColor& color)
{
    if (!widget) return;

    if (auto lineEdit = qobject_cast<QLineEdit*>(widget)) {
        lineEdit->setText(text);
        QPalette palette = lineEdit->palette();
        palette.setColor(QPalette::Text, color);
        lineEdit->setPalette(palette);
    }
    else if (auto label = qobject_cast<QLabel*>(widget)) {
        label->setText(text);
        QPalette palette = label->palette();
        palette.setColor(QPalette::WindowText, color);
        label->setPalette(palette);
    }
}

QPixmap DeviceHandlerBase::getStatusIcon(ResultStatus status)
{
    QPixmap icon;
    switch (status) {
    case ResultStatus::Success: icon.load(ICON_SUCCESS); break;
    case ResultStatus::Pending: icon.load(ICON_WAIT); break;
    default: icon.load(ICON_FAIL); break;
    }
    return icon;
}

void DeviceHandlerBase::setIcon(QLabel* label, const QPixmap& icon)
{
    if (label && !icon.isNull()) {
        label->setPixmap(icon.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void DeviceHandlerBase::updateStatusWidget(QWidget* widget, ResultStatus status, const QString& customText)
{
    QPair<QString, QColor> statusData = STATUS_CONFIG.value(status, STATUS_CONFIG[ResultStatus::Unknown]);
    QString showText = customText.isEmpty() ? statusData.first : customText;
    updateWidgetText(widget, showText, statusData.second);
}