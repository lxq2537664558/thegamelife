#ifndef __TcpNetworkParams_h__
#define __TcpNetworkParams_h__

#include <boost/system/error_code.hpp>
#include <boost/function.hpp>
#include <boost/asio/ssl.hpp>
#include "NetworkCommonDefine.h"

namespace Network
{
	/// TcpNetwork��ʼ������
	/// @{
	enum DataProcessMethod
	{
		DPM_Stream,
		DPM_Packet,
		DPM_MAX
	};
	struct TcpNetworkParams 
	{
		unsigned int uiListenConnectionNum;///< ����socket����
		unsigned int uiSendBuffSize;///< ���ͻ�������С
		unsigned int uiRecvBuffSize;///< ���ջ�������С
		unsigned int uiPreCreateConCnt;///< Ԥ��ʼ�������Ӹ���
		unsigned int uiMaxPoolConCnt;///< ��󻺴����Ӹ��������ӳش�С��
		unsigned int uiIOThreadCount;///< IO�̸߳�����0Ϊʹ��CPU����
		bool bCloseOnBufferOverflow;///< �Ƿ��ڷ��ͻ��������߽��ջ��������ʱ�Ͽ�����
		unsigned int uiMaxConnectionWriteSize;///< ͨ��@see TcpNetwork::send����͵����ݴ�С
		/// ΪbCloseOnBufferOverflowΪfalseʱ���淢�����ݰ����ڴ�ش�С��
		/// ��������uiMaxConnectionWriteSizeΪ��С���ڴ�飬��uiExtraBufferCount��ô�����
		/// ����uiExtraBufferCount�����С����new���������bCloseOnBufferOverflowΪtrueʱ
		/// ֻҪ�ڴ��㹻�Ͳ�����Ϊ������������Ͽ�����
		/// @{
		unsigned int uiExtraBufferCount;
		/// @}
		DataProcessMethod eDataProcessMethod;///< ���ݴ���ʽ���������ǰ��ķ�ʽ
		int iPacketProcessNumLimit;///< ÿ֡��������ݰ��������ƣ�-1Ϊ������
		bool bKeepAlive;///< �Ƿ���socket��keep aliveѡ��
		/// bKeepAliveΪtrueʱ����������������Ч
		///@{
		unsigned long ulKeepAliveTime;///< ����һ�η�����ú�ʼ����keep alive���ݰ�����λ����
		unsigned long ulKeepAliveInterval;///< ���ͼ������λ����
		/// @}


		/// SSL��ز�����bUseSSLΪtrueʱ����������������
		/// @{
		bool bUseSSL;///< �Ƿ�ʹ��SSL
		boost::asio::ssl::context_base::method eSSLMethod;
		std::string strCipherList;///< Ҫʹ�õļ����׼�
		unsigned int uiSSLOption;
		unsigned int uiVerifyMode;
		bool bUseDefaultVerifyPaths;
		std::string strVerifyFile;
		SSLVerifyCallback pfnVerifyCallback;
		SSLPasswordCallback pfnPwdCallback;
		std::string strCertificateChainFile;
		std::string strCertificateFile;
		std::string strPrivateKeyFile;
		std::string strRSAPrivateKeyFile;
		std::string strDHParamFile;
		/// @}

		static const TcpNetworkParams& Default();
	};
	/// @}

	/// ����ѡ��ö��
	/// @{
	enum ConnectionType
	{
		CT_DEFAULT,
		CT_TCP,
		CT_SSL
	};
	/// @}
}

#endif//__TcpNetworkParams_h__