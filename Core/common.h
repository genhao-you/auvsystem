#pragma once
#include <QString>

//¼òµ¥ÅĞ¶ÏÕûÊı
bool verifyNumber(QString str)
{
	std::string temp = str.toStdString();
	for (int i = 0; i < str.length(); i++)
	{
		if (temp[i] < '0' || temp[i] > '9')
		{
			return false;
		}
	}

	return true;
}
