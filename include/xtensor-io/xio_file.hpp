#ifndef XTENSOR_IO_FILE_HPP
#define XTENSOR_IO_FILE_HPP

#include <stdio.h>

namespace xt
{
    class xfile
    {
    public:
        xfile(FILE* stream);
        size_t fwrite(const void* ptr, size_t size, size_t count);
        size_t fread(void* ptr, size_t size, size_t count);
        long int ftell();
        int fseek(long int offset, int origin);
        void rewind();
        int fflush();
    private:
        FILE* m_stream;
    };

    inline xfile::xfile(FILE* stream)
        : m_stream(stream)
    {
    }

    inline size_t xfile::fwrite(const void* ptr, size_t size, size_t count)
    {
        return ::fwrite(ptr, size, count, m_stream);
    }

    inline size_t xfile::fread(void* ptr, size_t size, size_t count)
    {
        return ::fread(ptr, size, count, m_stream);
    }

    inline long int xfile::ftell()
    {
        return ::ftell(m_stream);
    }

    inline int xfile::fseek(long int offset, int origin)
    {
        return ::fseek(m_stream, offset, origin);
    }

    inline void xfile::rewind()
    {
        ::rewind(m_stream);
    }

    inline int xfile::fflush()
    {
        return ::fflush(m_stream);
    }
}

#endif
