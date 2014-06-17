#ifndef __HttpNetworkParams_h__
#define __HttpNetworkParams_h__

#include <boost/system/error_code.hpp>
#include <boost/function.hpp>
#include <boost/asio/ssl.hpp>
#include "NetworkCommonDefine.h"

namespace Network
{
	typedef std::map<std::string, std::string> Headers;

	namespace HttpMiscStrings
	{
		static const char colon = ':';
		static const char dot = '.';
		static const char space = ' ';
		static const char* name_value_separator = ": ";
		static const unsigned int name_value_separator_length = 2;
		static const char* http = "HTTP/";
		static const unsigned int http_length = 5;
		static const char* crlf = "\r\n";
		static const unsigned int crlf_length = 2;
		static const char* version = "1.1";
		static const unsigned int version_length = 3;
	}// namespace misc_strings

	namespace HttpRequestPriority
	{
		const float highest = 0.f;
		const float normal = 1.f;
		const float lowest = 2.f;
	}

	struct HttpNetworkParams 
	{
		std::string strListenAddress;///< http��������ĵ�ַ
		unsigned int uiListenPort;///< http��������Ķ˿�
		unsigned int uiListenConnectionNum;///< ����socket����
		unsigned int uiPreCreateConCnt;///< Ԥ��ʼ�������Ӹ���
		unsigned int uiMaxPoolConCnt;///< ��󻺴����Ӹ��������ӳش�С��
		unsigned int uiIOThreadCount;///< IO�̸߳�����0Ϊʹ��CPU����
		unsigned int uiClientTimeoutSeconds;///< �ͻ�����Ӧ��ʱʱ����λ��
		unsigned int uiServerTimeoutSeconds;///< �������Ӧ��ʱʱ����λ��
		unsigned int uiRequestSendNumPerFrame;///< ÿ֡�����͵���������
		unsigned int uiResponeProcessNumPerFrame;///< ÿ֡������Ļ�Ӧ����
		unsigned int uiRequestProcessNumPerFrame;/// ÿ֡���������������
		unsigned int uiResponeSendNumPerFrame;///< ÿ֡�����͵Ļ�Ӧ����

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
		static const HttpNetworkParams& Default();
	};
}

#endif//__HttpNetworkParams_h__