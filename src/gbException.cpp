#include "gbException.h"

namespace gasyboy
{
    namespace exception
    {
        explicit GbException::GbException(const std::string &message)
            : _message(message)
        {
        }

        virtual const char *what() const noexcept
        {
            return _message.c_str();
        }
    }
}