#include "gbException.h"

namespace gasyboy
{
    namespace exception
    {
        GbException::GbException(const std::string &message)
            : _message(message)
        {
        }

        const char *GbException::what() const noexcept
        {
            return _message.c_str();
        }
    }
}