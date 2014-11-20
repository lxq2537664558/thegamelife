#include "NetworkPCH.h"
#include "TcpConnection.h"

namespace Network
{
	TcpConnection::TcpConnection(boost::asio::io_service& io_service,
		TcpNetworkParams* pkParams)
		: Connection(io_service, pkParams),
		  mkSocket(io_service)
	{

	}

	TcpConnection::~TcpConnection()
	{
	}
	void TcpConnection::connect(boost::asio::ip::tcp::endpoint& kEndpoint, 
		boost::system::error_code& kErrorCode)
	{
		setAsClientSide();
		mkSocket.connect(kEndpoint, kErrorCode);
	}
	void TcpConnection::asyncConnect(boost::asio::ip::tcp::endpoint& kEndpoint,
		OnConnectFunc pfnOnConnect)
	{
		setAsClientSide();
		mkSocket.async_connect(kEndpoint, pfnOnConnect);
	}
	bool TcpConnection::open()
	{
		clearErrorCodes();
		if (!mkSocket.is_open() || !isClosed())
		{
			boost::system::error_code kErrorCode = Network::accepted_but_failed_when_open;
			pushErrorCode(kErrorCode);
			return false;
		}
		if (mpkExtraSendBuffPool)
		{
			mpkExtraSendBuffPool->resetPool();
		}
		mbClosed = false;
		mbClosing = false;
		mbShouldBeClosed = false;
		mbNeedCallCloseCallback = true;
		mpRecvBuffer->clear();
		mpSendBuffer->clear();
		if (!setKeepAlive())
		{
			return false;
		}
		recv(true);
		return true;
	}
	void TcpConnection::close()
	{
		if (mbClosed)
		{
			return;
		}
		markShouldBeClosed();
		mbClosing = true;
		boost::system::error_code kErrorCode;
		if (mkSocket.is_open())
		{	
			mkSocket.cancel(kErrorCode);
			kErrorCode ? pushErrorCode(kErrorCode) : kErrorCode.clear();
			mkSocket.shutdown(boost::asio::socket_base::shutdown_both, kErrorCode);
			kErrorCode ? pushErrorCode(kErrorCode) : kErrorCode.clear();
			mkSocket.close(kErrorCode);
			kErrorCode ? pushErrorCode(kErrorCode) : kErrorCode.clear();
		}
#ifdef _DEBUG
		if (kErrorCode)
		{
			unsigned int uiErrorCode = kErrorCode.value();
			std::string strErrorMsg = kErrorCode.message();
			BOOST_ASSERT(0);
		}
#endif // _DEBUG
		handleClose(kErrorCode);
	}
	void TcpConnection::asyncClose()
	{
		markShouldBeClosed();
	}
	void TcpConnection::handleClose(const boost::system::error_code& kErrorCode)
	{
		mbClosed = true;
	}
	void TcpConnection::send(bool bCallAtMainThread)
	{
		/// ֻ�������̲߳���Ҫ����Ƿ��з������ڽ��У�
		/// mbSendIOReqRelayֻ����@see TcpConnection::handleSent������û�з����µķ���
		/// �����Լ�@see TcpConnection::send����ִ��ʱû�����ݿ��Է��ͣ�����
		/// ����������²ŻὫmbSendIOReqRelay��Ϊfalse������ֻ�����̵߳���@see TcpConnection::send
		/// ʱ��Ҫ���mbSendIOReqRelay�Ƿ�Ϊtrue������ֱ�ӷ���
		if (isClosed() || (bCallAtMainThread && mbSendIOReqRelay))
		{
			return;
		}
		unsigned char *acBuff = 0;
		unsigned int uiReadableSize = 0;
		if (mpSendBuffer->readBegin(acBuff, uiReadableSize))
		{
			if (uiReadableSize > 0)
			{
				mbSendIOReqRelay = true;
				mkSocket.async_write_some(boost::asio::buffer(acBuff, uiReadableSize),
					boost::bind(&Connection::handleSent, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			}
			else
			{
				mpSendBuffer->readEnd(0);
				mbSendIOReqRelay = false;
			}
		}
	}
	void TcpConnection::recv(bool bCallAtMainThread)
	{
		/// ֻ�������߲���Ҫ����Ƿ��н������ڽ��У�
		/// mbRecvIOReqRelayֻ����@see TcpConnection::handleRecvd������û�з����µĽ���
		/// �����Լ�@see TcpConnection::recv����ִ��ʱû�з��������������
		/// ����������²ŻὫmbRecvIOReqRelay��Ϊfalse������ֻ�����̵߳���@see TcpConnection::recv
		/// ʱ��Ҫ���mbRecvIOReqRelay�Ƿ�Ϊtrue������ֱ�ӷ���
		if (bCallAtMainThread && mbRecvIOReqRelay)
		{
			return;
		}
		if (isClosed() || shouldBeClosed())
		{
			mbRecvIOReqRelay = false;
			return;
		}
		unsigned char *acBuff = 0;
		unsigned int uiWritableSize = 0;
		if (mpRecvBuffer->writeBegin(acBuff, uiWritableSize))
		{
			if (uiWritableSize > 0)
			{
				mbRecvIOReqRelay = true;
				mkSocket.async_read_some(boost::asio::buffer(acBuff, uiWritableSize),
					boost::bind(&Connection::handleRecvd, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
				return;
			}
			else
			{
				mpRecvBuffer->writeEnd(0);
				if (mpkTcpNetworkParams->bCloseOnBufferOverflow)
				{
					/// ���ջ����������ֱ�ӶϿ�
					asyncClose();
					boost::system::error_code kErrorCode = Network::recv_buff_overflow;
					pushErrorCode(kErrorCode);
				}
			}
		}
		mbRecvIOReqRelay = false;
	}
	void TcpConnection::handleSent(const boost::system::error_code& kErrorCode,
								size_t uBytesTransferred)
	{
	#ifdef _DEBUG
		BOOST_INTERLOCKED_EXCHANGE_ADD(&msSendCnt, uBytesTransferred);
	#endif // _DEBUG

		mpSendBuffer->readEnd(uBytesTransferred);
		if (kErrorCode)
		{
			pushErrorCode(kErrorCode);
			asyncClose();
			mbSendIOReqRelay = false;
			return;
		}
		if (uBytesTransferred == 0)
		{
			asyncClose();
			mbSendIOReqRelay = false;
			return;
		}
		if (isClosed() || mpSendBuffer->readableSize() == 0)
		{
			mbSendIOReqRelay = false;
			return;
		}
		send(false);
	}
	void TcpConnection::handleRecvd(const boost::system::error_code& kErrorCode,
								 size_t uBytesTransferred)
	{
	#ifdef _DEBUG
		BOOST_INTERLOCKED_EXCHANGE_ADD(&msRecvCnt, uBytesTransferred);
	#endif // _DEBUG

		mpRecvBuffer->writeEnd(uBytesTransferred);
		if (kErrorCode)
		{
			pushErrorCode(kErrorCode);
			asyncClose();
			mbRecvIOReqRelay = false;
			return;
		}
		if (uBytesTransferred == 0)
		{
			asyncClose();
			mbRecvIOReqRelay = false;
			return;
		}
		if (isClosed())
		{
			mbRecvIOReqRelay = false;
			return;
		}
		recv(false);
	}
	std::string TcpConnection::getRemoteIP()
	{
		std::string strResult("");
		if (isClosed() || shouldBeClosed())
		{
			return strResult;
		}
		try
		{
			strResult = mkSocket.remote_endpoint().address().to_string();
		}
		catch (...)
		{
			return strResult;
		}
		return strResult;
	}
	unsigned long TcpConnection::getRemoteAddress()
	{
		unsigned long ulAddress = 0;
		if (isClosed() || shouldBeClosed())
		{
			return ulAddress;
		}
		try
		{
			ulAddress = inet_addr(getRemoteIP().c_str());
		}
		catch (...)
		{
			return ulAddress;
		}
		return ulAddress;
	}
	std::string TcpConnection::getLocalIP()
	{
		std::string strResult("");
		if (isClosed() || shouldBeClosed())
		{
			return strResult;
		}
		try
		{
			strResult = mkSocket.local_endpoint().address().to_string();
		}
		catch (...)
		{
			return strResult;
		}
		return strResult;
	}
	unsigned long TcpConnection::getLocalAddress()
	{
		unsigned long ulAddress = 0;
		if (isClosed() || shouldBeClosed())
		{
			return ulAddress;
		}
		try
		{
			ulAddress = inet_addr(getLocalIP().c_str());
		}
		catch (...)
		{
			return ulAddress;
		}
		return ulAddress;
	}
}