#include "NetworkPCH.h"
#include "HttpDownloadTask.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <iostream>


namespace Network
{
	void ThreadHTTPDownloadFunc(HttpSec* task)
	{
		try
		{
			if (!task)
				return;

			// 0.��δ��������ļ�
			//////////////////////////////////////////////////////////////////////////
			FILE* f = NULL;
			int startpos = 0; // �����صĳߴ�
			if (boost::filesystem::exists(task->strLocal))
			{
				startpos = (int)boost::filesystem::file_size(task->strLocal);
				f = fopen(task->strLocal.c_str(), "r+b");
				if (!f)
					return;
				fseek(f, startpos, SEEK_SET);
			}
			else
			{
				f = fopen(task->strLocal.c_str(), "wb");
				if (!f)
					return;
			}

			// 1.����
			//////////////////////////////////////////////////////////////////////////
			// ȡ�������������صĶ˵��б�
			boost::asio::io_service io_service;
			boost::asio::ip::tcp::resolver resolver(io_service);
			boost::asio::ip::tcp::resolver::query query(task->strHost.c_str(), task->strPort.c_str());
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

			// ��������ÿ���������˵㣬ֱ���ɹ���������
			boost::asio::ip::tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator);

			// 2.��������
			//////////////////////////////////////////////////////////////////////////
			// ��д����������
			boost::asio::streambuf request;
			std::ostream request_stream(&request);
			request_stream << "GET " << task->strFilePath << task->strFileName << " HTTP/1.1\r\n";
			request_stream << "Host: " << task->strHost << "\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Range: bytes=" << task->nStart+startpos << "-" << task->nEnd << "\r\n";
			request_stream << "Connection: close\r\n\r\n";
			boost::asio::write(socket, request);

			// 3.��ȡ�ļ��ߴ�
			//////////////////////////////////////////////////////////////////////////
			// ��ȡ��Ӧͷ
			boost::array<char, BUFFER_SIZE> buf; // ÿ��������1024�ֽڣ�1K��
			boost::system::error_code ec;
			std::size_t nLen = socket.read_some(boost::asio::buffer(buf), ec);

			if (strstr(buf.data(), "416 Requested Range Not Satisfiable")) // ��Χ���ԣ���������������
			{
				fclose(f);
				return;
			}

			// Ѱ��"Content-Length:"�����ĺ�����ž����ļ��ߴ���Ϣ��
			char* sz_content_len = strstr(buf.data(), "Content-Length:");
			if (!sz_content_len)
			{
				fclose(f);
				return;
			}
			sz_content_len += strlen("Content-Length:");
			int content_len = atoi(sz_content_len);

			// 4.���ؾ�����ļ�����
			//////////////////////////////////////////////////////////////////////////
			// ��ȡ�����գ���д��ʵ�����ݵ��ļ�
			// ��д���һ�ν��յ���������
			int recved = 0;
			char* datastart = strstr(buf.data(), "\r\n\r\n");
			if (!datastart)
			{
				fclose(f);
				return;
			}
			datastart += 4;
			int firstlen = nLen-(datastart-buf.data());
			fwrite(datastart, 1, firstlen, f);
			recved += firstlen;
			// �ٽ���д����������
			while(recved < content_len)
			{
				if (task->task->getStatus() == DownloadTask::TS_PAUSE)
				{
					boost::this_thread::sleep(boost::posix_time::milliseconds(10)); // ������ͣʱCPUռ�ù���
					continue;
				}
				else if (task->task->getStatus() == DownloadTask::TS_STOPED)
					break;

				nLen = socket.read_some(boost::asio::buffer(buf), ec);
				if (nLen == 0)
					break;
				fwrite(buf.data(), 1, nLen, f);
				recved += nLen;
			}
			fclose(f);
		}
		catch (std::exception& e)
		{
			//std::cout << "Exception: " << e.what() << "\n";
			throw e;
		}
	}

	HttpDownloadTask::HttpDownloadTask(const std::string& strURL, const std::string& strLocal)
	: DownloadTask(strURL, strLocal)
	{
	}

	HttpDownloadTask::~HttpDownloadTask()
	{
	}

	bool HttpDownloadTask::start()
	{
		// ���״̬
		if (meStatus != TS_STOPED)
			return false;

		//// ����Ƿ��Ѿ����ع���
		//if (boost::filesystem::exists(mstrLocal))
		//{
		//	meStatus = DownloadTask::TS_COMPLETED;
		//	return true;
		//}

		// ȷ������Ŀ¼������
		int pos = mstrLocal.find_last_of("/");
		if (pos == -1)
			return false;
		std::string strLocalPath = mstrLocal.substr(0, pos+1);
		if (!boost::filesystem::exists(strLocalPath))
			boost::filesystem::create_directories(strLocalPath);

		// �������ص�ַ
		if (!parseURL())
			return false;

		// �ж�Ԥ���Զ���ļ��ߴ�
		if (muiTotalBytes == 0)
		{
			muiTotalBytes = getContentLength(); // δԤ�⣬���ֶ�ȥ��ȡ
			if (muiTotalBytes == 0)
				return false;
		}

		// ��������ֶγߴ�
		unsigned int secSize = muiTotalBytes / THREAD_COUNT;

		// ����״̬
		meStatus = TS_DOWNLOADING;

		// ���ֶ���Ϣ
		HttpSec secs[THREAD_COUNT];
		for (int i = 0; i < THREAD_COUNT; i++)
		{
			secs[i].task = this;
			secs[i].strHost = mstrHost;
			secs[i].strPort = mstrPort;
			secs[i].strFilePath = mstrFilePath;
			secs[i].strFileName = mstrFileName;
			char num[4] = {};
			sprintf(num, "%d", i);
			secs[i].strLocal = mstrLocal + num;
			secs[i].nStart = i*secSize;
			if (i == THREAD_COUNT-1)
				secs[i].nEnd = muiTotalBytes;
			else
				secs[i].nEnd = (i+1)*secSize - 1;
		}

#ifdef WIN32
		DWORD starttick = GetTickCount();
#else
		unsigned int starttick = clock();
#endif // WIN32
		//std::cout << "Start download...\n";

		// ���������ֶε������߳�
		boost::thread_group kThreadGroup;
		for (int i = 0; i < THREAD_COUNT; i++)
			kThreadGroup.create_thread(boost::bind(&ThreadHTTPDownloadFunc, &secs[i]));

		// �ȴ������ֶ��������
		kThreadGroup.join_all();
		//std::cout << "Download completed. Cost tick: " << GetTickCount()-starttick << "\n";

		if (THREAD_COUNT > 1)
		{
			// �ϲ��ֶ�Ϊ��һ�ļ�
			boost::filesystem::ofstream to_file(mstrLocal, std::ios_base::out | std::ios_base::binary);
			if (!to_file)
				return false;
			char buf[BUFFER_SIZE] = {};
			for (int i = 0; i < THREAD_COUNT; i++)
			{
				boost::filesystem::ifstream from_file(secs[i].strLocal, std::ios_base::in | std::ios_base::binary);
				if (!from_file)
					return false;

				while (1)
				{
					int s = (int)from_file.read(buf, 1024).gcount();
					if (s <= 0)
						break;
					to_file.write(buf, s);
					if (to_file.fail())
						return false;
				}

				if (!from_file.eof())
					return false;
			}

			// ɾ���ֶ��ļ�
			for (int i = 0; i < THREAD_COUNT; i++)
				boost::filesystem::remove(secs[i].strLocal);
		}
		else // ��Ե��̵߳��Ż�
		{
			boost::filesystem::rename(secs[0].strLocal.c_str(), mstrLocal.c_str());
		}

		//// ��ѹ�ļ�
		//unsigned int uiPos = mstrLocal.find_last_of('.');
		//std::string destFileName = mstrLocal.substr(0, uiPos);
		//if (dec.DecodeFile(mstrLocal.c_str(), destFileName.c_str()) != SZ_OK)
		//	return false;

		//// ɾ��ѹ���ļ�
		//boost::filesystem::remove(mstrLocal);

		meStatus = DownloadTask::TS_COMPLETED;

		//std::cout << "Combine completed. Total cost tick: " << GetTickCount()-starttick << "\n";

		return true;
	}

	bool HttpDownloadTask::pause()
	{
		meStatus = TS_PAUSE;
		return true;
	}

	bool HttpDownloadTask::resume()
	{
		meStatus = TS_DOWNLOADING;
		return true;
	}

	bool HttpDownloadTask::stop()
	{
		meStatus = TS_STOPED;
		return true;
	}

	unsigned int HttpDownloadTask::getContentLength()
	{
		try
		{
			// 1.����
			//////////////////////////////////////////////////////////////////////////
			// ȡ�������������صĶ˵��б�
			boost::asio::io_service io_service;
			boost::asio::ip::tcp::resolver resolver(io_service);
			boost::asio::ip::tcp::resolver::query query(mstrHost.c_str(), mstrPort.c_str());
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

			// ��������ÿ���������˵㣬ֱ���ɹ���������
			boost::asio::ip::tcp::socket socket(io_service);
			boost::asio::connect(socket, endpoint_iterator);

			// 2.��������
			//////////////////////////////////////////////////////////////////////////
			// ��д����������
			boost::asio::streambuf request;
			std::ostream request_stream(&request);
			request_stream << "GET " << mstrFilePath << mstrFileName << " HTTP/1.1\r\n";
			request_stream << "Host: " << mstrHost << "\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Range: bytes=0-\r\n";
			request_stream << "Connection: close\r\n\r\n";
			boost::asio::write(socket, request);

			// 3.��ȡ�ļ��ߴ�
			//////////////////////////////////////////////////////////////////////////
			// ��ȡ��Ӧͷ
			boost::array<char, BUFFER_SIZE> buf;
			boost::system::error_code ec;
			socket.read_some(boost::asio::buffer(buf), ec);
			// URL�ļ��Ƿ����
			if (strstr(buf.data(), "404 Not Found"))
				return 0;

			// Ѱ��"Content-Length:"�����ĺ�����ž����ļ��ߴ���Ϣ��
			char* sz_content_len = strstr(buf.data(), "Content-Length:");
			if (!sz_content_len)
				return 0;
			sz_content_len += strlen("Content-Length:");
			return atoi(sz_content_len);
		}
		catch (std::exception& e)
		{
			//std::cout << "Exception: " << e.what() << "\n";
			throw e;
		}
		return 0;
	}
}