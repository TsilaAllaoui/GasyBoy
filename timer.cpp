#include "timer.h"

Timer::Timer()
{

}

Timer::~Timer()
{
	mmu->~Mmu();
}

Timer::Timer(Mmu *p_mmu)
{
    mmu = p_mmu;
    divide_counter = 0;
    timer_counter = 1024;
}

void Timer::update_timer(int m_cycle)
{
	divider(m_cycle);
	if (timer_on())
	{
		timer_counter -= m_cycle;
		if (timer_counter <= 0)
		{
			setFreq();
			uint8_t val = mmu->read_ram(0xFF05);
			if (val == 0xFF)
			{
			    mmu->write_ram(0xFF05, mmu->read_ram(0xFF06));
				requestInterrupt(2);
			}
			else
                mmu->write_ram(0xFF05, val + 1);
		}
	}
}

int Timer::timer_on()
{
	uint8_t control = mmu->read_ram(0xFF07);
	return (control >> 2) & 1;
}

void Timer::setFreq()
{
	uint8_t	freq = mmu->read_ram(0xFF07) & 0x3;
	if (freq == 0x0)
		timer_counter = 1024;
	else if (freq == 0x1)
		timer_counter = 16;
	else if (freq == 0x2)
		timer_counter = 64;
	else if (freq == 0x3)
		timer_counter = 256;
}

void Timer::divider(int m_cycle)
{
	divide_counter += m_cycle;
	if (divide_counter >= 255)
	{
		divide_counter = 0;
		mmu->directSet(0xFF04, mmu->read_ram(0xFF04) + 1);
	}
}

void Timer::requestInterrupt(int id)
{
    uint8_t req = mmu->read_ram(0xFF0F);
    req |= (1 << id);
    mmu->write_ram(0xFF0F, req);
}
