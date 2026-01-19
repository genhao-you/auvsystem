#include "stdafx.h"
#include "cs57settings.h"
#include <QSettings>
#include <QDomDocument>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QApplication>


CS57AppConfig* CS57AppConfig::m_pInstance = nullptr;
CS57AppConfig::CS57AppConfig()
	: m_strXmlFile(QApplication::applicationDirPath() + "/s57Config/AppConfig.xml")
	, m_dStartLon(107.206)
	, m_dStartLat(35.199)
	, m_dBaseLine(0.00)
	, m_dStartDisplayScale(30000000)
{
	m_pInstance = this;
	static Garbo mGarbo;
	xmlExists() ? readXml() : createXml();
}

CS57AppConfig::~CS57AppConfig()
{}

//************************************
// Method:    xmlExists
// Brief:	  xml文件是否存在
// Returns:   bool
// Author:    cl
// DateTime:  2021/08/02
//************************************
bool CS57AppConfig::xmlExists()
{
	QFile xml(m_strXmlFile);
	if (!xml.exists())
		return false;

	return true;
}

//************************************
// Method:    writeXml
// Brief:	  初始没有xml文件情况下创建xml
// Returns:   void
// Author:    cl
// DateTime:  2021/08/02
//************************************
void CS57AppConfig::createXml()
{
	QFile file(m_strXmlFile);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QDomDocument doc;
	QDomProcessingInstruction instruction;
	instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	QDomElement root = doc.createElement("App");
	doc.appendChild(root);

	QDomElement General = doc.createElement("General");
	doc.appendChild(General);

	//SymbolMode
	QDomElement SymbolMode = addElement(doc,"SymbolMode","PAPER_CHART");

	//Location
	QDomElement Location = addElement(doc, "Location", "");

	//BaseLine
	QDomElement BaseLine = addElement(doc, "BaseLine", "0.00");

	//EncDir
	QDomElement EncDir = addElement(doc, "EncDir", "");

	General.appendChild(SymbolMode);
	General.appendChild(Location);
	General.appendChild(BaseLine);
	General.appendChild(EncDir);

	root.appendChild(General);

	QTextStream out_stream(&file);
	doc.save(out_stream, 4);
	file.close();
}
//************************************
// Method:    addElement
// Brief:	  xml中添加节点
// Returns:   QT_NAMESPACE::QDomElement
// Author:    cl
// DateTime:  2021/08/02
// Parameter: QDomDocument & doc
// Parameter: const QString & elementName
// Parameter: const QString & value
//************************************
QDomElement CS57AppConfig::addElement(QDomDocument& doc, const QString& elementName, const QString& value)
{
	QDomElement element = doc.createElement(elementName);
	doc.appendChild(element);
	QDomText text = doc.createTextNode(value);
	element.appendChild(text);

	return element;
}

//************************************
// Method:    readXml
// Brief:	  读取xml
// Returns:   void
// Author:    cl
// DateTime:  2021/08/02
//************************************
void CS57AppConfig::readXml()
{
	QFile file(m_strXmlFile);
	if (!file.open(QFile::ReadOnly))
		return;

	QDomDocument doc;
	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomNode node = root.firstChild();
	while (!node.isNull())
	{
		if (node.isElement())
		{
			QDomElement element = node.toElement();
			QDomNodeList lstNode = element.childNodes();
			for (int i = 0; i < lstNode.size(); i++)
			{
				QDomNode n = lstNode.at(i);
				if (n.nodeName() == "SymbolMode")
				{
					QString strSymbolMode = n.toElement().text();

					if (strSymbolMode == "SIMPLIFIED_CHART")
						m_eSymbolMode = XT::SIMPLIFIED_CHART;
					else if (strSymbolMode == "PAPER_CHART")
						m_eSymbolMode = XT::PAPER_CHART;
					else if (strSymbolMode == "CDC_CHART")
						m_eSymbolMode = XT::CDC_CHART;
					else
						m_eSymbolMode = XT::PAPER_CHART;
				}
				else if (n.nodeName() == "Location")
				{
					QString strLocation = n.toElement().text();
					QStringList values = strLocation.split(",");
					if (values.size() < 3)
						return;

					m_dStartLon = values[0].toDouble();
					m_dStartLat = values[1].toDouble();
					m_dStartDisplayScale = values[2].toDouble();

					if (m_dStartLat == 0 && m_dStartLat == 0)
					{
						m_dStartLon = 107.206;
						m_dStartLat = 35.199;
						m_dStartDisplayScale = 30000000;
					}
				}
				else if (n.nodeName() == "BaseLine")
				{
					m_dBaseLine = n.toElement().text().toDouble();
				}
				else if (n.nodeName() == "EncDir")
				{
					m_strEncDir = n.toElement().text();
				}
			}
		}

		node = node.nextSibling();
	}
}

//************************************
// Method:    getEncDir
// Brief:	  获取ENC目录
// Returns:   QString
// Author:    cl
// DateTime:  2022/07/21
//************************************
QString CS57AppConfig::getEncDir() const
{	
	QString strEncDir;
	QDir dir(m_strEncDir);
	if (!dir.exists())
	{
		strEncDir = "";
		return strEncDir;
	}
	return m_strEncDir;
}

