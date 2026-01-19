#include "cs57loadcellthread.h"
#include "cs57cell.h"

using namespace Doc;
CS57LoadCellThread::CS57LoadCellThread()
	: QThread()
	, m_pMcovrProvider(nullptr)
{}

CS57LoadCellThread::~CS57LoadCellThread()
{
	exit();
}

//************************************
// Method:    exit
// Brief:	  退出线程
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57LoadCellThread::exit()
{
	requestInterruption();
	wait();
	quit();
}
//************************************
// Method:    setEncFiles
// Brief:	  设置ENC文件
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: const QVector<CS57WorkSpaceDoc::sEnErFiles> & encFiles
//************************************
void CS57LoadCellThread::setEncFiles(const QVector<CS57WorkSpaceDoc::sEnErFiles>& encFiles)
{
	m_vecEncFiles.clear();
	m_vecEncFiles = encFiles;
}

//************************************
// Method:    setMcovrProvider
// Brief:	  设置Mcovr数据源
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
// Parameter: CS57McovrProvider * pMcovrProvider
//************************************
void CS57LoadCellThread::setMcovrProvider(CS57McovrProvider* pMcovrProvider)
{
	m_pMcovrProvider = pMcovrProvider;
}

//************************************
// Method:    run
// Brief:	  执行线程
// Returns:   void
// Author:    cl
// DateTime:  2022/07/21
//************************************
void CS57LoadCellThread::run()
{
	while (!isInterruptionRequested())
	{
		for (int i = 0; i < m_vecEncFiles.size(); i++)
		{
			emit loadProgress(i + 1, m_vecEncFiles.size());

			CS57Cell oCell;
			oCell.importEncFile(m_vecEncFiles[i].strEnPath);
			oCell.initFeatureInst();
			oCell.writeSenc();
			//判断是否包含升级文件，进行ER自动更新操作
			for (int j = 0; j < m_vecEncFiles[i].lstErPaths.size(); j++)
			{
				//容器一开始就是按照001,002,003顺序，所有没有特意去排序
				oCell.autoErUpdate(m_vecEncFiles[i].lstErPaths[j]);
			}
			m_pMcovrProvider->addMsg(new CS57CellMessage(&oCell));
			msleep(5);
		}
		exit();
	}
}


