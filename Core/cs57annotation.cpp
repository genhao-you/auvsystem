#include "stdafx.h"
#include "cs57annotation.h"
#include "cs57recfeature.h"
#include "cs57projection.h"
#include "cs57viewport.h"
#include <QTextCodec>
#include <QVariant>
#include <QDebug>
#include "cs57auxiliaryrenderer.h"

using namespace Core;
#pragma execution_character_set("utf-8")
CS57AbstractAnnotation::CS57AbstractAnnotation()
{}

CS57AbstractAnnotation::~CS57AbstractAnnotation()
{}

CS57PointAnnotation::CS57PointAnnotation(CS57PresLib* preslib)
{
	m_pPresLib = preslib;
}

CS57PointAnnotation::~CS57PointAnnotation()
{}

void CS57PointAnnotation::setTE(showTextTE* pTE)
{
	m_pTE = pTE;
}

void CS57PointAnnotation::setTX(showTextTX* pTX)
{
	m_pTX = pTX;
}
void CS57PointAnnotation::renderTE(QPainter* painter, QPoint& pt, float scale)
{
	QString				formatstr;		
	QString				attrb;			
	char				hjust;// HJUST "水平调整"参数:‘1’表示中心对齐			 ‘2’表示右对齐   ‘3’表示左对齐（缺省值）			
	char				vjust;// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐 ‘3’表示向上对齐			
	char				space;			
	char				charsStyle;		
	char				charsWeight;	
	char				charsItalic;	
	unsigned short		charsSize;				
	QString				color;			
	unsigned short		textGroup;		
	double				xpos = 0.0, ypos = 0.0;

	formatstr = m_pTE->formatstr;
	attrb = m_pTE->attrb;
	hjust = m_pTE->hjust;
	vjust = m_pTE->vjust;

	int hAlign = hjust - '0';
	int vAligh = vjust - '0';

	painter->save();
	S57AttributeValue* pAttributeValue = m_pPresLib->getAttributeInfo(attrb);
	if (pAttributeValue == nullptr) return;

	float angle = 0.0;
	double sina = sin(angle / 180. * PI);
	double cosa = cos(angle / 180. * PI);
	QString strResultAnno = "";

	for (int i = 0; i < m_pFeature->m_Attf.m_vecAttf.size(); i++)
	{
		if (m_pFeature->m_Attf.m_vecAttf[i].attl == 117)
		{
			angle = m_pFeature->m_Attf.m_vecAttf[i].atvl.toFloat();

			angle = 90. - angle;
			sina = sin(angle / 180. * PI);
			cosa = cos(angle / 180. * PI);
			if (angle < 0.)
				angle += 360.;
			if (angle > 360.)
				angle -= 360.;
			if (angle >= 0 && angle <= 90 ||
				angle >= 270 && angle <= 360)
			{}

			if (angle > 90 && angle < 270)
			{
				angle += 180.;
			}
			continue;
		}

		if (m_pFeature->m_Attf.m_vecAttf[i].attl == pAttributeValue->nFeaAttrCode)
		{
			int nVal = 0;
			float fVal = 0.0;
			QString strVal = "";
			char szText[100] = { '\0' };
			switch (pAttributeValue->eAttrType)
			{
			case F:
				fVal = m_pFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
				sprintf(szText, (const char*)formatstr.toLocal8Bit(), fVal);
				break;
			case I:
				nVal = m_pFeature->m_Attf.m_vecAttf[i].atvl.toInt();
				sprintf(szText, (const char*)formatstr.toLocal8Bit(), nVal);
				break;
			case S:
				strVal = m_pFeature->m_Attf.m_vecAttf[i].atvl;
				sprintf(szText, (const char*)formatstr.toLocal8Bit(), (const char*)strVal.toLocal8Bit());
				break;
			}
			int len = strlen(szText);
			szText[len] = '\0';
			string text = szText;
			QString strText = QString::fromLocal8Bit(text.c_str());
			strResultAnno = strText;
		}
	}
	float dpm_x = painter->device()->physicalDpiX() / 25.4;
	
	charsSize = m_pTE->charsSize * 0.351 * dpm_x * scale;
	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(m_pTE->color, r, g, b);

	double offsetX = charsSize * m_pTE->xoffs;
	double offsetY = charsSize * m_pTE->yoffs;
	textGroup = m_pTE->textGroup;

	QFont font;
	font.setFamily("Times New Roman");
	font.setPixelSize(charsSize);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);

	int textWidth = painter->fontMetrics().width(strResultAnno);
	int textHeight = painter->fontMetrics().height();

	QPen pen;
	pen.setColor(QColor(r, g, b));
	painter->setPen(pen);

	switch (hAlign)
	{
	case 1://中心对齐
		xpos = (double)textWidth * -0.5;
		break;
	case 2://右对齐
		xpos = (double)textWidth * -1;
		break;
	case 3://左对齐
		xpos = 0;
		break;
	}

	switch (vAligh)
	{
	case 1://下对齐
		ypos = (double)textHeight * -1;
		break;
	case 2://中心对齐
		ypos = (double)textHeight * -0.5;
		break;
	case 3://上对齐
		ypos = 0;
		break;
	}

	if (m_pFeature->m_Frid.objl == 160|| m_pFeature->m_Frid.objl == 36)
	{
		xpos = (double)textWidth * -0.5;
		offsetX = 0;
		offsetY = 0;
	}
	QPoint tePoint;
	tePoint = pt;
	painter->translate(tePoint.x(), tePoint.y());
	painter->rotate(-angle);

	QRectF textRect;
	textRect.setX(offsetX + xpos);
	textRect.setY(offsetY + ypos);
	textRect.setWidth((double)textWidth);
	textRect.setHeight((double)textHeight);
	painter->drawText(textRect, 0, strResultAnno);
	//painter->drawRect(textRect);

	painter->restore();

	//CS57AuxiliaryRenderer aux;
	//aux.renderCrossLine(painter, tePoint);
}
void CS57PointAnnotation::renderTX(QPainter* painter, QPoint& pt, float scale)
{
	QString				str;				
	char				hjust = '3';		
	char				vjust = '1';		
	char				space = '2';		
	char				charsStyle = '1';	
	char				charsWeight = '5';	
	char				charsItalic = '1';	
	unsigned short		charsSize = 10;		
	short				xoffs = 0;			
	short				yoffs = 0;			
	QString				color;				
	unsigned short		textGroup = 0;	
	double				xpos = 0.0, ypos = 0.0;


	QString strResultAnno = "";
	if (m_pTX->str.length() == 6)
	{
		bool isInform = false;
		bool isObjnam = false;
		S57AttributeValue* pAttributeValue = m_pPresLib->getAttributeInfo(m_pTX->str);
		if (pAttributeValue == nullptr) 
			return;

		for (int i = 0; i < m_pFeature->m_Natf.m_vecNatf.size(); i++)
		{
			if (m_pFeature->m_Natf.m_vecNatf[i].attl == 300 ||
				m_pFeature->m_Natf.m_vecNatf[i].attl == 301 ||
				m_pFeature->m_Natf.m_vecNatf[i].attl == 302 ||
				m_pFeature->m_Natf.m_vecNatf[i].attl == 303 ||
				m_pFeature->m_Natf.m_vecNatf[i].attl == 304)
			{
				QTextCodec* codec_16 = QTextCodec::codecForName("UTF-16");
				QString strValue = codec_16->toUnicode(m_pFeature->m_Natf.m_vecNatf[i].atvl);
				strResultAnno.append(" ").append(strValue);
			}
		}
		for (int i = 0; i < m_pFeature->m_Attf.m_vecAttf.size(); i++)
		{
			if (!isInform && !isObjnam && m_pFeature->m_Attf.m_vecAttf[i].attl == pAttributeValue->nFeaAttrCode)
			{
				int nVal = 0;
				float fVal = 0.0;
				QString strVal = "";
				switch (pAttributeValue->eAttrType)
				{
				case E:
					break;
				case L:
				{
					QString cName;
					QString srcStr = m_pFeature->m_Attf.m_vecAttf[i].atvl;
					QStringList values = srcStr.split(",");
					if (values.size() == 0)
						continue;

					if (values.size() == 1)
					{
						cName = m_pPresLib->getS57AttrExpValueInfo(pAttributeValue->nFeaAttrCode, values[0].toUShort())->strCName;
					}
					else
					{
						cName = m_pPresLib->getS57AttrExpValueInfo(pAttributeValue->nFeaAttrCode, values[0].toUShort())->strCName;
						for (int j = 1; j < values.size(); j++)
						{
							cName.append(",").append(m_pPresLib->getS57AttrExpValueInfo(pAttributeValue->nFeaAttrCode, values[j].toUShort())->strCName);
						}
					}
					strResultAnno.append(" ").append(cName);
					break;
				}
				case F:
					fVal = m_pFeature->m_Attf.m_vecAttf[i].atvl.toFloat();
					strResultAnno.append(" ").append(QString::number(fVal));
					break;
				case I:
					nVal = m_pFeature->m_Attf.m_vecAttf[i].atvl.toInt();
					strResultAnno.append(" ").append(QString::number(nVal));
					break;
				case A:
					break;
				case S:
					strVal = m_pFeature->m_Attf.m_vecAttf[i].atvl;
					strResultAnno.append(" ").append(strVal);
					break;
				}
			}
		}
	}
	else
	{
		QString str = m_pTX->str;
		strResultAnno.append(" ").append(str.mid(1, str.length() - 2));
	}
	hjust = m_pTX->hjust;
	vjust = m_pTX->vjust;

	int hAlign = hjust - '0';
	int vAligh = vjust - '0';

	painter->save();

	float dpm_x = painter->device()->physicalDpiX() / 25.4;
	charsSize = m_pTX->charsSize * 0.351 * dpm_x * scale;
	double offsetX = charsSize * m_pTX->xoffs;
	double offsetY = charsSize * m_pTX->yoffs;
	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(m_pTX->color, r, g, b);

	space = m_pTX->space;
	charsStyle = m_pTX->charsStyle;
	charsWeight = m_pTX->charsWeight;

	if (charsSize < 5)
		return;

	textGroup = m_pTX->textGroup;
	QPoint txPoint;
	txPoint = pt;

	QFont font;
	font.setPixelSize(charsSize);
	font.setStyleStrategy(QFont::PreferAntialias);
	switch (m_pFeature->m_Frid.objl)
	{
	case 121:
		font.setFamily("长细等线");
		break;
	default:
		font.setFamily("Times New Roman");
		break;
	}
	painter->setFont(font);

	QPen pen;
	pen.setColor(QColor(r, g, b));
	painter->setPen(pen);

	painter->translate(txPoint.x(), txPoint.y());
	int textWidth = painter->fontMetrics().width(strResultAnno);
	int textHeight = painter->fontMetrics().height();


	switch (hAlign)
	{
	case 1://中心对齐
		xpos = (double)textWidth * -0.5;
		break;
	case 2://右对齐
		xpos = (double)textWidth * -1;
		break;
	case 3://左对齐
		xpos = 0;
		break;
	}

	switch (vAligh)
	{
	case 1://下对齐
		ypos = (double)textHeight * -1;
		break;
	case 2://中心对齐
		ypos = (double)textHeight * -0.5;
		break;
	case 3://上对齐
		ypos = 0;
		break;
	}

	QRectF textRect;
	textRect.setX(offsetX + xpos);
	textRect.setY(offsetY + ypos);
	textRect.setWidth((double)textWidth);
	textRect.setHeight((double)textHeight);
	painter->drawText(textRect, 0, strResultAnno);
	//painter->drawRect(textRect); //绘制矩形框

	painter->restore();
}