//************************************
// Method:    getSymbolMode
// Brief:	  获取符号模式
// Returns:   XT::SymbolMode
// Author:    cl
// DateTime:  2022/07/21
//************************************
XT::SymbolMode CS57AppConfig::getSymbolMode() const
{
	return m_eSymbolMode;
}

//************************************
// Method:    getStartLon
// Brief:	  获取起始点经度
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AppConfig::getStartLon() const
{
	return m_dStartLon;
}

//************************************
// Method:    getStartLat
// Brief:	  获取起始点纬度
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AppConfig::getStartLat() const
{
	return m_dStartLat;
}

//************************************
// Method:    setLocation
// Brief:	  设置视点坐标及比例尺
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double lon
// Parameter: double lat
// Parameter: double display
//************************************
void CS57AppConfig::setLocation(double lon, double lat, double display)
{
	if (lon == 0.0 && lat == 0.0)
	{
		m_dStartLon = 107.206;
		m_dStartLat = 35.199;
		m_dStartDisplayScale = 30000000;
	}
	else
	{
		m_dStartLon = lon;
		m_dStartLat = lat;
		m_dStartDisplayScale = display;
	}
}

//************************************
// Method:    setEncDir
// Brief:	  设置配置文件ENC目录
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QString & dir
//************************************
void CS57AppConfig::setEncDir(const QString& dir)
{
	m_strEncDir = dir;
}

//************************************
// Method:    setSymbolMode
// Brief:	  设置符号模式到配置文件中
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: XT::SymbolMode mode
//************************************
void CS57AppConfig::setSymbolMode(XT::SymbolMode mode)
{
	m_eSymbolMode = mode;
}

//************************************
// Method:    updateXml
// Brief:	  更新Xml配置文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57AppConfig::updateXml()
{
	QFile file(m_strXmlFile);
	if (!file.open(QFile::ReadOnly | QFile::Text))
		return;

	QDomDocument doc;
	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	file.close();

	QDomElement root = doc.documentElement();
	QDomNode generalNode = root.firstChild();
	QDomNode node = generalNode.firstChild();
	while (!node.isNull())
	{
		if (node.nodeName() == "SymbolMode")
		{
			QDomElement SymbolMode = doc.createElement("SymbolMode");
			QString strSymbolMode = "PAPER_CHART";
			switch (m_eSymbolMode)
			{
			case XT::SIMPLIFIED_CHART:
				strSymbolMode = "SIMPLIFIED_CHART";
				break;
			case XT::PAPER_CHART:
				strSymbolMode = "PAPER_CHART";
				break;
			case XT::CDC_CHART:
				strSymbolMode = "CDC_CHART";
				break;
			}
			QDomText symbolmode = doc.createTextNode(strSymbolMode);
			SymbolMode.appendChild(symbolmode);
			generalNode.replaceChild(SymbolMode, node);
			node = SymbolMode;
		}
		else if (node.nodeName() == "Location")
		{
			QDomElement Location = doc.createElement("Location");
			QString strLocation = QString::number(m_dStartLon) + "," +
								  QString::number(m_dStartLat) + "," +
								  QString::number(m_dStartDisplayScale, 'f', 6);
			QDomText position = doc.createTextNode(strLocation);
			Location.appendChild(position);
			generalNode.replaceChild(Location, node);
			node = Location;
		}
		else if (node.nodeName() == "BaseLine")
		{
			QDomElement BaseLine = doc.createElement("BaseLine");
			QString strBaseLine = QString::number(m_dBaseLine,'f',2);
			QDomText baseLine = doc.createTextNode(strBaseLine);
			BaseLine.appendChild(baseLine);
			generalNode.replaceChild(BaseLine, node);
			node = BaseLine;
		}
		else if (node.nodeName() == "EncDir")
		{
			QDomElement EncDir = doc.createElement("EncDir");
			QString strEncDir = m_strEncDir;
			QDomText encdir = doc.createTextNode(strEncDir);
			EncDir.appendChild(encdir);
			generalNode.replaceChild(EncDir, node);
			node = EncDir;
		}
		node = node.nextSibling();
	}

	QFile xml(m_strXmlFile);
	if (!xml.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return;

	QTextStream out_stream(&xml);
	doc.save(out_stream, 4);
	xml.close();
}

//************************************
// Method:    getStartDisplayScale
// Brief:	  获取起始显示比例尺
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AppConfig::getStartDisplayScale() const
{
	double scale = 0.0;
	if (m_dStartDisplayScale < 0)
	{
		scale = 100000000.0;
		return scale;
	}

	return m_dStartDisplayScale;
}

CS57AppConfig::Garbo::~Garbo()
{
	if (CS57AppConfig::m_pInstance)
	{
		delete CS57AppConfig::m_pInstance;
		CS57AppConfig::m_pInstance = nullptr;
	}
}

//************************************
// Method:    getBaseLine
// Brief:	  从配置文件获取基准纬线
// Returns:   double
// Author:    cl
// DateTime:  2022/07/21
//************************************
double CS57AppConfig::getBaseLine() const
{
	return m_dBaseLine;
}

//************************************
// Method:    setBaseLine
// Brief:	  设置基准纬线到配置文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: double baseLine
//************************************
void CS57AppConfig::setBaseLine(double baseLine)
{
	m_dBaseLine = baseLine;
}
