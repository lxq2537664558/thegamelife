#include "NetworkPCH.h"
#include "DownloadMgr.h"
#include "HttpDownloadTask.h"
namespace Network
{
	DownloadMgr::DownloadMgr()
		:muiLastStatTick(0),muiDownloadSpeed(0),muiDownloadedBytes(0),muiLastDownloadedBytes(0),muiFinishedBytes(0)
	{
	}

	DownloadMgr::~DownloadMgr()
	{
		shut();
	}

	bool DownloadMgr::init()
	{
		stat(); // ���е�һ��ͳ�ƣ��൱�ڳ�ʼ��
		return true;
	}

	bool DownloadMgr::shut()
	{
		for (std::list<DownloadTask*>::iterator itr = mDownloadTasksList.begin(); itr != mDownloadTasksList.end(); ++itr)
		{
			DownloadTask* task = *itr;
			task->stop();
			delete task;
			task = 0;
		}
		mDownloadTasksList.clear();

		return true;
	}

	DownloadTask* DownloadMgr::addTask(std::string strURL, const std::string& strLocal, unsigned int size)
	{
		// ����URL��Э�����ͣ����ö�Ӧ��Э������
		int pos = (int)strURL.find_first_of(":");
		if (pos == -1)
			return 0;
		std::string strProtocol = strURL.substr(0, pos);
		if (strProtocol.compare("http") == 0)
		{
			HttpDownloadTask* task = new HttpDownloadTask(strURL, strLocal);
			if (!task)
				return 0;
			task->muiTotalBytes = size;
			mDownloadTasksList.push_back(task);
			task->start();
			return task;
		}
		else if (strProtocol.compare("ftp") == 0)
		{
		}
		else if (strProtocol.compare("https") == 0)
		{
		}

			return 0;
	}

	bool DownloadMgr::removeTask(DownloadTask* pkTask)
	{
		if (!pkTask)
			return false;

		std::list<DownloadTask*>::iterator itr = mDownloadTasksList.begin();
		for (; itr != mDownloadTasksList.end(); ++itr)
		{
			if (pkTask == *itr)
			{
				pkTask->stop();
				delete pkTask;
				pkTask = 0;
				break;
			}
		}
		mDownloadTasksList.erase(itr);

		return true;
	}

	void DownloadMgr::pauseAllTasks()
	{
		for (std::list<DownloadTask*>::iterator itr = mDownloadTasksList.begin(); itr != mDownloadTasksList.end(); ++itr)
		{
			DownloadTask* task = *itr;
			task->pause();
		}
	}

	void DownloadMgr::resumeAllTasks()
	{
		for (std::list<DownloadTask*>::iterator itr = mDownloadTasksList.begin(); itr != mDownloadTasksList.end(); ++itr)
		{
			DownloadTask* task = *itr;
			task->resume();
		}
	}

	void DownloadMgr::stat()
	{
		// �������ɵ���������
		std::list<DownloadTask*>::iterator itr = mDownloadTasksList.begin();
		while (itr != mDownloadTasksList.end())
		{
			DownloadTask* task = *itr;
			if (task->meStatus != DownloadTask::TS_COMPLETED)
			{
				++itr;
				continue;
			}

			// ��������ɵĳߴ�
			muiFinishedBytes += task->getTotalBytes();

			std::list<DownloadTask*>::iterator itrtmp = itr;
			itr++;
			delete *itrtmp;
			*itrtmp = 0;
			mDownloadTasksList.erase(itrtmp);
		}

		// ͳ�������ٶȣ�ָ�����������������ƽ����
		// ������������
		muiDownloadedBytes = muiFinishedBytes;
		for (std::list<DownloadTask*>::iterator itr = mDownloadTasksList.begin(); itr != mDownloadTasksList.end(); ++itr)
		{
			DownloadTask* task = *itr;
			muiDownloadedBytes += task->getDownloadedBytes();
		}
		// ���������ٶ�
#ifdef WIN32
		unsigned int nowatick = GetTickCount();
#else
		unsigned int nowatick = clock();
#endif//WIN32
		unsigned int elapsedtick = nowatick-muiLastStatTick;
		if (elapsedtick == 0)
			elapsedtick = 1;
		muiDownloadSpeed = (muiDownloadedBytes-muiLastDownloadedBytes) / elapsedtick;
		// ���±�������
		muiLastDownloadedBytes = muiDownloadedBytes;
		muiLastStatTick = nowatick;
	}
}