void CS57PointAnnotation::setAnnoFeature(CS57RecFeature* pFE)
{
	m_pFeature = pFE;
}

CS57LineAnnotation::CS57LineAnnotation(CS57PresLib* preslib)
{
	m_pPresLib = preslib;
}

CS57LineAnnotation::~CS57LineAnnotation()
{}

void CS57LineAnnotation::setTE(showTextTE* pTE)
{
	m_pTE = pTE;
}

void CS57LineAnnotation::setTX(showTextTX* pTX)
{
	m_pTX = pTX;
}
void CS57LineAnnotation::setAnnoFeature(CS57RecFeature* pFE)
{
	m_pFeature = pFE;
}

void CS57LineAnnotation::renderTE(QPainter* painter, CPolylines& polylines, QString strAnno, float angle, float scale)
{
	int hjust;				// HJUST "水平调整"参数:‘1’表示中心对齐‘2’表示右对齐‘3’表示左对齐（缺省值）
	int vjust;				// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐‘3’表示向上对齐
	int space;				// SPACE 字符间距参数: 1’表示撑满（FIT）间距‘2’表示标准间距，缺省值	‘3’表示带有字框的标准间距
	char charsStyle;		// 字体	"1" 简单，sans serif字体
	int charsWeight;		// 字粗	4 细体	5 中粗体，这是缺省值。	6 粗体
	char charsWidth;		// 字宽	1表示正直朝上（非斜体），ENC $CHARS属性用"2"时必须改为 "1"。
	float charsSize;		// 字号以pica points为单位给定(1点=0.351mm)，pica点规定了大写字母的高度，最小字号是pica10，并且这也默认大小，允许使用较大的字号。
	short xoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点X-偏移 (如果XOFFS未给定或定义则缺省为0);正向X偏移向右延伸(字号单位意味着，如果字号为10个pica点，则每单位的偏移是10(0.351)=3.51mm)
	short yoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点的Y-偏移 (如果YOFFS未给定或定义则缺省为0);正向Y偏移向下延伸。
	char color[6];			// 颜色标识
	unsigned short textGroup;		// 文本组合代码
	double	xpos = 0.0, ypos = 0.0;

	hjust = m_pTE->hjust;
	vjust = m_pTE->vjust;

	int hAlign = hjust - '0';
	int vAligh = vjust - '0';

	float dpm_x = painter->device()->physicalDpiX() / 25.4;
	space = m_pTE->space;
	charsStyle = m_pTE->charsStyle;
	charsWeight = m_pTE->charsWeight;
	charsSize = m_pTE->charsSize * 0.351 * dpm_x * scale;
	double offsetX = charsSize * m_pTE->xoffs;
	double offsetY = charsSize * m_pTE->yoffs;
	//xoffs = m_pTE->xoffs * 0.351 * dpm_x;
	//yoffs = m_pTE->yoffs * 0.351 * dpm_x;
	textGroup = m_pTE->textGroup;

	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(m_pTE->color, r, g, b);

	angle = 90. - angle;

	QPoint p1, p2;
	calcLinePts(polylines, p1, p2);

	QPoint tePoint;
	tePoint.setX((p1.x() + p2.x()) / 2);
	tePoint.setY((p1.y() + p2.y()) / 2);

	painter->save();

	painter->translate(tePoint.x(), tePoint.y());
	painter->rotate(-angle);

	QFont font;
	font.setFamily("Times New Roman");
	font.setPixelSize(charsSize);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);

	QPen pen;
	pen.setColor(QColor(r, g, b));
	painter->setPen(pen);

	int textWidth = painter->fontMetrics().width(strAnno);
	int textHeight = painter->fontMetrics().height();

	switch (hAlign)
	{
	case 1://中心对齐
		xpos = (double)textWidth * -0.5;
		break;
	case 2://右对齐
		xpos = (double)textWidth * -1;
		break;
	case 3://左对齐
		xpos = 0;
		break;
	}

	switch (vAligh)
	{
	case 1://下对齐
		ypos = (double)textHeight * -1;
		break;
	case 2://中心对齐
		ypos = (double)textHeight * -0.5;
		break;
	case 3://上对齐
		ypos = 0;
		break;
	}
	offsetY = 0;
	//if (m_pFeature->m_Frid.objl == 85)
	//{
	//	offsetY = 0;
	//}

	QRectF textRect;
	textRect.setX(offsetX + xpos);
	textRect.setY(offsetY + ypos);
	textRect.setWidth((double)textWidth);
	textRect.setHeight((double)textHeight);
	painter->drawText(textRect, 0, strAnno);
	//painter->drawRect(textRect);

	painter->restore();
}

