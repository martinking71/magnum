#ifndef Magnum_Trade_ArrayAllocator_h
#define Magnum_Trade_ArrayAllocator_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024, 2025
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class @ref Magnum::Trade::ArrayAllocator
 * @m_since{2020,06}
 */

#include <Corrade/Containers/GrowableArray.h>

#include "Magnum/Magnum.h"
#include "Magnum/Trade/visibility.h"

namespace Magnum { namespace Trade {

/**
@brief Growable array allocator to be used in importer plugins
@m_since{2020,06}

Compared to @relativeref{Corrade,Containers::ArrayMallocAllocator} ensures that
the @relativeref{Corrade,Containers::Array} deleter function pointer is defined
in the @ref Trade library and not in the plugin binary itself, avoiding
dangling function pointer call when the data array is destructed after the
plugin has been unloaded. Other than that the behavior is identical.
*/
template<class T> struct ArrayAllocator: Containers::ArrayMallocAllocator<T> {};

#ifndef DOXYGEN_GENERATING_OUTPUT
template<> struct ArrayAllocator<char>: Containers::ArrayMallocAllocator<char> {
    MAGNUM_TRADE_EXPORT static void deleter(char* data, std::size_t size);
};
#endif

}}

#endif
