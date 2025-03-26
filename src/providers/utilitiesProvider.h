#ifndef __UTILITIES_PROVIDER_H__
#define __UTILITIES_PROVIDER_H__

#include <string>
#include <memory>

namespace gasyboy
{

    struct Utilities
    {
        bool executeBios;
        std::string romFilePath;
        std::string newRomFilePath;
        bool debugMode;
        bool wasReset;
        bool wasRefreshed;
    };

    namespace provider
    {
        class UtilitiesProvider
        {
        private:
            static std::shared_ptr<Utilities> _utilitiesInstance;

            UtilitiesProvider() = default;

        public:
            ~UtilitiesProvider() = default;

            UtilitiesProvider(const UtilitiesProvider &) = delete;
            UtilitiesProvider &operator=(const UtilitiesProvider &) = delete;

            static std::shared_ptr<Utilities> getInstance();

            static void deleteInstance();
        };
    }
}

#endif