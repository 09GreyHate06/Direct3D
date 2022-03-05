#include "d3dcpch.h"
#include "D3DCException.h"

namespace d3dcore
{
    const char* D3DCException::what() const
    {
        std::ostringstream oss;
        oss << GetType() << '\n'
            << GetOriginString();

        m_whatBuffer = oss.str();
        return m_whatBuffer.c_str();
    }

    std::string D3DCException::GetOriginString() const
    {
        std::ostringstream oss;
        oss << "[FILE] " << m_file << '\n'
            << "[LINE] " << m_line;

        return oss.str();
    }
}

