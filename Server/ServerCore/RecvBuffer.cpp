#include "ServerCorePch.h"
#include "RecvBuffer.h"
#include "AtomicNonTemplate.h"

namespace ServerCore
{
	RecvBuffer::RecvBuffer(c_int32 bufferSize_)noexcept
		: m_bufferSize{ bufferSize_ }
		, m_capacity{ bufferSize_ * BUFFER_COUNT }
	{
	}

	RecvBuffer::~RecvBuffer()
	{
	}

	void RecvBuffer::Clear()noexcept
	{
		const int32 dataSize = DataSize();
		if (0 == dataSize)
		{
			// ����Ŀ���� ����ƮĿ���� �� ��ġ�� ��Ȳ (�˸°� �� ����) �׳� 0���� ���� ������.
			m_readPos = m_writePos = 0;
		}
		else
		{
			// ��¿ �� ���� ������ ����� ������ �߻�
			// ���� ������ ���� 1�� ũ�� �̸��̸� ���
			if (FreeSize() < static_cast<c_int32>(RECV_BUFFER_SIZE::BUFFER_SIZE >> 1))
			{
				::memcpy(m_buffer, m_buffer + m_readPos, dataSize);
				m_readPos = 0;
				m_writePos = dataSize;
			}
		}
	}
}
