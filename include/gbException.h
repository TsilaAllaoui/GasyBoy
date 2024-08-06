#ifndef _GB_EXCEPTION_H_
#define _GB_EXCEPTION_H_

#include <exception>
#include <string>

namespace gasyboy
{
    namespace exception
    {
        class GbException : public std::exception
        {
            std::string _message;

        public:
            explicit GbException(const std::string &message);

            virtual const char *what() const noexcept override;
        };
    }
}

#endif