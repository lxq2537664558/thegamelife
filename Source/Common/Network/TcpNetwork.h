#ifndef __TcpNetwork_h__
#define __TcpNetwork_h__

#include "TcpConnection.h"
#include "SSLConnection.h"
#include "ConnectionIDGenerator.h"
#include "TcpNetworkParams.h"
#include "TcpNetworkErrorCode.h"

namespace Network
{
	class IdealConnectionContainer;
	class ActiveConnectionContainer;

	class TcpNetwork
	{
	public:
		typedef std::list<ConnectionPtr> ConnectionList;
		typedef std::map<ConnectionID, ConnectionPtr> ConnectionMap;
		typedef boost::function<void(ConnectionID uConID)> OnConFunc;
		typedef boost::function<void(ConnectionID uConID)> OnDisConFunc;
		typedef boost::function<void(ConnectionID uConID, const unsigned char* acData, unsigned int uSize)> OnRecvFunc;
		typedef boost::function<bool(ConnectionID uConID, const unsigned char* acPacket, unsigned int uPacketSize)> OnPacketFunc;
		typedef boost::function<void(ConnectionID uConID, unsigned int uSize)> OnSentFunc;
		typedef boost::function<void(ConnectionID uConID, const boost::system::error_code& kErrorCode)> ErrorHandleFunc;

	public:
		TcpNetwork();
		/// ����TcpNetwork
		bool startup(const TcpNetworkParams& pkParams = TcpNetworkParams::Default());
		/// ����Ƿ�������ʹ����Ҫ����listenȥ����ָ����IP�Ͷ˿�
		bool listen(const char* pcListenIP, unsigned int uiListenPort);
		/// ��������ָ��IP�͵�ַ
		ConnectionID connect(const char* pcServerIP, 
			unsigned int uiServerPort, ConnectionType eConnectionType = CT_DEFAULT);
		/// �첽����ָ��IP�͵�ַ�����ӽ���ص�ͨ��@see bindOnConnected����
		ConnectionID asyncConnect(const char* pcServerIP, 
			unsigned int uiServerPort, ConnectionType eConnectionType = CT_DEFAULT);
		/// �첽�������ݣ�������ɻص�ͨ��@see bindOnSent����
		// @bWait :  true: �������������, ����false; false: �������������, �Ͽ���new�µĻ�����.
		bool send(ConnectionID uConID, unsigned char* acData, unsigned int uSize);
		/// ����ʽ�Ͽ�ָ��ID������
		bool closeConnection(ConnectionID uConID);
		/// �첽�Ͽ�ָ��ID�����ӣ��Ͽ��ص�ͨ��@see bindOnDisconnected����
		bool asyncCloseConnection(ConnectionID uConID);
		/// ��ѭ��
		void running();
		/// ǿ���˳���������ӵĽ��ջ����������ݻᱻ���������ͻ�������������˲��ᱻ������
		bool shutdown();
		/// ֻ�е������Ѿ����յ����ݴ����꣬�������������Ѿ��رգ��Źرպͷ���true
		bool tryShutdown();
		/// ���ӱ����ܺ�����һ������@see mkWaitActiveConList
		/// handleWaitForActivingConnections()���������ᱻ����mpkActiveConContainer
		/// ֻ����mpkActiveConContainer������Ӳſ��Խ��պͷ�������
		bool isActived(ConnectionID uID);
		/// ��ȡ���ӵ�std::string��ʽ��Զ��IP��ַ
		std::string getConnectionRemoteIP(ConnectionID uID);
		/// ��ȡ���ӵ�unsigned long��ʽ��Զ��IP��ַ
		unsigned long getConnectionRemoteAddress(ConnectionID uID);
		/// ��ȡ���ӵ�std::string��ʽ�ı���IP��ַ
		std::string getConnectionLocalIP(ConnectionID uID);
		/// ��ȡ���ӵ�unsigned long��ʽ�ı���IP��ַ
		unsigned long getConnectionLocalAddress(ConnectionID uID);
		/// ��ȡ�����ʼ������
		const Network::TcpNetworkParams& getNetworkParames();

