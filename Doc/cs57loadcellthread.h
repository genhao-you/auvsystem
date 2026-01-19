#pragma once

#include "cs57docmanager.h"
#include <QThread>
namespace Doc
{
	class CS57LoadCellThread : public QThread
	{
		Q_OBJECT

	public:
		CS57LoadCellThread();
		~CS57LoadCellThread();

		//退出线程
		void exit();
		//设置ENC文件
		void setEncFiles(const QVector<CS57WorkSpaceDoc::sEnErFiles>& encFiles);
		//设置Mcovr数据
		void setMcovrProvider(CS57McovrProvider* pMcovrProvider);
	protected:
		//运行线程
		void run();

	signals:
		void loadProgress(int cur, int total);

	private:
		QVector<CS57WorkSpaceDoc::sEnErFiles> m_vecEncFiles;
		CS57McovrProvider*		m_pMcovrProvider;
	};
}
