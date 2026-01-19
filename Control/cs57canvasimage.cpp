#include "cs57canvasimage.h"
#include <QPainter>

//显示模式使用
CS57CanvasImage::CS57CanvasImage(QColor fillColor, QSize size, qreal dpiX, qreal dpiY)
	: m_pImage(nullptr)
	, m_pPainter(nullptr)
	, m_pLayerTask(nullptr)
{
	m_pImage = new QImage(size, QImage::Format_ARGB32);
	m_pImage->setDevicePixelRatio(1.0);
	m_pImage->setDotsPerMeterX(1.0 * 1000 * dpiX / 25.4);
	m_pImage->setDotsPerMeterY(1.0 * 1000 * dpiY / 25.4);
	m_pImage->fill(fillColor);

	m_pPainter = new QPainter(m_pImage);

	m_CanvasCenterPt.setX(size.width() / 2);
	m_CanvasCenterPt.setY(size.height() / 2);

	//抗锯齿
	//m_pPainter->setRenderHint(QPainter::Antialiasing, true);
}

//打印模式使用
CS57CanvasImage::CS57CanvasImage(QImage* pImage, QPainter* pPainter)
	: m_pLayerTask(nullptr)
{
	m_pImage = pImage;
	m_pPainter = pPainter;
	
	m_CanvasCenterPt.setX(pImage->width() / 2);
	m_CanvasCenterPt.setY(pImage->height() / 2);
}

CS57CanvasImage::~CS57CanvasImage()
{
	if (m_pPainter != nullptr)
		delete m_pPainter;
	m_pPainter = nullptr;

	if (m_pImage != nullptr)
		delete m_pImage;
	m_pImage = nullptr;
}

//************************************
// Method:    canvasImage
// Brief:	  画布图像
// Returns:   QT_NAMESPACE::QImage*
// Author:    cl
// DateTime:  2022/07/21
//************************************
QImage* CS57CanvasImage::canvasImage() const
{
	return m_pImage;
}

//************************************
// Method:    centerPt
// Brief:	  画布中心点
// Returns:   QPoint
// Author:    cl
// DateTime:  2022/07/21
//************************************
QPoint CS57CanvasImage::centerPt() const
{
	return m_CanvasCenterPt;
}

//************************************
// Method:    setTask
// Brief:	  设置画布层任务
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57LayerTask * pTask
//************************************
void CS57CanvasImage::setTask(CS57LayerTask* pTask)
{
	m_pLayerTask = pTask;
}

//************************************
// Method:    render
// Brief:	  画布上绘制层
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57CanvasImage::render()
{
	if (m_pLayerTask)
	{
		m_pLayerTask->render(m_pPainter);
	}
}

QPainter* CS57CanvasImage::getCanvasImagePainter()
{
	return m_pPainter;
}



