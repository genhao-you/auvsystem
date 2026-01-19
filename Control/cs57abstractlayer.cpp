#include "cs57abstractlayer.h"


CS57AbstractLayer::CS57AbstractLayer()
	: _layerName("")
	, _layerVisible(true)
{}
CS57AbstractLayer::CS57AbstractLayer(const QString& name)
	: _layerName(name)
	, _layerVisible(true)
{}

CS57AbstractLayer::~CS57AbstractLayer()
{}

//************************************
// Method:    setLayerName
// Brief:	  …Ë÷√Õº≤„√˚≥∆
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & name
//************************************
void CS57AbstractLayer::setLayerName(const QString& name)
{
	_layerName = name;
}

//************************************
// Method:    getLayerName
// Brief:	  ªÒ»°Õº≤„√˚≥∆
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57AbstractLayer::getLayerName() const
{
	return _layerName;
}

//************************************
// Method:    setLayerVisible
// Brief:	  …Ë÷√Õº≤„œ‘“˛
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: bool visible
//************************************
void CS57AbstractLayer::setLayerVisible(bool visible)
{
	_layerVisible = visible;
}

//************************************
// Method:    getLayerVisible
// Brief:	  ªÒ»°Õº≤„œ‘“˛
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/21
//************************************
bool CS57AbstractLayer::getLayerVisible() const
{
	return _layerVisible;
}
