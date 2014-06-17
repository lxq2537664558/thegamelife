#ifndef __RingBuffer_h__
#define __RingBuffer_h__

namespace Network
{
	/// ��ʼHead = 0, Tail = 1
	/// Tail - Head = 1ʱΪ��
	/// Tail - Head = 0ʱΪ��
	/// 0 <= Head < Size
	/// 0 <= Tail < Size
	/// ���ֶ�m_uiHead, m_uiTailֻ����ԭ�Ӳ���
	/// Tailָ���Ԫ����ֱ�ӿ�д��Ԫ��
	/// Head + 1ָ���Ԫ����ֱ�ӿɶ���Ԫ��

	template<typename T>
	class RingBuffer
	{
	public:
		/// @param uiSize ����
		/// @param uiMaxPacketSize ������С
		RingBuffer(unsigned int uiSize, unsigned int uiMaxPacketSize)
			: m_uiTotalSize(uiSize + 1), m_uiMaxPacketSize(uiMaxPacketSize),
			m_uiHead(0), m_uiTail(1), m_bReading(false), m_bWritting(false),
			m_uiPacketFromBufferHeadSize(0)
		{
			/// ������m_uiMaxPacketSize����@see readPacketBegin
			/// ��@see readPacketEnd��ʵ��
			m_pkBuffer = new T[m_uiTotalSize + m_uiMaxPacketSize];
		}
		/// ��ջ���
		~RingBuffer()
		{
			delete[] m_pkBuffer;
		}

		void clear()
		{
			m_uiHead = 0;
			m_uiTail = 1;
		}