void CS57LineAnnotation::renderTX(QPainter* painter, CPolylines& polylines, QString strAnno, float angle, float scale)
{
	int hjust;				// HJUST "水平调整"参数:‘1’表示中心对齐‘2’表示右对齐‘3’表示左对齐（缺省值）
	int vjust;				// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐‘3’表示向上对齐
	int space;				// SPACE 字符间距参数: 1’表示撑满（FIT）间距‘2’表示标准间距，缺省值	‘3’表示带有字框的标准间距
	char charsStyle;		// 字体	"1" 简单，sans serif字体
	int charsWeight;		// 字粗	4 细体	5 中粗体，这是缺省值。	6 粗体
	float charsSize;		// 字号以pica points为单位给定(1点=0.351mm)，pica点规定了大写字母的高度，最小字号是pica10，并且这也默认大小，允许使用较大的字号。
	short xoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点X-偏移 (如果XOFFS未给定或定义则缺省为0);正向X偏移向右延伸(字号单位意味着，如果字号为10个pica点，则每单位的偏移是10(0.351)=3.51mm)
	short yoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点的Y-偏移 (如果YOFFS未给定或定义则缺省为0);正向Y偏移向下延伸。
	char color[6];			// 颜色标识
	unsigned short textGroup;		// 文本组合代码
	double	xpos = 0.0, ypos = 0.0;

	float dpm_x = painter->device()->physicalDpiX() / 25.4;

	hjust = m_pTX->hjust;
	vjust = m_pTX->vjust;

	int hAlign = hjust - '0';
	int vAligh = vjust - '0';

	space = m_pTX->space;
	charsStyle = m_pTX->charsStyle;
	charsWeight = m_pTX->charsWeight;
	charsSize = m_pTX->charsSize * 0.351 * dpm_x * scale;
	double offsetX = charsSize * m_pTX->xoffs;
	double offsetY = charsSize * m_pTX->yoffs;
	//xoffs = m_pTX->xoffs * 0.351 * dpm_x;
	//yoffs = m_pTX->yoffs * 0.351 * dpm_x;
	textGroup = m_pTX->textGroup;

	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(m_pTX->color, r, g, b);

	// 计算标注点坐标
	QPoint p1, p2;
	calcLinePts(polylines, p1, p2);

	QPoint txPoint;
	txPoint.setX((p1.x() + p2.x()) / 2);
	txPoint.setY((p1.y() + p2.y()) / 2);

	painter->save();

	QFont font;
	font.setFamily("Times New Roman");
	font.setPixelSize(charsSize);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);
	QPen pen;
	pen.setColor(QColor(r, g, b));
	painter->setPen(pen);

	painter->translate(txPoint.x(), txPoint.y());
	painter->rotate(-angle);
	int textWidth = painter->fontMetrics().width(strAnno);
	int textHeight = painter->fontMetrics().height();
	switch (hAlign)
	{
	case 1://中心对齐
		xpos = (double)textWidth * -0.5;
		break;
	case 2://右对齐
		xpos = (double)textWidth * -1;
		break;
	case 3://左对齐
		xpos = 0;
		break;
	}

	switch (vAligh)
	{
	case 1://下对齐
		ypos = (double)textHeight * -1;
		break;
	case 2://中心对齐
		ypos = (double)textHeight * -0.5;
		break;
	case 3://上对齐
		ypos = 0;
		break;
	}

	QRectF textRect;
	textRect.setX(offsetX + xpos);
	textRect.setY(offsetY + ypos);
	textRect.setWidth((double)textWidth);
	textRect.setHeight((double)textHeight);
	painter->drawText(textRect, 0, strAnno);
	//painter->drawRect(textRect);

	painter->restore();
}

