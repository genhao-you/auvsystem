#pragma once
#include "xt_global.h"
#include "control_global.h"
#include <QString>
#include <QVariant>

class QDomDocument;
class QDomElement;
class CONTROL_EXPORT CS57AppConfig
{
public:
	~CS57AppConfig();

	static CS57AppConfig* instance()
	{
		if (m_pInstance == nullptr)
		{
			m_pInstance = new CS57AppConfig();
		}
		return m_pInstance;
	}

public:
	//设置视点
	void setLocation(double lon, double lat, double display);
	//设置ENC目录
	void setEncDir(const QString& dir);
	//设置基准纬线
	void setBaseLine(double baseLine);
	//设置符号模式
	void setSymbolMode(XT::SymbolMode mode);

	//xml文件是否存在
	bool xmlExists();
	//创建xml配置文件
	void createXml();
	//读取xml配置文件
	void readXml();
	//更新xml配置文件
	void updateXml();

	//获取ENC目录
	QString getEncDir() const;
	//获取符号模式
	XT::SymbolMode getSymbolMode() const;
	//获取起始点经度
	double	getStartLon() const;
	//获取起始点纬度
	double	getStartLat() const;
	//获取基准纬线
	double  getBaseLine() const;
	//获取起始显示比例尺
	double	getStartDisplayScale() const;

private:
	CS57AppConfig();

	//专门用于析构单例类
	class Garbo
	{
	public:
		~Garbo();
	};

	QDomElement addElement(QDomDocument& doc,const QString& elementName,const QString& value);

private:
	QString m_strXmlFile;
	QString m_strEncDir;
	double m_dStartLon;
	double m_dStartLat;
	double m_dStartDisplayScale;
	double m_dBaseLine;
	XT::SymbolMode m_eSymbolMode;
	static CS57AppConfig* m_pInstance;
};
