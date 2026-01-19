#include "cs57geometry.h"


CS57Geometry::CS57Geometry(CS57Control* control,const QString& name)
	: QObject(nullptr)
	, CS57CanvasItem(control)
	, m_strName(name)
{}

CS57Geometry::~CS57Geometry()
{}

void CS57Geometry::paint(QPainter *p)
{}

//************************************
// Method:    setName
// Brief:	  设置名称
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name
//************************************
void CS57Geometry::setName(const QString& name)
{
	m_strName = name;
}

//************************************
// Method:    name
// Brief:	  获取名称
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57Geometry::name() const
{
	return m_strName;
}