void CS57LineAnnotation::calcLinePts(CPolylines& polylines,QPoint & pt1, QPoint & pt2)
{
	int x, y, x1, y1;
	int distance = 0;
	for (int i = 0; i < polylines.size(); i++)
	{
		for (int j = 0; j < polylines[i].size() - 1; j++)
		{
			x = polylines[i][j].x();
			y = polylines[i][j].y();
			x1 = polylines[i][j + 1].x();
			y1 = polylines[i][j + 1].y();
			int dis = (x1 - x)*(x1 - x) + (y1 - y)*(y1 - y);
			if (distance < dis)
			{
				distance = dis;
				pt1 = polylines[i][j];
				pt2 = polylines[i][j + 1];
			}
			x = x1;
			y = y1;
		}
	}
}


CS57AreaAnnotation::CS57AreaAnnotation(CS57PresLib* preslib)
{
	m_pPresLib = preslib;
}

CS57AreaAnnotation::~CS57AreaAnnotation()
{}

void CS57AreaAnnotation::setTE(showTextTE* pTE)
{
	m_pTE = pTE;
}

void CS57AreaAnnotation::setTX(showTextTX* pTX)
{
	m_pTX = pTX;
}

void CS57AreaAnnotation::renderTE(QPainter * painter, CPolylines& polygons, unsigned short objl, QString strAnno, float angle, float scale)
{
	int hjust;				// HJUST "水平调整"参数:‘1’表示中心对齐‘2’表示右对齐‘3’表示左对齐（缺省值）
	int vjust;				// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐‘3’表示向上对齐
	int space;				// SPACE 字符间距参数: 1’表示撑满（FIT）间距‘2’表示标准间距，缺省值	‘3’表示带有字框的标准间距
	char charsStyle;		// 字体	"1" 简单，sans serif字体
	int charsWeight;		// 字粗	4 细体	5 中粗体，这是缺省值。	6 粗体
	char charsWidth;		// 字宽	1表示正直朝上（非斜体），ENC $CHARS属性用"2"时必须改为 "1"。
	float charsSize;		// 字号以pica points为单位给定(1点=0.351mm)，pica点规定了大写字母的高度，最小字号是pica10，并且这也默认大小，允许使用较大的字号。
	short xoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点X-偏移 (如果XOFFS未给定或定义则缺省为0);正向X偏移向右延伸(字号单位意味着，如果字号为10个pica点，则每单位的偏移是10(0.351)=3.51mm)
	short yoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点的Y-偏移 (如果YOFFS未给定或定义则缺省为0);正向Y偏移向下延伸。
	char color[6];			// 颜色标识
	unsigned short textGroup;		// 文本组合代码
	double	xpos = 0.0, ypos = 0.0;

	hjust = m_pTE->hjust;
	vjust = m_pTE->vjust;

	int hAlign = hjust - '0';
	int vAligh = vjust - '0';

	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(m_pTE->color, r, g, b);

	QString strFontFamily = "汉仪书宋二";
	float fontSize = 9;
	QPoint tePoint = getCentriodOfPolygon(polygons);

	bool bItalic = false;
	switch (objl)
	{
	case 119: // 海床区
		bItalic = true;
		strFontFamily = "汉仪中宋";
		fontSize = 12;
		if (strAnno.contains("海") || 
			strAnno.contains("洋") && 
			!strAnno.contains("港"))
			fontSize = 24;
		break;
	case 4:	// 锚地
		strFontFamily = "汉仪中等线";
		fontSize = 12;
	case 51:	// 航道
		strFontFamily = "汉仪书宋二";
		fontSize = 12;
		bItalic = true;
	default:
		break;
	}
	float dpm_x = painter->device()->physicalDpiX() / 25.4;
	space = m_pTE->space;
	charsStyle = m_pTE->charsStyle;
	charsWeight = m_pTE->charsWeight;
	charsSize = fontSize * 0.351 * dpm_x * scale;
	//xoffs = m_pTE->xoffs * 0.351 * dpm_x;
	//yoffs = m_pTE->yoffs * 0.351 * dpm_x;
	double offsetX = charsSize * m_pTE->xoffs;
	double offsetY = charsSize * m_pTE->yoffs;
	textGroup = m_pTE->textGroup;
	if (scale < 1)
	{
		if (charsSize < 5)
			return;
	}
	painter->save();

	painter->translate(tePoint.x(), tePoint.y());
	painter->rotate(-angle);

	QFont font;
	font.setFamily(strFontFamily);
	font.setPixelSize(charsSize);
	font.setItalic(bItalic);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);
	QPen pen;
	pen.setColor(QColor(r, g, b));
	painter->setPen(pen);

	int textWidth = painter->fontMetrics().width(strAnno)* 1.1;
	int textHeight = painter->fontMetrics().height();

	switch (hAlign)
	{
	case 1://中心对齐
		xpos = (double)textWidth * -0.5;
		break;
	case 2://右对齐
		xpos = (double)textWidth * -1;
		break;
	case 3://左对齐
		xpos = 0;
		break;
	}

	switch (vAligh)
	{
	case 1://下对齐
		ypos = (double)textHeight * -1;
		break;
	case 2://中心对齐
		ypos = (double)textHeight * -0.5;
		break;
	case 3://上对齐
		ypos = 0;
		break;
	}
	QRectF textRect;
	textRect.setX(offsetX + xpos);
	textRect.setY(offsetY + ypos);
	textRect.setWidth((double)textWidth);
	textRect.setHeight((double)textHeight);
	painter->drawText(textRect, 0, strAnno);
	//painter->drawRect(textRect);

	painter->restore();
}

