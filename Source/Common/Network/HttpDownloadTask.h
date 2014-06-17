#ifndef __HttpDownloadTask_h__
#define __HttpDownloadTask_h__
#include "DownloadTask.h"
namespace Network
{
	// HTTP���طֶ�
	struct HttpSec
	{
		HttpSec():task(0),nStart(0),nEnd(0)
		{
		}
		~HttpSec()
		{
		}
		DownloadTask* task;			// ����ָ��
		std::string strHost;			// ������ַ
		std::string strPort;			// �����˿�
		std::string strFilePath;	// �ļ���Զ��URLĿ¼
		std::string strFileName;	// �ļ���
		std::string strLocal;		// ���ر���λ��
		unsigned int nStart;					// �ֶε���ʼλ��
		unsigned int nEnd;						// �ֶε���ʼλ��
	};

	// HTTP��������
	class HttpDownloadTask : public DownloadTask
	{
	public:
		virtual bool start();
		virtual bool pause();
		virtual bool resume();
		virtual bool stop();

	protected:
		unsigned int getContentLength();

	private:
		HttpDownloadTask(const std::string& strURL, const std::string& strLocal);
		virtual ~HttpDownloadTask();

		friend class DownloadMgr;
	};
}
#endif//__HttpDownloadTask_h__