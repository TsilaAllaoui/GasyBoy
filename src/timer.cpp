#include "timer.h"

namespace gasyboy
{
	Timer::Timer(Mmu &mmu, InterruptManager &interruptManager)
		: _mmu(mmu),
		  _interruptManager(interruptManager),
		  _div(0),
		  _tima(0),
		  _tma(0),
		  _tac(0),
		  _divIncrementRate(256),
		  _timaIncrementRate(0)
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

	void Timer::update(const int &cycles)
	{
		updateDIV(cycles);
		updateTIMA(cycles);
	}

	void Timer::updateDIV(const int &cycle)
	{
		_divIncrementRate -= cycle;
		if (_divIncrementRate <= 0)
		{
			_divIncrementRate = 256;
			_div++;
		}
	}

	void Timer::resetDIV()
	{
		_div = 0;
	}

	void Timer::updateTIMA(const int &cycle)
	{
		// If the TIMA control is enabled
		if ((_tac >> 2) & 0x1)
		{
			_timaIncrementRate -= cycle;
			if (_timaIncrementRate <= 0)
			{
				_tima++;
				if (_tima > 0xFF)
				{
					_tima = _tma;
				}
			}
		}
	}

	void Timer::updateTMA(const int &value)
	{
		_tma = value;
	}

	void Timer::updateTAC(const uint8_t &value)
	{
		_tac = value;
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
			_timaIncrementRate = 256;
			break;
		}
	}

	void Timer::stopTimer()
	{
		_div = 0;
	}
}