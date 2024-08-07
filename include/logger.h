#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <sstream>
#include <string>
#include <memory>
#include <map>

namespace gasyboy
{
    namespace utils
    {
        class Logger
        {
            static std::stringstream _log;
            static std::stringstream _serialDebugLog;
            static std::shared_ptr<Logger> _instance;
            Logger();

        public:
            ~Logger() = default;

            enum class LogType
            {
                FUNCTIONAL,
                DEBUG,
                INFO,
                CRITICAL,
                SERIAL_DEBUG
            };

            // Get logger instance
            static std::shared_ptr<Logger> getInstance();

            // Add message to log
            void log(const LogType &type, const std::string &message);

            // Clear log
            void clear();

            // Delete log instance
            static void deleteInstance();

            // Get log content
            std::string getLogContent();

        private:
            std::map<LogType, std::string> _logTypeMap;
        };
    }
}

#endif