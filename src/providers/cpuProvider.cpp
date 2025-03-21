#include "cpuProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Cpu> CpuProvider::_cpuInstance = nullptr;

        std::shared_ptr<Cpu> CpuProvider::getInstance()
        {
            if (!_cpuInstance)
            {
                _cpuInstance = std::make_shared<Cpu>();
            }
            return _cpuInstance;
        }

        void CpuProvider::deleteInstance()
        {
            _cpuInstance.reset();
        }
    }
}