		/// �ص����ã����лص����������̵߳���
		/// @{
		/// ����������ɻص�
		void bindOnConnected(OnConFunc pOnConFunc);
		/// �������ӶϿ��ص�
		void bindOnDisconnected(OnDisConFunc pOnDisConFunc);
		/// ��@see bindOnPacket���⣬����Ѿ����ù�@see bindOnPacket�򷵻�false
		bool bindOnRecved(OnRecvFunc pOnRecvFunc);
		/// ��@see bindOnRecved���⣬����Ѿ����ù�@see bindOnRecved�򷵻�false
		/// OnPacketFunc����bool��ʾ�Ƿ�Ҫ��������δ�����������
		/// ����false��ʾ��ǰ���������ͽ�����ǰ������һ֡���������ݴ���
		bool bindOnPacket(OnPacketFunc pOnPacketFunc);
		/// ���÷��ͽ���ص�
		void bindOnSent(OnSentFunc pOnSentFunc);
		/// �������ӳ���ص�
		void bindOnError(ErrorHandleFunc pOnSentFunc);
		/// @}

	protected:
		bool startAccept();
		void onAccept(ConnectionPtr& pkConnection,
			const boost::system::error_code& kError);
		void onConnect(ConnectionPtr& pkConnection,
			boost::system::error_code& kErrorCode);
		void onHandshake(ConnectionPtr& pkConnection,
			boost::system::error_code& kErrorCode);

		void handleWaitForActivingConnections();
		void handleAllActivedConnections();
		void handleErrorConnections();
		void checkAndHandleConnectionErrors(ConnectionPtr& pkConnection);
		void visitAciveConnection(ConnectionID uID, ConnectionPtr& pkConnection);

	protected:
		std::string mstrListenIP;
		unsigned int muiListenPort;
		unsigned char* mpcRecvTmpBuffer;
		boost::asio::io_service mkIOService;
		boost::asio::io_service::work *mpkIOWork;
		boost::asio::ip::tcp::acceptor mkAcceptor;
		boost::thread_group mkThreadGroup;
		TcpNetworkParams mkNetworkParams;
		IdealConnectionContainer* mpkIdealConContainer;
		ActiveConnectionContainer* mpkActiveConContainer;
		ConnectionList mkWaitActiveConList;
		boost::mutex mkWaitActiveConListMutex;
		ConnectionMap mkHandshakingConMap;
		boost::mutex mkHandshakingConMapMutex;
		ConnectionList mkErrorConList;
		boost::mutex mkErrorConListMutex;
		OnConFunc mpOnConFunc;
		OnDisConFunc mpOnDisConFunc;
		OnRecvFunc mpOnRecvFunc;        ///< ��mpOnPacketFunc���⣬ֻ����һ����ֵ
		OnPacketFunc mpOnPacketFunc;    ///< ��mpOnRecvFunc���⣬ֻ����һ����ֵ
		OnSentFunc mpOnSentFunc;
		ErrorHandleFunc mpOnErrorHandleFunc;
		bool mbStartup;
		boost::asio::ssl::context* mpkSSLContext;///< SSL������
	};

	inline void TcpNetwork::bindOnConnected(OnConFunc pOnConFunc)
	{
		mpOnConFunc = pOnConFunc;
	}
	inline void TcpNetwork::bindOnDisconnected(OnDisConFunc pOnDisConFunc)
	{
		mpOnDisConFunc = pOnDisConFunc;
	}
	inline bool TcpNetwork::bindOnRecved(OnRecvFunc pOnRecvFunc)
	{
		if (mpOnPacketFunc)
		{
			return false;
		}
		mpOnRecvFunc = pOnRecvFunc;
		return true;
	}
	inline bool TcpNetwork::bindOnPacket(OnPacketFunc pOnPacketFunc)
	{
		if (mpOnRecvFunc)
		{
			return false;
		}
		mpOnPacketFunc = pOnPacketFunc;
		return true;
	}
	inline void TcpNetwork::bindOnSent(OnSentFunc pOnSentFunc)
	{
		mpOnSentFunc = pOnSentFunc;
	}
	inline void TcpNetwork::bindOnError(ErrorHandleFunc pOnErrorHandleFunc)
	{
		mpOnErrorHandleFunc = pOnErrorHandleFunc;
	}
	inline const Network::TcpNetworkParams& TcpNetwork::getNetworkParames()
	{
		return mkNetworkParams;
	}
}

#endif//__TcpNetwork_h__