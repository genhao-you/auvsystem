#pragma once

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QPair>
#include <QLabel>
#include "selfcheckmodule.h"
class DeviceHandlerBase  : public QObject
{
	Q_OBJECT

public:
    explicit DeviceHandlerBase(SelfCheckModule* module, QWidget* parent = nullptr);
    virtual ~DeviceHandlerBase() = default;

    // 虚函数接口
    virtual void handleResult(const SelfCheckResult * result) = 0;
    virtual void initializeUI() = 0;
    virtual void connectSignals() = 0;
    virtual void disconnectSignals() = 0;

    // 设备ID与编号映射
    virtual QString getDeviceNumber(DeviceId deviceId) const = 0;
    virtual bool supportsDevice(DeviceId deviceId) const = 0;

protected:
    // 工具方法
    void updateWidgetText(QWidget * widget, const QString & text, const QColor & color = Qt::black);
    QPixmap getStatusIcon(ResultStatus status);
    void setIcon(QLabel * label, const QPixmap & icon);
    void updateStatusWidget(QWidget * widget, ResultStatus status, const QString & customText = "");

    // 控件获取
    template<typename T>
    T* getWidget(const QString & pattern) const {
        return m_parent->findChild<T*>(pattern, Qt::FindDirectChildrenOnly);
    }

    template<typename T>
    T* getWidget(const QString & number, const QString & suffix) const {
        QString pattern = QString("%1_%2").arg(number).arg(suffix);
        return getWidget<T>(pattern);
    }

protected:
    SelfCheckModule* m_module;
    QWidget* m_parent;

    // 状态配置
    static const QMap<ResultStatus, QPair<QString, QColor>> STATUS_CONFIG;

private:
    // 图标路径
    static const QString ICON_SUCCESS;
    static const QString ICON_FAIL;
    static const QString ICON_WAIT;
};
