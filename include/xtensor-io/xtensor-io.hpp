/***************************************************************************
* Copyright (c) Wolf Vollprecht, Sylvain Corlay and Johan Mabille          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_IO_XTENSORIO_HPP
#define XTENSOR_IO_XTENSORIO_HPP

namespace xt
{
    enum class file_mode
    {
        create,
        overwrite,
        append,
        read
    };

    enum class dump_mode
    {
        create,
        overwrite
    };
}

#endif
