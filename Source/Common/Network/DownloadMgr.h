#ifndef __DownloadMgr_h__
#define __DownloadMgr_h__

#include <list>
namespace Network
{
	class DownloadTask;

	class DownloadMgr
	{
	public:
		DownloadMgr();
		~DownloadMgr();

		bool init();
		bool shut();

	DownloadTask* addTask(std::string strURL, const std::string& strLocal, unsigned int size = 0); // size��ʾԤ��ĳߴ�
	bool removeTask(DownloadTask* pkTask);
	void pauseAllTasks();
	void resumeAllTasks();

		void stat(); // ͳ������������ݣ�����ÿ��ͳ��һ��

		unsigned int getSpeed() { return muiDownloadSpeed; }
		unsigned int getDownloadedBytes() { return muiDownloadedBytes; }

	private:
		std::list<DownloadTask*> mDownloadTasksList;	// ���������б���ɵ�����Ӧ��ʱ���

		unsigned int muiLastStatTick;				// �ϴ�ͳ�Ƶ�ʱ��
		unsigned int muiDownloadSpeed;			// ƽ�������ٶ�
		unsigned int muiDownloadedBytes;			// �ۼ���������
		unsigned int muiLastDownloadedBytes;	// �ϴ�ͳ��ʱ�̵��ۼ���������
		unsigned int muiFinishedBytes;			// ����ɵ�����ĳߴ�
	};
}
#endif//__DownloadMgr_h__
