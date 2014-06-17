#ifndef __TcpConnection_h__
#define __TcpConnection_h__

#include "Connection.h"

namespace Network
{
	class TcpConnection : public Connection
	{
	public:
		friend class IdealConnectionContainer;
		friend class TcpNetwork;
		typedef boost::asio::ip::tcp::socket TCPSocket;

	public:
		virtual ~TcpConnection();
		virtual Socket& getSocket();
		virtual void connect(boost::asio::ip::tcp::endpoint& kEndpoint, 
			boost::system::error_code& kErrorCode);
		virtual void asyncConnect(boost::asio::ip::tcp::endpoint& kEndpoint,
			OnConnectFunc pfnOnConnect);

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
		
	protected:
		/// @param io_service boost::asio::io_service����
		/// @param pkParams ���Ӳ���
		TcpConnection(boost::asio::io_service& io_service,
			TcpNetworkParams* pkParams);
		
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
		virtual void handleClose(const boost::system::error_code& kErrorCode);
		
	private:
		TCPSocket mkSocket;
	};
	inline void TcpConnection::reset()
	{
		if (mpkExtraSendBuffPool)
		{
			mpkExtraSendBuffPool->resetPool();
		}
	}
	inline Connection::Socket& TcpConnection::getSocket()
	{
		return mkSocket;
	}
}
#endif//__TcpConnection_h__