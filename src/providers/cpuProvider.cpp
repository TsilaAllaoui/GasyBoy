#include "cpuProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Cpu> CpuProvider::_cpuInstance = nullptr;

        Cpu &CpuProvider::getInstance()
        {
            if (!_cpuInstance)
            {
                _cpuInstance = std::make_unique<Cpu>();
            }
            return *_cpuInstance;
        }

        void CpuProvider::deleteInstance()
        {
            _cpuInstance.reset(nullptr);
        }
    }
}