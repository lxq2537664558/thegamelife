#ifndef __SSLConnection_h__
#define __SSLConnection_h__

#include "Connection.h"
#include <boost/asio/ssl.hpp>

namespace Network
{
	class SSLConnection : public Connection
	{
	public:
		friend class IdealConnectionContainer;
		friend class TcpNetwork;
		typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> SSLSocket;

	public:
		virtual ~SSLConnection();
		virtual Socket& getSocket();
		virtual void connect(boost::asio::ip::tcp::endpoint& kEndpoint, 
			boost::system::error_code& kErrorCode);
		virtual void asyncConnect(boost::asio::ip::tcp::endpoint& kEndpoint,
			OnConnectFunc pfnOnConnect);

		virtual void handShake(boost::system::error_code& kErrorCode);
		virtual void asyncHandShake(OnHandShakeFunc pfnOnHandShakeFunc);

		/// ֻ�������̵߳��ã���ֻ����Ψһһ���̵߳���
		virtual bool open();
		/// ǿ�ƹرգ��п��ܵ��½��ջ���ͷ��ͻ����е�����δ������
		virtual void close();
		/// �첽�رգ����getErrorCodes����Ϊ����Ὣ���ݷ�����ɲ��ҽ��յ��������ݴ�����Ż�ر�
		virtual void asyncClose();
		/// SSLConnection���ӹرպ�������ʱ�ᷢ��handshakeʧ�ܣ�����д������ӿ���������
		virtual void reset();

		/// ��ȡԶ��IP��std::string��ʽ
		virtual std::string getRemoteIP();
		/// ��ȡԶ��IP, unsigned long��ʽ
		virtual unsigned long getRemoteAddress();

		/// ��ȡ����IP��std::string��ʽ
		virtual std::string getLocalIP();
		/// ��ȡ����IP, unsigned long��ʽ
		virtual unsigned long getLocalAddress();

		virtual bool isSSLConnection();

	protected:
		/// @param io_service boost::asio::io_service����
		/// @param pkParams ���Ӳ���
		SSLConnection(boost::asio::io_service& io_service,
			TcpNetworkParams* pkParams,
			boost::asio::ssl::context *pkSSLContext);

		/// ����һ��������������IO�߳���ͬʱֻ����һ��IO�̷߳���˺�������
		/// ���̼߳��mbSendIOReqRelayΪfalseʱ�ᷢ��˺�������
		/// @param bCallAtMainThread �Ƿ������̷߳���ĵ���
		virtual void send(bool bCallAtMainThread);
		/// ����һ��������������IO�߳���ͬʱֻ����һ��IO�̷߳���˺�������
		/// ���̼߳��mbRecvIOReqRelayΪfalseʱ�ᷢ��˺�������
		/// @param bCallAtMainThread �Ƿ������̷߳���ĵ���
		virtual void recv(bool bCallAtMainThread);
		/// ������ɺ�IO�̻߳�ص��������
		virtual void handleSent(const boost::system::error_code& kErrorCode,
			size_t uBytesTransferred);
		/// ������ɺ�IO�̻߳�ص��������
		virtual void handleRecvd(const boost::system::error_code& kErrorCode,
			size_t uBytesTransferred);
		/// asyncClose�Ļص�
		void handleClose(const boost::system::error_code& kErrorCode);

	private:
		virtual void asyncWriteSome(unsigned char *acBuff, unsigned int uiSize);
		virtual void asyncReadSome(unsigned char *acBuff, unsigned int uiSize);
		virtual void closeWrapper();
		bool hasKnownError();

	private:
		SSLSocket* mpkSSLSocket;
		boost::asio::ssl::context* mpkSSLContext;
		boost::asio::io_service& mkIOService;
		boost::asio::strand* mpkStrand;
		boost::mutex mkClosingMutex;              ///< �������ס�˱�ʾ�Ѿ����߳̽���رպ���
	};

	inline Connection::Socket& SSLConnection::getSocket()
	{
		BOOST_ASSERT(mpkSSLSocket);
		return mpkSSLSocket->next_layer();
	}
	inline bool SSLConnection::isSSLConnection()
	{
		return true;
	}
}
#endif//__SSLConnection_h__