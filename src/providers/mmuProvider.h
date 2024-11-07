#ifndef __MMU_PROVIDER_H__
#define __MMU_PROVIDER_H__

#include <memory>
#include "mmu.h"

namespace gasyboy
{
    namespace provider
    {
        class MmuProvider
        {
        private:
            static std::unique_ptr<Mmu> _mmuInstance;

            MmuProvider() = default;

        public:
            ~MmuProvider() = default;

            MmuProvider(const MmuProvider &) = delete;
            MmuProvider &operator=(const MmuProvider &) = delete;

            static Mmu &getInstance();
            static Mmu &create(const uint8_t *bytes, const size_t &romSize);

            static void deleteInstance();
        };
    }
}

#endif