#ifndef XTENSOR_IO_VSILFILE_HPP
#define XTENSOR_IO_VSILFILE_HPP

#include <cpl_vsi.h>

namespace xt
{
    class xvsilfile
    {
    public:
        xvsilfile(VSILFILE* stream);
        size_t fwrite(const void* ptr, size_t size, size_t count);
        size_t fread(void* ptr, size_t size, size_t count);
        long int ftell();
        int fseek(long int offset, int origin);
        void rewind();
        int fflush();
    private:
        VSILFILE* m_stream;
    };

    inline xvsilfile::xvsilfile(VSILFILE* stream)
        : m_stream(stream)
    {
    }

    inline size_t xvsilfile::fwrite(const void* ptr, size_t size, size_t count)
    {
        return VSIFWriteL(ptr, size, count, m_stream);
    }

    inline size_t xvsilfile::fread(void* ptr, size_t size, size_t count)
    {
        return VSIFReadL(ptr, size, count, m_stream);
    }

    inline long int xvsilfile::ftell()
    {
        return VSIFTellL(m_stream);
    }

    inline int xvsilfile::fseek(long int offset, int origin)
    {
        return VSIFSeekL(m_stream, offset, origin);
    }

    inline void xvsilfile::rewind()
    {
        VSIRewindL(m_stream);
    }

    inline int xvsilfile::fflush()
    {
        return VSIFFlushL(m_stream);
    }
}

#endif
