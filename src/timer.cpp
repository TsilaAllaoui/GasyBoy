#include "interruptManagerProvider.h"
#include "timer.h"

namespace gasyboy
{
	uint8_t Timer::_div = 0;
	uint8_t Timer::_tima = 0;
	uint8_t Timer::_tma = 0;
	uint8_t Timer::_tac = 0;

	int Timer::_divIncrementRate = 255;
	int Timer::_timaIncrementRate = 1024;

	Timer::Timer()
	{
		_div = 0;
		_tima = 0;
		_tma = 0;
		_tac = 0;
		_divIncrementRate = 255;
		_timaIncrementRate = 1024;
	}

	Timer &Timer::operator=(const Timer &other)
	{
		_div = other._div;
		_tima = other._tima;
		_tma = other._tma;
		_tac = other._tac;
		_divIncrementRate = other._divIncrementRate;
		_timaIncrementRate = other._timaIncrementRate;
		return *this;
	}

	void Timer::reset()
	{
	}

	uint8_t Timer::DIV()
	{
		return _div;
	}

	uint8_t Timer::TIMA()
	{
		return _tima;
	}

	uint8_t Timer::TMA()
	{
		return _tma;
	}

	uint8_t Timer::TAC()
	{
		return _tac;
	}

	void Timer::setDIV(const uint8_t &value)
	{
		_div = value;
	}

	void Timer::setTIMA(const uint8_t &value)
	{
		_tima = value;
	}

	void Timer::setTMA(const uint8_t &value)
	{
		_tma = value;
	}

	void Timer::setTAC(const uint8_t &value)
	{
		_tac = value;
	}

	void Timer::update(const uint16_t &cycles)
	{
		updateDIV(cycles);
		updateTIMA(cycles);
	}

	void Timer::updateDIV(const uint16_t &cycle)
	{
		_divIncrementRate -= cycle;
		if (_divIncrementRate <= 0)
		{
			_divIncrementRate = 255;
			_div++;
		}
	}

	void Timer::resetDIV()
	{
		_div = 0;
	}

	void Timer::updateTIMA(const uint16_t &cycle)
	{
		// If the TIMA control is enabled
		if ((_tac >> 2) & 0x1)
		{
			_timaIncrementRate -= cycle;
			if (_timaIncrementRate <= 0)
			{
				updateTAC(_tac);
				_tima++;
				if (_tima > 0xFF)
				{
					_tima = _tma;
					provider::InterruptManagerProvider::getInstance()->requestInterrupt(InterruptManager::InterruptType::Timer);
				}
			}
		}
	}

	void Timer::updateTMA(const uint16_t &value)
	{
		_tma = static_cast<uint8_t>(value);
	}

	void Timer::updateTAC(const uint8_t &value)
	{
		_tac = static_cast<uint8_t>(value);
		switch (value & 0x3)
		{
		case 0:
			_timaIncrementRate = 1024;
			break;
		case 1:
			_timaIncrementRate = 16;
			break;
		case 2:
			_timaIncrementRate = 64;
			break;
		case 3:
			_timaIncrementRate = 255;
			break;
		}
	}

	void Timer::stopTimer()
	{
		_div = 0;
	}
}