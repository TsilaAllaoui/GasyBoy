#ifndef __REGISTERS_PROVIDER_H__
#define __REGISTERS_PROVIDER_H__

#include <memory>
#include "registersProvider.h"
#include "registers.h"

namespace gasyboy
{
    namespace provider
    {
        class RegistersProvider
        {
            static std::unique_ptr<Registers> _registersInstance;

            RegistersProvider() = default;

        public:
            ~RegistersProvider() = default;

            RegistersProvider(const RegistersProvider &) = delete;
            RegistersProvider &operator=(const RegistersProvider &) = delete;

            static Registers &getInstance();

            static void deleteInstance();
        };
    }
}

#endif