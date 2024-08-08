#include "timer.h"

namespace gasyboy
{
	Timer::Timer(Mmu &mmu, InterruptManager &interruptManager)
		: _mmu(mmu),
		  _interruptManager(interruptManager),
		  _divideCounter(0),
		  _timerCounter(1024)
	{
	}

	bool Timer::isTimerOn()
	{
		uint8_t control = _mmu.readRam(0xFF07);
		return (control >> 2) & 1;
	}

	void Timer::updateTimer(const int &cycle)
	{
		updateDivider(cycle);

		if (isTimerOn())
		{
			_timerCounter -= cycle;

			if (_timerCounter <= 0)
			{
				updateFrequence();

				uint8_t val = _mmu.readRam(0xFF05);

				if (val == 0xFF)
				{
					_mmu.writeRam(0xFF05, _mmu.readRam(0xFF06));
					_interruptManager.requestInterrupt(InterruptManager::InterruptType::Timer);
				}
				else
				{
					_mmu.writeRam(0xFF05, val + 1);
				}
			}
		}
	}

	void Timer::updateFrequence()
	{
		uint8_t freq = _mmu.readRam(0xFF07) & 0x3;

		if (freq == 0x0)
			_timerCounter = 1024;
		else if (freq == 0x1)
			_timerCounter = 16;
		else if (freq == 0x2)
			_timerCounter = 64;
		else if (freq == 0x3)
			_timerCounter = 256;
	}

	void Timer::updateDivider(const int &cycle)
	{
		_divideCounter += cycle;

		if (_divideCounter >= 255)
		{
			_divideCounter = 0;
			_mmu.directSet(0xFF04, _mmu.readRam(0xFF04) + 1);
		}
	}
}