#pragma once
#include <QMap>
#include <QWidget>
#include <QString>
#include <QVariant>
#include <QStackedWidget>
class WidgetManager 
{
public:
    static WidgetManager& instance();

    // 获取控件（带缓存）
    template<typename T>
    T* getWidget(const QString& name, QWidget* parent) {
        if (!parent) {
            return nullptr;
        }

        QString key = QString("%1::%2::%3")
            .arg(parent->objectName())
            .arg(name)
            .arg(typeid(T).name());

        // 检查缓存
        typename QMap<QString, QWidget*>::iterator it = m_cache.find(key);  // 使用 typename
        if (it != m_cache.end()) {
            return qobject_cast<T*>(it.value());
        }



        // 首先在当前父控件中查找
        T* widget = parent->findChild<T*>(name);

        // 如果没有找到，检查是否是QStackedWidget，并在其所有页面中查找
        if (!widget) {
            QStackedWidget* stackedWidget = dynamic_cast<QStackedWidget*>(parent);  // 使用 dynamic_cast
            if (stackedWidget) {
                for (int i = 0; i < stackedWidget->count() && !widget; ++i) {
                    QWidget* page = stackedWidget->widget(i);
                    widget = page->findChild<T*>(name);
                    
                }
            }
        }

        if (!widget) {
          
            QList<QWidget*> childWidgets = parent->findChildren<QWidget*>();
        
            for (int j = 0; j < childWidgets.size(); ++j) {  // 使用索引循环
                QWidget* child = childWidgets.at(j);
          
            }
        }
        else {
        
            m_cache[key] = widget;
        }

        return widget;
    }

    // 批量获取控件
    template<typename T>
    QList<T*> getWidgets(const QString& pattern, QWidget* parent) {
        if (!parent) return QList<T*>();

        // 使用 typedef 避免 >> 语法问题
        typedef QList<T*> WidgetList;
        QString key = QString("%1::%2::list::%3")
            .arg(parent->objectName())
            .arg(pattern)
            .arg(typeid(T).name());

        // 检查列表缓存
        typename QMap<QString, QVariant>::iterator listIt = m_listCache.find(key);  // 使用 typename
        if (listIt != m_listCache.end()) {
            return listIt.value().value<WidgetList>();  // 这里也要注意
        }

        // 未缓存，查找并缓存
        QList<T*> widgets = parent->findChildren<T*>(pattern, Qt::FindDirectChildrenOnly);
        if (!widgets.isEmpty()) {
            // 同时缓存单个代表（用于快速访问）
            QString singleKey = QString("%1::%2::single::%3")
                .arg(parent->objectName())
                .arg(pattern)
                .arg(typeid(T).name());
            m_cache[singleKey] = widgets.first();

            // 缓存列表
            m_listCache[key] = QVariant::fromValue(widgets);
        }

        return widgets;
    }

    // 根据编号和类型获取控件（常用模式）
    template<typename T>
    T* getWidgetByNumber(const QString& number, const QString& widgetType,
        const QString& suffix, QWidget* parent) {
        QString name = QString("%1_%2_%3").arg(number).arg(widgetType).arg(suffix);
        return getWidget<T>(name, parent);
    }

    // 获取父控件的子控件（带类型检查）
    template<typename T>
    T* getChildWidget(QWidget* parent, const QString& name) {
        if (!parent) return nullptr;

        // 如果父控件有objectName，使用完整缓存键
        QString key;
        if (!parent->objectName().isEmpty()) {
            key = QString("%1::child::%2::%3")
                .arg(parent->objectName())
                .arg(name)
                .arg(typeid(T).name());
        }
        else {
            // 使用地址作为标识
            key = QString("%1::child::%2::%3")
                .arg(reinterpret_cast<quintptr>(parent))
                .arg(name)
                .arg(typeid(T).name());
        }

        // 检查缓存
        typename QMap<QString, QWidget*>::iterator it = m_cache.find(key);  // 使用 typename
        if (it != m_cache.end()) {
            return qobject_cast<T*>(it.value());
        }

        // 查找子控件
        T* widget = parent->findChild<T*>(name);
        if (widget) {
            m_cache[key] = widget;
        }
        return widget;
    }

    // 注册控件到缓存
    template<typename T>
    void registerWidget(const QString& key, T* widget) {
        if (widget) {
            m_cache[key] = widget;
        }
    }

    // 清理缓存
    void clearCache() {
        m_cache.clear();
        m_listCache.clear();
    }

    void clearCache(QWidget* parent) {
        if (!parent) return;

        QString parentName = parent->objectName();
        if (parentName.isEmpty()) {
            // 如果没有objectName，使用地址
            parentName = QString::number(reinterpret_cast<quintptr>(parent));
        }

        QString prefix = parentName + "::";

        // 删除所有以parent开头的缓存项
        typename QMap<QString, QWidget*>::iterator it = m_cache.begin();
        while (it != m_cache.end()) {
            if (it.key().startsWith(prefix)) {
                it = m_cache.erase(it);
            }
            else {
                ++it;
            }
        }

        typename QMap<QString, QVariant>::iterator listIt = m_listCache.begin();
        while (listIt != m_listCache.end()) {
            if (listIt.key().startsWith(prefix)) {
                listIt = m_listCache.erase(listIt);
            }
            else {
                ++listIt;
            }
        }
    }

    // 获取缓存统计信息（调试用）
    struct CacheStats {
        int widgetCount;
        int listCount;
        int memoryUsage; // 估算的内存使用（字节）
    };

    CacheStats getCacheStats() const {
        CacheStats stats;
        stats.widgetCount = m_cache.size();
        stats.listCount = m_listCache.size();

        // 估算内存使用（简化）
        stats.memoryUsage = (m_cache.size() * 100) + (m_listCache.size() * 200);

        return stats;
    }

private:
    WidgetManager() = default;
    ~WidgetManager() = default;

    // 禁止拷贝和赋值
    WidgetManager(const WidgetManager&) = delete;
    WidgetManager& operator=(const WidgetManager&) = delete;

    QMap<QString, QWidget*> m_cache;
    QMap<QString, QVariant> m_listCache;
};
