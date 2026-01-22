#include "widgetmanager.h"

WidgetManager& WidgetManager::instance()
{
    static WidgetManager instance;
    return instance;
}