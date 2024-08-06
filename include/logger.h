#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <sstream>
#include <string>
#include <memory>

namespace gasyboy
{
    namespace utils
    {
        class Logger
        {
            static std::stringstream _log;
            static std::shared_ptr<Logger> _instance;
            Logger() {};

        public:
            ~Logger() = default;

            // Get logger instance
            static std::shared_ptr<Logger> getInstance();

            // Add message to log
            void log(const std::string &message);

            // Clear log
            void clear();

            // Delete log instance
            static void deleteInstance();

            std::string getLogContent();
        };
    }
}

#endif