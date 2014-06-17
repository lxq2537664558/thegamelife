#ifndef __DownloadTask_h__
#define __DownloadTask_h__
namespace Network
{

#define THREAD_COUNT	2		// ÿ�������߳���
#define BUFFER_SIZE	1024	// �������ߴ�

	class DownloadTask
	{
	public:
		// ��������״̬
		enum TaskStatus
		{
			TS_STOPED,			// ֹͣ
			TS_DOWNLOADING,	// ������
			TS_PAUSE,			// ��ͣ
			TS_COMPLETED,		// ���
		};

	public:
		virtual bool start() = 0;
		virtual bool pause() = 0;
		virtual bool resume() = 0;
		virtual bool stop() = 0;

	unsigned int getDownloadedBytes() { return muiDownloadedBytes; }
	unsigned int getTotalBytes() { return muiTotalBytes; }
	TaskStatus getStatus() { return meStatus; }

	protected:
		DownloadTask(const std::string& strURL, const std::string& strLocal);
		virtual ~DownloadTask();

		/*
		����URL��ַ�������������������ע�⣺mstrURL������������ʽ��
		ʾ����
		mstrURL��		"http://127.0.0.1:80/123/abc/abc.txt"
		mstrHost:		"127.0.0.1"
		muiPort:			80
		mstrFilePath:	"http://127.0.0.1:80/123/abc/"
		mstrFileName:	"abc.txt"
		*/
		bool parseURL();

		friend class DownloadMgr;

	protected:
		std::string mstrURL;					// ���ص�ַ
		unsigned int muiTotalBytes;				// �ܵĳߴ�
		TaskStatus meStatus;					// ����״̬
		unsigned int muiDownloadedBytes;		// �ۼ�������

		std::string mstrHost;					// ������ַ
		std::string mstrPort;					// �����˿�
		std::string mstrFilePath;				// �ļ���Զ��URLĿ¼
		std::string mstrFileName;				// �ļ���
		std::string mstrLocal;					// ���ر���λ��
	};
}
#endif//__DownloadTask_h__