#pragma once

#ifndef FIFO_H_
#define FIFO_H_

class FIFO
{
public:
	FIFO();
	bool FIFO_Init(unsigned int slot_cnt, unsigned int esize); // slot_cnt是环形缓冲区槽位个数,esize是环形缓冲区每个槽位元素大小
	void FIFO_In(void *data, unsigned int cnt);
	unsigned int FIFO_Out(void *data, unsigned int cnt);
	unsigned int FIFO_Out_Peek(void *data, unsigned int cnt);
	
private:
	unsigned int rounddown_to_power_2(unsigned int size)/* 舍入至2的整数幂 */
	{
		unsigned int data = 0;

		for (data = 1; data < size; data <<= 1);

		return (data == size) ? size : (data >> 1);
	}

	int FIFO_USED_SLOTS()
	{
		return m_in - m_out;
	}

	int FIFO_UNUSED_SLOTS()
	{
		return (m_mask + 1) - FIFO_USED_SLOTS();
	}

	void CopyIn(void *data, unsigned int len, unsigned int off);
	void CopyOut(void *data, unsigned int len, unsigned int off);

	char m_buf[160];
	unsigned int m_in;
	unsigned int m_out;
	unsigned int  m_esize;
	unsigned int  m_mask;
};

#endif