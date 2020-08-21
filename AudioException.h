#pragma once
#include <exception>
#include <string>

namespace Audio
{
    class AudioException : public std::exception
    {
    public:

        AudioException(const std::string& what)
            : m_what(what)
        {

        };

        const char* what() const override
        {
            return m_what.data();
        }

    private:
        std::string m_what;
    };
}