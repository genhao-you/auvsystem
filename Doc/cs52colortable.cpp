#include "stdafx.h"
#include "cs52colortable.h"
#include <QFile>
#include <QMessageBox>
#include <QApplication>

using namespace Doc;
#pragma execution_character_set("utf-8")
CS52ColorTable::CS52ColorTable()
	: m_eColorMode(XT::DAY)
	, m_strColorTabPath(QApplication::applicationDirPath() + "/s57Data/s52color.dai")
{
	readAsciiFile(m_strColorTabPath);
}

CS52ColorTable::~CS52ColorTable()
{
	clear();
}

//************************************
// Method:    serachColor
// Brief:	  通过颜色模式和颜色名称获取颜色
// Returns:   Doc::S52Color*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: QString colorName
// Parameter: XT::S52ColorMode colorMode
//************************************
S52Color* CS52ColorTable::serachColor(QString colorName, XT::S52ColorMode colorMode)
{
	colorName.toUpper();
	if (colorName.size() > 5)
	{
		colorName = colorName.left(5);
	}
	QMap<QString, S52Color*>::iterator iter;
	switch (colorMode)
	{
	case XT::DAY:
		iter = m_mapDay.find(colorName);
		if (iter != m_mapDay.end())
			return iter.value();
		break;
	case XT::DAY_WHITEBACK:
		iter = m_mapDwb.find(colorName);
		if (iter != m_mapDwb.end())
			return iter.value();
		break;
	case XT::DAY_BLACKBACK:
		iter = m_mapDbb.find(colorName);
		if (iter != m_mapDbb.end())
			return iter.value();
		break;
	case XT::DUSK:
		iter = m_mapDusk.find(colorName);
		if (iter != m_mapDusk.end())
			return iter.value();
		break;
	case XT::NIGHT:
		iter = m_mapNight.find(colorName);
		if (iter != m_mapNight.end())
			return iter.value();
		break;
	}
	return nullptr;
}

//************************************
// Method:    serachColor
// Brief:	  通过颜色名称获取颜色
// Returns:   Doc::S52Color*
// Author:    cl
// DateTime:  2022/07/22
// Parameter: QString colorName
//************************************
S52Color* CS52ColorTable::serachColor(QString colorName)
{
	colorName.toUpper();
	if (colorName.size() > 5)
	{
		colorName = colorName.left(5);
	}
	QMap<QString, S52Color*>::iterator iter;
	switch (m_eColorMode)
	{
	case XT::DAY:
		iter = m_mapDay.find(colorName);
		if (iter != m_mapDay.end())
			return iter.value();
		break;
	case XT::DAY_WHITEBACK:
		iter = m_mapDwb.find(colorName);
		if (iter != m_mapDwb.end())
			return iter.value();
		break;
	case XT::DAY_BLACKBACK:
		iter = m_mapDbb.find(colorName);
		if (iter != m_mapDbb.end())
			return iter.value();
		break;
	case XT::DUSK:
		iter = m_mapDusk.find(colorName);
		if (iter != m_mapDusk.end())
			return iter.value();
		break;
	case XT::NIGHT:
		iter = m_mapNight.find(colorName);
		if (iter != m_mapNight.end())
			return iter.value();
		break;
	}
	return nullptr;
}

//************************************
// Method:    serachColor
// Brief:	  通过颜色名称获取颜色值
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: QString colorName
// Parameter: unsigned short & r
// Parameter: unsigned short & g
// Parameter: unsigned short & b
//************************************
void CS52ColorTable::serachColor(QString colorName, unsigned short& r, unsigned short& g, unsigned short& b)
{
	colorName.toUpper();
	if (colorName.size() > 5)
	{
		colorName = colorName.left(5);
	}
	QMap<QString, S52Color*>::iterator iter;
	switch (m_eColorMode)
	{
	case XT::DAY:
		iter = m_mapDay.find(colorName);
		if (iter != m_mapDay.end())
		{
			r = iter.value()->r;
			g = iter.value()->g;
			b = iter.value()->b;
		}
		break;
	case XT::DAY_WHITEBACK:
		iter = m_mapDwb.find(colorName);
		if (iter != m_mapDwb.end())
		{
			r = iter.value()->r;
			g = iter.value()->g;
			b = iter.value()->b;
		}
		break;
	case XT::DAY_BLACKBACK:
		iter = m_mapDbb.find(colorName);
		if (iter != m_mapDbb.end())
		{
			r = iter.value()->r;
			g = iter.value()->g;
			b = iter.value()->b;
		}
		break;
	case XT::DUSK:
		iter = m_mapDusk.find(colorName);
		if (iter != m_mapDusk.end())
		{
			r = iter.value()->r;
			g = iter.value()->g;
			b = iter.value()->b;
		}
		break;
	case XT::NIGHT:
		iter = m_mapNight.find(colorName);
		if (iter != m_mapNight.end())
		{
			r = iter.value()->r;
			g = iter.value()->g;
			b = iter.value()->b;
		}
		break;
	}
}

