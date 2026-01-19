#pragma once
#include "control_global.h"
#include "cs57canvas.h"
#include <QPainter>

class CS57Geometry : public QObject, public CS57CanvasItem
{
	Q_OBJECT
public:
	CS57Geometry(CS57Control* control,const QString& name);
	~CS57Geometry();

	void setName(const QString& name);
	QString name() const;
protected:
	void paint(QPainter *p) override;

private:
	QString		m_strName;
};
