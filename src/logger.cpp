#include <iostream>
#include "logger.h"

namespace gasyboy
{
    namespace utils
    {
        std::shared_ptr<Logger> Logger::_instance = nullptr;
        std::stringstream gasyboy::utils::Logger::_log;
        std::stringstream gasyboy::utils::Logger::_serialDebugLog;

        Logger::Logger()
        {
            clear();
            _logTypeMap = {{LogType::CRITICAL, "[CRITICAL] - "},
                           {LogType::DEBUG, "[DEBUG] - "},
                           {LogType::FUNCTIONAL, "[FUNCTIONAL] - "},
                           {LogType::INFO, "[INFO] - "}};
        }

        std::shared_ptr<Logger> Logger::getInstance()
        {
            if (!_instance)
            {
                _instance = std::shared_ptr<Logger>(new Logger());
            }
            return _instance;
        }

        void Logger::log(const LogType &type, const std::string &message)
        {
            std::stringstream currentLog;
            currentLog << _logTypeMap[type] << message << std::endl;
            type == LogType::SERIAL_DEBUG ? _serialDebugLog : _log << currentLog.str();
            std::cout << currentLog.str();
        }

        void Logger::clear()
        {
            _log.str("");
            _log.clear();
        }

        void Logger::deleteInstance()
        {
            _instance.reset();
        }

        std::string Logger::getLogContent()
        {
            return _log.str();
        }
    }
}