//************************************
// Method:    setColorMode
// Brief:	  设置颜色模式
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
// Parameter: XT::S52ColorMode colorMode
//************************************
void CS52ColorTable::setColorMode(XT::S52ColorMode colorMode)
{
	m_eColorMode = colorMode;
}

//************************************
// Method:    clear
// Brief:	  清理
// Returns:   void
// Author:    cl
// DateTime:  2022/07/22
//************************************
void CS52ColorTable::clear()
{
	QMap<QString, S52Color*>::iterator iter;
	for (iter = m_mapDay.begin(); iter != m_mapDay.end(); iter++)
	{
		S52Color* pColor = iter.value();
		if (pColor != nullptr)
			delete pColor;
		pColor = nullptr;
	}
	m_mapDay.clear();

	for (iter = m_mapDbb.begin(); iter != m_mapDbb.end(); iter++)
	{
		S52Color* pColor = iter.value();
		if (pColor != nullptr)
			delete pColor;
		pColor = nullptr;
	}
	m_mapDbb.clear();

	for (iter = m_mapDwb.begin(); iter != m_mapDwb.end(); iter++)
	{
		S52Color* pColor = iter.value();
		if (pColor != nullptr)
			delete pColor;
		pColor = nullptr;
	}
	m_mapDwb.clear();

	for (iter = m_mapDusk.begin(); iter != m_mapDusk.end(); iter++)
	{
		S52Color* pColor = iter.value();
		if (pColor != nullptr)
			delete pColor;
		pColor = nullptr;
	}
	m_mapDusk.clear();

	for (iter = m_mapNight.begin(); iter != m_mapNight.end(); iter++)
	{
		S52Color* pColor = iter.value();
		if (pColor != nullptr)
			delete pColor;
		pColor = nullptr;
	}
	m_mapNight.clear();
}

//************************************
// Method:    readAsciiFile
// Brief:	  读取Ascii文件
// Returns:   bool
// Author:    cl
// DateTime:  2022/07/22
// Parameter: const QString & fileName
//************************************
bool CS52ColorTable::readAsciiFile(const QString& fileName)
{
	char nDelimite = '\x1F';	// 分隔符
	QString info;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		info = fileName + "文件打开失败，退出！";
		//QMessageBox::information(nullptr, "提示", info);
		return false;
	}

	QString line;
	quint16 num = 0;
	QString nameFieldStr;
	QString contentLine;
	unsigned short len;
	XT::S52ColorMode curColorMode = XT::S52ColorMode::DAY;
	QStringList lstValues;
	while (!file.atEnd())
	{
		line = file.readLine();
		num++;
		if (line.size() <= 1)
		{
			info = fileName + "文件中第" + QString("%1").arg(num) + "行是空行，跳过！";
			//QMessageBox::information(nullptr, "提示", info);
			continue;
		}

		nameFieldStr = line.left(4);
		len = line.mid(4, 5).toUShort();
		contentLine = line.mid(9);

		if (nameFieldStr == "COLS")
		{
			QString str = contentLine.split(nDelimite)[0].mid(10);
			if (str == "DAY")
			{
				curColorMode = XT::DAY;
				continue;
			}
			if (str == "DAY_BLACKBACK")
			{
				curColorMode = XT::DAY_BLACKBACK;
				continue;
			}
			if (str == "DAY_WHITEBACK")
			{
				curColorMode = XT::DAY_WHITEBACK;
				continue;
			}
			if (str == "DUSK")
			{
				curColorMode = XT::DUSK;
				continue;
			}
			if (str == "NIGHT")
			{
				curColorMode = XT::NIGHT;
				continue;
			}
		}

		if (nameFieldStr != "CCIE")
			continue;

		lstValues = contentLine.split(nDelimite);
		if (lstValues.size() == 0 ||
			lstValues.size() != 4)
		{
			info = fileName + "文件中第" + QString("%1").arg(num) + "行格式错，跳过！";
			//QMessageBox::information(nullptr, "提示", info);
			continue;
		}

		S52Color* pColor = new S52Color();
		pColor->colorName = lstValues[0].left(5).toUpper();
		pColor->r = lstValues[0].mid(5).toUShort();
		pColor->g = lstValues[1].toUShort();
		pColor->b = lstValues[2].toUShort();
		switch (curColorMode)
		{
		case XT::DAY:
			m_mapDay.insert(pColor->colorName, pColor);
			break;
		case XT::DAY_WHITEBACK:
			m_mapDwb.insert(pColor->colorName, pColor);
			break;
		case XT::DAY_BLACKBACK:
			m_mapDbb.insert(pColor->colorName, pColor);
			break;
		case XT::DUSK:
			m_mapDusk.insert(pColor->colorName, pColor);
			break;
		case XT::NIGHT:
			m_mapNight.insert(pColor->colorName, pColor);
			break;
		}
	}
	file.close();
	return true;
}
