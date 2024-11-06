#ifndef __PPU_PROVIDER_H__
#define __PPU_PROVIDER_H__

#include <memory>
#include "ppu.h"

namespace gasyboy
{
    namespace provider
    {
        class PpuProvider
        {
        private:
            static std::unique_ptr<Ppu> _ppuInstance;

            PpuProvider() = default;

        public:
            ~PpuProvider() = default;

            PpuProvider(const PpuProvider &) = delete;
            PpuProvider &operator=(const PpuProvider &) = delete;

            static Ppu &getInstance();

            static void deleteInstance();
        };
    }
}

#endif