void CS57AreaAnnotation::renderTX(QPainter * painter, CPolylines& polygons, unsigned short objl, QString strAnno, float angle, float scale)
{
	int hjust;				// HJUST "水平调整"参数:‘1’表示中心对齐‘2’表示右对齐‘3’表示左对齐（缺省值）
	int vjust;				// VJUST "垂直调整"参数:‘1’表示向下对齐（缺省值）‘2’表示中心对齐‘3’表示向上对齐
	int space;				// SPACE 字符间距参数: 1’表示撑满（FIT）间距‘2’表示标准间距，缺省值	‘3’表示带有字框的标准间距
	char charsStyle;		// 字体	"1" 简单，sans serif字体
	int charsWeight;		// 字粗	4 细体	5 中粗体，这是缺省值。	6 粗体
	char charsWidth;		// 字宽	1表示正直朝上（非斜体），ENC $CHARS属性用"2"时必须改为 "1"。
	float charsSize;		// 字号以pica points为单位给定(1点=0.351mm)，pica点规定了大写字母的高度，最小字号是pica10，并且这也默认大小，允许使用较大的字号。
	short xoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点X-偏移 (如果XOFFS未给定或定义则缺省为0);正向X偏移向右延伸(字号单位意味着，如果字号为10个pica点，则每单位的偏移是10(0.351)=3.51mm)
	short yoffs;			// 该参数定义了相对于空间物标位置以字号(单位见CHARS参数)给出的中枢点的Y-偏移 (如果YOFFS未给定或定义则缺省为0);正向Y偏移向下延伸。
	char color[6];			// 颜色标识
	unsigned short textGroup;		// 文本组合代码
	double	xpos = 0.0, ypos = 0.0;

	hjust = m_pTX->hjust;
	vjust = m_pTX->vjust;

	int hAlign = hjust - '0';
	int vAligh = vjust - '0';

	unsigned short r = 0, g = 0, b = 0;
	m_pPresLib->getS52Color(m_pTX->color, r, g, b);

	QString strFontFamily = "汉仪书宋二";
	float fontSize = 9;
	QPoint txPoint = getCentriodOfPolygon(polygons);

	bool bItalic = false;
	switch (objl)
	{
	case 119: // 海床区
	{
		bItalic = true;
		strFontFamily = "汉仪中宋";
		fontSize = 12;
		if (strAnno.contains("海")  ||
			strAnno.contains("洋")  &&
			!strAnno.contains("港") )
			fontSize = 24;
		break;
	}
	case 4:	// 锚地
		strFontFamily = "汉仪中等线";
		fontSize = 12;
	case 51:	// 航道
		strFontFamily = "汉仪书宋二";
		fontSize = 12;
		bItalic = true;
	default:
		break;
	}
	float dpm_x = painter->device()->physicalDpiX() / 25.4;
	space = m_pTX->space;
	charsStyle = m_pTX->charsStyle;
	charsWeight = m_pTX->charsWeight;
	charsSize = fontSize * 0.351 * dpm_x * scale;
	//xoffs = m_pTX->xoffs * 0.351 * dpm_x;
	//yoffs = m_pTX->yoffs * 0.351 * dpm_x;
	double offsetX = charsSize * m_pTX->xoffs;
	double offsetY = charsSize * m_pTX->yoffs;
	textGroup = m_pTX->textGroup;
	if (scale < 1)
	{
		if (charsSize < 5)
			return;
	}

	painter->save();

	painter->translate(txPoint.x(), txPoint.y());
	painter->rotate(-angle);

	QFont font;
	font.setFamily(strFontFamily);
	font.setPixelSize(charsSize);
	font.setItalic(bItalic);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);

	QPen pen;
	pen.setColor(QColor(r, g, b));
	painter->setPen(pen);

	int textWidth = painter->fontMetrics().width(strAnno) * 1.1;
	int textHeight = painter->fontMetrics().height();

	switch (hAlign)
	{
	case 1://中心对齐
		xpos = (double)textWidth * -0.5;
		break;
	case 2://右对齐
		xpos = (double)textWidth * -1;
		break;
	case 3://左对齐
		xpos = 0;
		break;
	}

	switch (vAligh)
	{
	case 1://下对齐
		ypos = (double)textHeight * -1;
		break;
	case 2://中心对齐
		ypos = (double)textHeight * -0.5;
		break;
	case 3://上对齐
		ypos = 0;
		break;
	}

	QRectF textRect;
	textRect.setX(offsetX + xpos);
	textRect.setY(offsetY + ypos);
	textRect.setWidth((double)textWidth);
	textRect.setHeight((double)textHeight);
	painter->drawText(textRect, 0, strAnno);
	//painter->drawRect(textRect);

	painter->restore();
}

QPoint CS57AreaAnnotation::getCentriodOfPolygon(CPolylines& polygons)
{
	float area = 0.0;//多边形面积  
	float Gx = 0.0, Gy = 0.0;// 重心的x、y  
	if (polygons.size() == 0)
		return QPoint((int)Gx, (int)Gy);

	for (int i = 1; i < polygons[0].size(); i++)
	{
		float iLat = polygons[0][(i % polygons[0].size())].x();
		float iLng = polygons[0][(i % polygons[0].size())].y();
		float nextLat = polygons[0][(i - 1)].x();
		float nextLng = polygons[0][(i - 1)].y();
		float temp = (iLat * nextLng - iLng * nextLat) / 2.0f;
		area += temp;
		Gx += temp * (iLat + nextLat) / 3.0f;
		Gy += temp * (iLng + nextLng) / 3.0f;
	}
	Gx = Gx / area;
	Gy = Gy / area;
	return  QPoint((int)Gx, (int)Gy);
}