		/// ��pkBuffer�ڵ�uiSize���Ƶ�RingBuffer���п����ݷֲ���RingBufferͷβ��������
		/// �ú���ֻ�޸�m_uiTail���޸�m_uiHead��������m_uiHead���ܻᱻ���������޸�
		bool writeImmediately(T *pkBuffer, unsigned int uiSize)
		{
			unsigned int uiHead = m_uiHead;
			if (uiSize > writableSize(uiHead, m_uiTail) || !pkBuffer)
			{
				return false;
			}
			unsigned int uiNextTail = 0;
			if (m_uiTail > uiHead)
			{
				unsigned int uiDivider = m_uiTotalSize - m_uiTail;
				if (uiDivider > uiSize)
				{
					memcpy(m_pkBuffer + m_uiTail, pkBuffer, uiSize * sizeof(T));
					uiNextTail = m_uiTail + uiSize;
				}
				else if (uiDivider < uiSize)
				{
					memcpy(m_pkBuffer + m_uiTail, pkBuffer, uiDivider * sizeof(T));
					uiNextTail = uiSize - uiDivider;
					memcpy(m_pkBuffer, pkBuffer + uiDivider, uiNextTail * sizeof(T));
				}
				else //if (uiDivider == uiSize)
				{
					memcpy(m_pkBuffer + m_uiTail, pkBuffer, uiSize * sizeof(T));
					uiNextTail = 0;
				}
			}
			else
			{
				memcpy(m_pkBuffer + m_uiTail, pkBuffer, uiSize * sizeof(T));
				uiNextTail = m_uiTail + uiSize;
			}
			m_uiTail = uiNextTail;
			return true;
		}
		/// ȡ��RingBuffer������д����ʼ��ַ����д�Ĵ�С��Ȼ���pkBuffer��д����
		/// ������Ҫ����@see writeEnd
		/// �������д�ŷ���false
		/// ���޸�m_uiHead�����޸�m_uiTail��������m_uiHead���ܻᱻ�����߳��޸�
		bool writeBegin(T *&pkBuffer, unsigned int &uiWritableSize)
		{
			if (m_bWritting)
			{
				return false;
			}
			m_bWritting = true;
			unsigned int uiHead = m_uiHead;
			if (m_uiTail == m_uiTotalSize)
			{
				pkBuffer = m_pkBuffer;
			}
			else
			{
				pkBuffer = m_pkBuffer + m_uiTail;
			}
			uiWritableSize = consequentMemWritableSize(uiHead, m_uiTail);
			return true;
		}
		/// ��Ӧ@see writeBegin, ����RingBufferд���˶�������@param uiWroteSize
		/// ���޸�m_uiHead���޸�m_uiTail��������m_uiHead���ܻᱻ�����߳��޸�
		void writeEnd(unsigned int uiWroteSize)
		{
			m_uiTail = (m_uiTail + uiWroteSize) % m_uiTotalSize;
			m_bWritting = false;
		}
		/// ��ȡ���uiBuffSize���������п��ܶ�����������RingBuffer�в����������ڴ��
		/// �޸�m_uiHead�����޸�m_uiTail��������m_uiTail���ܻᱻ�����߳��޸�
		unsigned int readImmediately(T *pkBuffer, unsigned int uiBuffSize)
		{
			unsigned int uiTail = m_uiTail;
			unsigned int uiReadableSize = readableSize(m_uiHead, uiTail);
			if (uiReadableSize == 0 || !pkBuffer)
			{
				return 0;
			}
			unsigned int uiSizeToRead = uiReadableSize < uiBuffSize ? uiReadableSize : uiBuffSize;
			unsigned int uiNextHead = 0;
			if (uiTail > m_uiHead)
			{
				memcpy(pkBuffer, m_pkBuffer + (m_uiHead + 1), uiSizeToRead * sizeof(T));
				uiNextHead = m_uiHead + uiSizeToRead;
			}
			else
			{
				unsigned int uiDivider = m_uiTotalSize - (m_uiHead + 1);
				if (uiSizeToRead <= uiDivider)
				{
					memcpy(pkBuffer, m_pkBuffer + (m_uiHead + 1), uiSizeToRead * sizeof(T));
					uiNextHead = m_uiHead + uiSizeToRead;
				}
				else if (uiSizeToRead > uiDivider)
				{
					memcpy(pkBuffer, m_pkBuffer + (m_uiHead + 1), uiDivider * sizeof(T));
					uiNextHead = uiSizeToRead - uiDivider;
					memcpy(pkBuffer + uiDivider, m_pkBuffer, uiNextHead * sizeof(T));
					uiNextHead -= 1;
				}
			}
			m_uiHead = uiNextHead;
			return uiSizeToRead;
		}
		/// ȡ���������Զ����ڴ��ַ@param pkBuffer���ɶ��Ĵ�С@param uiReadableSize
		/// ��@param pkBuffer��ȡ�������@param uiReadableSize����Ҫ����readEnd
		/// ������ڶ��ŷ���false
		/// ���޸�m_uiHead�����޸�m_uiTail��������m_uiTail���ܻᱻ�����߳��޸�
		bool readBegin(T *&pkBuffer, unsigned int &uiReadableSize)
		{
			if (m_bReading)
			{
				return false;
			}
			unsigned int uiTail = m_uiTail;
			m_bReading = true;
			if (m_uiHead + 1 == m_uiTotalSize)
			{
				pkBuffer = m_pkBuffer;
			}
			else
			{
				pkBuffer = m_pkBuffer + (m_uiHead + 1);
			}
			uiReadableSize = consequentMemReadableSize(m_uiHead, uiTail);
			return true;
		}
		/// ��Ӧ@see readBegin, ����RingBuffer���˶�������@param uiReadSize
		/// �޸�m_uiHead�����޸�m_uiTail��������m_uiTail���ܻᱻ�����߳��޸�
		void readEnd(unsigned int uiReadSize)
		{
			m_uiHead = (m_uiHead + uiReadSize) % m_uiTotalSize;
			m_bReading = false;
		}
		/// ��һ���������Ŀɶ��ڴ��С
		unsigned int readableSize()
		{
			return readableSize(m_uiHead, m_uiTail);
		}
		/// ��һ���������Ŀ�д�ڴ��С
		unsigned int writableSize()
		{
			return writableSize(m_uiHead, m_uiTail);
		}
		/// RingBuffer����
		unsigned int capacity()
		{
			return m_uiTotalSize - 1;
		}
		/// �Ƿ������@see readBegin����δ����@see readEnd
		bool isReading()
		{
			return m_bReading;
		}
		/// �Ƿ������@see writeBegin����δ����@see writeEnd
		bool isWritting()
		{
			return m_bWritting;
		}
		/// ������ʱȡ����֤���ٰ���һ�����ݰ��������ڴ��ַ
		/// ��Ҫ��@see readPacketEnd��Ӧ����
		/// ����ǰsizeof(unsigned int)���ֽڱ�ʾ���Ĵ�С
		/// ֻ�е�ǰ���ڶ�����false, uiReadableSize��Ϊ0
		/// �����ǰû�����������ݰ�����true, uiReadableSize��Ϊ0
		bool readPacketsBegin(T *&pkBuffer, unsigned int &uiReadableSize)
		{
			if (m_bReading)
			{
				return false;
			}
			uiReadableSize = 0;
			unsigned int uiTail = m_uiTail;
			m_bReading = true;
			unsigned int uiTotalReadableSize = readableSize(m_uiHead, uiTail);
			const unsigned int uiBitOfSize = sizeof(unsigned int);
			if (uiTotalReadableSize <= uiBitOfSize)
			{
				return true;
			}
			m_uiPacketFromBufferHeadSize = 0;
			unsigned int uiConsequentMemReadableSize = consequentMemReadableSize(m_uiHead, uiTail);
			if (uiConsequentMemReadableSize < uiBitOfSize)
			{
				m_uiPacketFromBufferHeadSize = uiBitOfSize - uiConsequentMemReadableSize;
#ifdef WIN32
				memcpy_s(m_pkBuffer + m_uiHead + 1 + uiConsequentMemReadableSize, m_uiMaxPacketSize,
					m_pkBuffer, m_uiPacketFromBufferHeadSize);
#else
				memcpy(m_pkBuffer + m_uiHead + 1 + uiConsequentMemReadableSize,
					m_pkBuffer, m_uiPacketFromBufferHeadSize);
#endif // WIN32
			}
			unsigned int uiCurrentPacketSize = 0;
			if (m_uiTotalSize - m_uiHead == 1)
			{
				uiCurrentPacketSize = *((unsigned int*)m_pkBuffer);
				pkBuffer = m_pkBuffer;
			}
			else
			{
				uiCurrentPacketSize = *((unsigned int*)(m_pkBuffer + m_uiHead + 1));
				pkBuffer = m_pkBuffer + m_uiHead + 1;
			}
			if (uiCurrentPacketSize > uiTotalReadableSize)
			{
				m_uiPacketFromBufferHeadSize = 0;
				return true;
			}
			if (uiCurrentPacketSize > uiConsequentMemReadableSize)
			{
				m_uiPacketFromBufferHeadSize = uiCurrentPacketSize - uiConsequentMemReadableSize;
#ifdef WIN32
				memcpy_s(m_pkBuffer + m_uiHead + 1 + uiConsequentMemReadableSize, m_uiMaxPacketSize,
					m_pkBuffer, m_uiPacketFromBufferHeadSize);
#else
				memcpy(m_pkBuffer + m_uiHead + 1 + uiConsequentMemReadableSize,
					m_pkBuffer, m_uiPacketFromBufferHeadSize);
#endif // WIN32
			}
			if (m_uiPacketFromBufferHeadSize == 0)
			{
				uiReadableSize = uiConsequentMemReadableSize;
			}
			else
			{
				uiReadableSize = uiConsequentMemReadableSize + m_uiPacketFromBufferHeadSize;
			}
			return true;
		}
		/// ��Ӧ@see readPacketBegin, ����RingBuffer���˶��������ڴ�@param uiReadSize
		/// �޸�m_uiHead�����޸�m_uiTail��������m_uiTail���ܻᱻ�����߳��޸�
		void readPacketEnd(unsigned int uiReadSize)
		{
			if (m_uiPacketFromBufferHeadSize == 0)
			{
				m_uiHead = (m_uiHead + uiReadSize) % m_uiTotalSize;
			}
			else
			{
				m_uiHead = m_uiPacketFromBufferHeadSize - 1;
			}
			m_bReading = false;
		}
		/// �жϵ�ǰ����С�Ƿ����
		bool packetSizeValid()
		{
			return readableSize() >= sizeof(unsigned int);
		}
		/// ��ȡ��ǰ����С��Ĭ��ǰ���ֽ�Ϊ����С
		bool currentPacketSize(unsigned int& uiPacketSize)
		{
			unsigned int uiCurrentPacketSize = (unsigned int)-1;
			unsigned int uiTail = m_uiTail;
			unsigned int uiTotalReadableSize = readableSize(m_uiHead, uiTail);
			const unsigned int uiBitOfSize = sizeof(unsigned int);
			if (uiTotalReadableSize < uiBitOfSize)
			{
				uiPacketSize = uiCurrentPacketSize;
				return false;
			}
			unsigned int uiConsequentMemReadableSize = consequentMemReadableSize(m_uiHead, uiTail);
			if (uiConsequentMemReadableSize < uiBitOfSize)
			{
#ifdef WIN32
				memcpy_s(&uiCurrentPacketSize, uiBitOfSize,
					m_pkBuffer + m_uiHead + 1, uiConsequentMemReadableSize);
				memcpy_s((((char*)&uiCurrentPacketSize) + uiConsequentMemReadableSize), 
					uiBitOfSize - uiConsequentMemReadableSize,
					m_pkBuffer, uiBitOfSize - uiConsequentMemReadableSize);
#else
				memcpy(&uiCurrentPacketSize, m_pkBuffer + m_uiHead + 1, uiConsequentMemReadableSize);
				memcpy((((char*)&uiCurrentPacketSize) + uiConsequentMemReadableSize),
					m_pkBuffer, uiBitOfSize - uiConsequentMemReadableSize);
#endif // WIN32
			}
			else
			{
				if (m_uiHead + 1 == m_uiTotalSize)
				{
					uiCurrentPacketSize = *((unsigned int*)m_pkBuffer);
				}
				else
				{
					uiCurrentPacketSize = *((unsigned int*)(m_pkBuffer + m_uiHead + 1));
				}
			}
			uiPacketSize = uiCurrentPacketSize;
			return true;
		}

