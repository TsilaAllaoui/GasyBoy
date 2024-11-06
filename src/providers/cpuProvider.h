#ifndef __CPU_PROVIDER_H__
#define __CPU_PROVIDER_H__

#include <memory>
#include "cpu.h"

namespace gasyboy
{
    namespace provider
    {
        class CpuProvider
        {
            static std::unique_ptr<Cpu> _cpuInstance;

            CpuProvider() = default;

        public:
            ~CpuProvider() = default;

            CpuProvider(const CpuProvider &) = delete;
            CpuProvider &operator=(const CpuProvider &) = delete;

            static Cpu &getInstance();

            static void deleteInstance();
        };
    }
}

#endif