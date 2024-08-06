#include "logger.h"

namespace gasyboy
{
    namespace utils
    {
        std::shared_ptr<Logger> Logger::_instance = nullptr;
        std::stringstream gasyboy::utils::Logger::_log;

        std::shared_ptr<Logger> Logger::getInstance()
        {
            if (!_instance)
            {
                _instance = std::shared_ptr<Logger>(new Logger());
            }
            return _instance;
        }

        void Logger::log(const std::string &message)
        {
            _log << message << std::endl;
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