	private:
		/// �п��ܷ������Ŀɶ��ڴ���С
		unsigned int readableSize(unsigned int uiHead, unsigned int uiTail)
		{
			if (uiTail > uiHead)
			{
				return uiTail - (uiHead + 1);
			}
			return m_uiTotalSize - (uiHead + 1) + uiTail;
		}
		/// �п��ܷ������Ŀ�д�ڴ���С
		unsigned int writableSize(unsigned int uiHead, unsigned int uiTail)
		{
			if (uiTail > uiHead)
			{
				return m_uiTotalSize - uiTail + uiHead;
			}
			return uiHead - uiTail;
		}
		/// �ɶ��������ڴ���С��ʹ�ô˽ӿڵ�ʱ��ע��m_uiTotalSize -  m_uiHead == 1��ʱ��
		/// �ɶ�����ʼ�ڴ��ַ��m_pkBuffer��������m_pkBuffer + m_uiHead + 1
		unsigned int consequentMemReadableSize(unsigned int uiHead, unsigned int uiTail)
		{
			if (uiTail > uiHead)
			{
				return uiTail - (uiHead + 1);
			}
			else
			{
				if (m_uiTotalSize -  uiHead == 1)
				{
					return uiTail;
				}
				else
				{
					return m_uiTotalSize - (uiHead + 1);
				}
			}
		}
		/// ��д�������ڴ���С��ʹ�ô˽ӿڵ�ʱ��ע��m_uiTotalSize -  m_uiTail == 0��ʱ��
		/// ��д����ʼ�ڴ��ַ��m_pkBuffer��������m_pkBuffer + m_uiTail
		unsigned int consequentMemWritableSize(unsigned int uiHead, unsigned int uiTail)
		{
			if (uiTail > uiHead)
			{
				if (uiTail == m_uiTotalSize)
				{
					return uiHead;
				}
				return m_uiTotalSize - uiTail;
			}
			return uiHead - uiTail;
		}

	private:
		/// ������RingBuffer���ڴ�β��Ҫ����һ����ʱ��
		/// ���������¼��m_pkBuffer������ڴ��Ѿ���
		/// ���Ƶ�ĩβ��������һ����
		/// @{
		unsigned int volatile m_uiPacketFromBufferHeadSize;
		/// @}
		unsigned int m_uiTotalSize;      ///< ����RingBuffer�Ļ����С
		unsigned int m_uiMaxPacketSize;  ///< ������С
		unsigned int volatile m_uiHead;  ///< m_uiHead + 1Ϊֱ�ӿɶ��ڴ���ʼλ��
		unsigned int volatile m_uiTail;  ///< m_uiTailΪֱ�ӿ�д�ڴ���ʼλ��
		bool volatile m_bReading;
		bool volatile m_bWritting;
		T *m_pkBuffer;

		friend class TcpConnection;
	};
}

#endif //__RingBuffer_h__