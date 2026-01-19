#pragma once
#include "control_global.h"
#include "cs57layertask.h"
#include <QImage>

class CONTROL_EXPORT CS57CanvasImage
{
public:
	CS57CanvasImage(QColor fillColor, QSize size, qreal dpiX, qreal dpiY);
	CS57CanvasImage(QImage* pImage,QPainter* pPainter);
	~CS57CanvasImage();
	//返回画布图像
	QImage*		canvasImage() const;
	//返回画布中心点
	QPoint		centerPt() const;
	//往画布图像上设置层任务
	void		setTask(CS57LayerTask* pTask);
	//渲染画布图形
	void		render();
	QPainter*   getCanvasImagePainter();
private:
	QPoint				m_CanvasCenterPt;
	QImage*				m_pImage;
	QPainter*			m_pPainter;		
	CS57LayerTask*		m_pLayerTask;
};
