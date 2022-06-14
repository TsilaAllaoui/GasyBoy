#include "eventListener.h"

eventListener::eventListener(Gpu* _gpu, Mmu* _mmu)
{
	gpu = _gpu;
	mmu = _mmu;
}


eventListener::~eventListener()
{
}

void eventListener::listen()
{
	if (notifiers.size() != 0)
	{
		for (int i=0; i<notifiers.size(); i++)
		{
			if (notifiers[i].isEmitting)
				react(i);
		}
	}
}

void eventListener::react(int index)
{
	signal s = notifiers[index]._signal;
	if (s.sender == "mmu" and s.receiver == "gpu")
	{
		//gpu->showTileData(s.value);
	}
}

eventNotifier::eventNotifier()
{
	_signal.sender = "";
	_signal.receiver = "";
	_signal.value = 0;
	isEmitting = false;
}

eventNotifier::~eventNotifier()
{

}

void eventNotifier::emit(uint8_t _value, string _sender, string _receiver)
{
	_signal = { _sender, _receiver, _value };
	isEmitting = true;
}

void eventNotifier::unset()
{
	isEmitting = false;
}
