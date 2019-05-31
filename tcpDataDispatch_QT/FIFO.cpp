#include <stdio.h>
#include "FIFO.h"
#include <Windows.h>

void FIFO::CopyOut(void *data, unsigned int len, unsigned int off)
{
	unsigned int size = m_mask + 1;
	unsigned int esize = m_esize;
	unsigned int l;

	off &= m_mask;
	if(esize != 1)
	{
		off *= m_esize;
		size *= m_esize;
		len *= m_esize;
	}

	l = min(len, size - off);

	memcpy((char *)data, (char *)m_buf + off, l);
	memcpy((char *)data + l, (char *)m_buf, len - l);
}

unsigned int FIFO::FIFO_Out_Peek(void *data, unsigned int cnt)
{
	unsigned int l = FIFO_USED_SLOTS();
	if(cnt > l)
	{
		cnt = l;
	}
	CopyOut(data, cnt, m_out);
	return cnt;
}


FIFO::FIFO()
{

}

bool FIFO::FIFO_Init(unsigned int slot_cnt, unsigned int esize)
{
	slot_cnt = rounddown_to_power_2(slot_cnt);
	if (slot_cnt < 2)
	{
		m_mask = 0;
		return false;
	}
	m_mask = slot_cnt - 1;
	m_esize = esize;
	m_out = 0;
	m_in = 0;

	return true;
}


void FIFO::CopyIn(void *data, unsigned int len, unsigned int off)
{
	unsigned int size = m_mask + 1;
	unsigned int esize = m_esize;
	unsigned int l;

	off &= m_mask;
	if (m_esize != 1)
	{
		off *= m_esize;
		size *= m_esize;
		len *= m_esize;
	}
	l = min(len, size - off);

	memcpy((char *)m_buf + off, (char *)data, l);
	memcpy((char *)m_buf, (char *)data + l, len - l);
}


void FIFO::FIFO_In(void *data, unsigned int cnt)
{
	unsigned int l;
	l = FIFO_UNUSED_SLOTS();
	if (cnt > l)
	{
		cnt = l;
	}
	CopyIn(data, cnt, m_in);
	m_in += cnt;

}

unsigned int FIFO::FIFO_Out(void *data, unsigned int cnt)
{
	cnt = FIFO_Out_Peek(data, cnt);
	m_out += cnt;
	
	return cnt;
}


#if 0
bool Event::createEvent(bool manualReset, bool initState)
{
	mhEvent = CreateEventA(NULL, manualReset, initState, "Global\\myEvent");
	if (mhEvent == NULL) {
		return false;
	}
	return true;
}

bool Event::openEvent()
{
	mhEvent = OpenEventA(EVENT_ALL_ACCESS, false, "Global\\myEvent");
	if (mhEvent == NULL) {
		return false;
	}
	return true;
}

bool Event::setEvent()
{
	return SetEvent(mhEvent);
}

bool Event::resetEvent()
{
	return ResetEvent(mhEvent);
}

extern "C"
{
	FIFO obj;
	Event evt;

	DLL_FIFO_API bool Create_Event(bool manualReset, bool initState)
	{
		return evt.createEvent(manualReset, initState);
	}

	DLL_FIFO_API bool Set_Event()
	{
		bool b = evt.setEvent();
		printf("Set_Event: %d\n", b);
		return b;
	}

	DLL_FIFO_API bool Reset_Event()
	{
		bool b = evt.resetEvent();
		printf("Reset_Event: %d\n", b);
		return b;
	}

	DLL_FIFO_API bool Open_Event()
	{
		bool b = evt.openEvent();
		printf("Open_Event: %d\n", b);
		return b;
	}

	DLL_FIFO_API void FIFO_Init(char *pName, unsigned int size, unsigned int cnt, unsigned int esize)
	{
		obj.FIFO_Init(pName, size, cnt, esize);
	}

	DLL_FIFO_API int FIFO_Open(unsigned int cnt, unsigned int esize)
	{
		return obj.FIFO_Open(cnt, esize);
	}

	DLL_FIFO_API void FIFO_In(void *data, unsigned int cnt)
	{
		obj.FIFO_In(data, cnt);
	}

	DLL_FIFO_API unsigned int FIFO_Out(void *data, unsigned int cnt)
	{
		return obj.FIFO_Out(data, cnt);
	}

	DLL_FIFO_API unsigned int FIFO_Out_Peek(void *data, unsigned int cnt)
	{
		return obj.FIFO_Out_Peek(data, cnt);
	}

};
#endif