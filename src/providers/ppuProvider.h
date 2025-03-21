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
            static std::shared_ptr<Ppu> _ppuInstance;

            PpuProvider() = default;

        public:
            ~PpuProvider() = default;

            PpuProvider(const PpuProvider &) = delete;
            PpuProvider &operator=(const PpuProvider &) = delete;

            static std::shared_ptr<Ppu> getInstance();

            static void deleteInstance();
        };
    }
}

#endif