#include "packparser1.h"
#include "pack1.h"
#include "myhandler.h"

class CPacketParser :  public IPacketParser
{
public:
	CPacketParser(TcpHandler * pHandler):IPacketParser(pHandler)
	{
		m_pBuf = m_Packet.packet_buf();
		m_version = NETInputPacket::SERVER_PACKET_DEFAULT_VER;
		m_subVersion = NETInputPacket::SERVER_PACKET_DEFAULT_SUBVER;
		reset();
	}

	virtual ~CPacketParser(void){}

	// ����PACKET����
	int ParsePacket(const char *data, const size_t length)
	{
		//reset();

		int ret = -1; //������
		size_t ndx = 0;
		while(ndx < length && m_nStatus != REQ_ERROR)//���ܻ�ͬʱ�������� 
		{
			switch(m_nStatus)
			{
			case REQ_REQUEST:
			case REQ_HEADER:
				if(!read_header(data, length, ndx))
					break;
				ret = parse_header();
				if(ret != 0)
				{
					m_nStatus = REQ_ERROR;
					break;
				}else
					m_nStatus = REQ_BODY;
			case REQ_BODY:
				if(parse_body(data, length, ndx))
					m_nStatus = REQ_DONE;
				break;
			default:
                {
                    //log_debug("parse error state return -1 \n");
                    return -1;
                }
			}
			if(m_nStatus == REQ_ERROR)
            {
                //log_debug("parse state is REQ_ERROR \n");
                reset();
            }
			if(m_nStatus == REQ_DONE)
			{
				m_pHandler->OnPacketComplete(&m_Packet);
				this->reset();
			}
		}

		return 0; // return 0 to continue
	}
protected:
	void reset(void)
	{
		m_nStatus = REQ_REQUEST;
		m_nPacketPos = 0;
		m_nBodyLen = 0;
		m_Packet.Reset();//����ɸ�λ
	}

public:
	short m_version;
	short m_subVersion;

private:
	// ��ǰ����״̬
	int m_nStatus; 
	// PacketPos
	size_t	m_nPacketPos;
	// BODY����
	size_t m_nBodyLen;
	// PacketBuffer ָ��
	char *m_pBuf;
	// PacketBuffer
	NETInputPacket m_Packet;
	// ״̬
	enum REQSTATUS{	REQ_REQUEST=0, REQ_HEADER, REQ_BODY, REQ_DONE, REQ_PROCESS, REQ_ERROR };

private:

	// ��ȡPacketͷ����
	bool read_header(const char *data, const size_t length, size_t & ndx)
	{
		if (0 == ndx)  //��Ѷ����������һ��m_nPacketPos��Ϊ������⣬�ᵼ�½��ʧ�ܣ�socket�رգ������ȹ����
		{
			//RAW_LOG_ERROR("read_header, m_nPacketPos=%d", m_nPacketPos);
			m_nPacketPos = 0;			
		}
		
		while(m_nPacketPos < NETInputPacket::PACKET_HEADER_SIZE && ndx < length)//
		{
			m_pBuf[m_nPacketPos++] = data[ndx++];
		}
		if(m_nPacketPos < NETInputPacket::PACKET_HEADER_SIZE)
			return false;

		return true;
	}
	// ����Packetͷ��Ϣ
	int parse_header(void) //0:�ɹ� -1:������ -2:���Χ���� -3:�汾���� -4:���ȴ���
	{
		if(m_pBuf[0] != 'I' || m_pBuf[1] != 'C')
			return -1;

		short nCmdType = m_Packet.GetCmdType();
		if(nCmdType < 0 || nCmdType >= 32000)
			return -2;

		if(m_Packet.GetVersion() != m_version || m_Packet.GetSubVersion() != m_subVersion)
			return -3;

		m_nBodyLen = m_Packet.GetBodyLength();
		if(m_nBodyLen < (NETInputPacket::PACKET_BUFFER_SIZE - NETInputPacket::PACKET_HEADER_SIZE))
			return 0;
		return -4;
	}
	// ����BODY����
	bool parse_body(const char *data, const size_t length, size_t & ndx)
	{
		size_t nNeed = (m_nBodyLen + NETInputPacket::PACKET_HEADER_SIZE) - m_nPacketPos;
		size_t nBuff = length - ndx;

		if(nNeed <= 0)
			return true;
		if(nBuff <= 0)
			return false;

		size_t nCopy = nBuff < nNeed ? nBuff : nNeed;
		if(!m_Packet.WriteBody(data + ndx,  static_cast<int>(nCopy)))
			return false;

		m_nPacketPos += nCopy;
		ndx += nCopy;

		if(m_nPacketPos < (m_nBodyLen + NETInputPacket::PACKET_HEADER_SIZE))
			return false;

		return true;
	}
};

IPacketParser * IPacketParser::CreateObject(TcpHandler * pObject)
{
	return new CPacketParser(pObject);
}
