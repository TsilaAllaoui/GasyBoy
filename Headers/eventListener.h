#pragma once

#include <string>
#include <vector>

#include "gpu.h"
#include "mmu.h"

using namespace std;

struct Signal
{
	string sender, receiver;
	uint8_t value;
};


class eventNotifier
{
	public:
		Signal _signal;
		bool isEmitting;
		eventNotifier();
		~eventNotifier();
		void emit(uint8_t _value, string _sender, string _receiver);
		void unset();
		
};


class eventListener
{
	public:
		Mmu* mmu;
		Gpu* gpu;

		vector<eventNotifier> notifiers;
		eventListener(Gpu *_gpu, Mmu *_mmu);
		~eventListener();
		void listen();
		void react(int index);
};

