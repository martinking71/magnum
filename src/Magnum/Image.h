#ifndef Magnum_Image_h
#define Magnum_Image_h
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
 * @brief Class @ref Magnum::Image, @ref Magnum::CompressedImage, typedef @ref Magnum::Image1D, @ref Magnum::Image2D, @ref Magnum::Image3D, @ref Magnum::CompressedImage1D, @ref Magnum::CompressedImage2D, @ref Magnum::CompressedImage3D
 */

#include <Corrade/Containers/Array.h>

#include "Magnum/DimensionTraits.h"
#include "Magnum/ImageFlags.h"
#include "Magnum/PixelStorage.h"

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Corrade { namespace Containers {

/* Forward declaration of an utility used in pixels() to avoid forcing users to
   include the relatively large StridedArrayView header *before* the Image
   class definition. */
template<unsigned newDimensions, class U, unsigned dimensions, class T> StridedArrayView<newDimensions, U> arrayCast(const StridedArrayView<dimensions, T>& view);

}}
#endif

namespace Magnum {

/**
@brief Image

Stores multi-dimensional image data together with layout and pixel format
description. See @ref ImageView for a non-owning alternative.

This class can act as a drop-in replacement for @ref ImageView and
@ref Trade::ImageData APIs and is implicitly convertible to @ref ImageView.
Particular graphics API wrappers provide additional image classes, for example
@ref GL::BufferImage. See also @ref CompressedImage for equivalent
functionality targeted on compressed image formats.

@section Image-usage Basic usage

The image takes ownership of a passed @relativeref{Corrade,Containers::Array},
together with a @ref PixelFormat and size in pixels:

@snippet Magnum.cpp Image-usage

The constructor internally checks that the passed array is large enough. For
performance reasons it by default expects rows aligned to four bytes, which you
need to account for if using odd image sizes in combination with one-, two- or
three-component formats. The recommended way is to pad the row data to satisfy
the alignment:

@snippet Magnum.cpp Image-usage-padding

Alternatively, if padding is not possible or desirable, you can pass a
@ref PixelStorage instance with the alignment overriden to @cpp 1 @ce:

@snippet Magnum.cpp Image-usage-alignment

It's also possible to create just an image placeholder, storing only the image
properties without data or size. That is useful for example to specify desired
format of image queries in graphics APIs such as @ref GL::Texture::image():

@snippet Magnum.cpp Image-usage-query

As with @ref ImageView, this class supports extra storage parameters and
implementation-specific pixel format specification. See the @ref ImageView
documentation for more information.

@section Image-pixel-access Pixel data access

While the raw image data are available through @ref data(), for correct pixel
addressing it's required to incorporate all @ref storage() parameters such as
row alignment, row length, skip offset and such. This is very error-prone to
do by hand even with the help of @ref dataProperties().

The @ref pixels() accessor returns a multi-dimensional
@relativeref{Corrade,Containers::StridedArrayView} describing layout of the
data and providing easy access to particular rows, pixels and pixel contents.
The non-templated version returns a view that has one dimension more than the
actual image, with the last dimension being bytes in a particular pixels. The
second-to-last dimension is always pixels in a row, the one before (if the
image is at least 2D) is rows in an image, and for 3D images the very first
dimension describes image slices. Desired usage is casting to a concrete type
based on @ref format() first, either using the templated @ref pixels<T>() or
using @relativeref{Corrade,Containers::arrayCast()} and then operating on the
concretely typed array. The following example brightens the center 32x32 area
of an image:

@snippet Magnum.cpp Image-pixels

@attention Note that the correctness of the cast can't be generally checked
    apart from comparing that the last dimension size to the type size. It's
    the user responsibility to ensure the type matches given @ref format().

This operation is available also on a @ref ImageView, and non-compressed
@ref Trade::ImageData. See @relativeref{Corrade,Containers::StridedArrayView}
docs for more information about transforming, slicing and casting the view
further.
@see @ref Image1D, @ref Image2D, @ref Image3D
*/
template<UnsignedInt dimensions> class Image {
    public:
        enum: UnsignedInt {
            Dimensions = dimensions /**< Image dimension count */
        };

        /**
         * @brief Constructor
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * The @p data array is expected to be of proper size for given
         * parameters. For a 3D image, if @p flags contain
         * @ref ImageFlag3D::CubeMap, the @p size is expected to match its
         * restrictions.
         *
         * The @p format is expected to not be implementation-specific, use the
         * @ref Image(PixelStorage, PixelFormat, UnsignedInt, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * overload to explicitly pass an implementation-specific
         * @ref PixelFormat along with a pixel size, or the
         * @ref Image(PixelStorage, T, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * overload with the original implementation-specific enum type to have
         * the pixel size determined implicitly.
         */
        explicit Image(PixelStorage storage, PixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Constructor
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Equivalent to calling @ref Image(PixelStorage, PixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with default-constructed @ref PixelStorage.
         */
        explicit Image(PixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept: Image{{}, format, size, Utility::move(data), flags} {}

        /**
         * @brief Construct an image placeholder
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         *
         * Size is set to zero, data pointer to @cpp nullptr @ce and data
         * layout flags are empty. Move over a non-empty instance to make it
         * useful.
         *
         * The @p format is expected to not be implementation-specific, use the
         * @ref Image(PixelStorage, PixelFormat, UnsignedInt, UnsignedInt)
         * overload to explicitly pass an implementation-specific
         * @ref PixelFormat along with a pixel size, or the
         * @ref Image(PixelStorage, T) overload with the original
         * implementation-specific enum type to have the pixel size determined
         * implicitly.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        /*implicit*/ Image(PixelStorage storage, PixelFormat format) noexcept;

        /**
         * @brief Construct an image placeholder
         * @param format            Format of pixel data
         *
         * Equivalent to calling @ref Image(PixelStorage, PixelFormat)
         * with default-constructed @ref PixelStorage.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        /*implicit*/ Image(PixelFormat format) noexcept: Image{{}, format} {}

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param pixelSize         Size of a pixel in given format, in bytes
         * @param size              Image size, in pixels
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Unlike with @ref Image(PixelStorage, PixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>),
         * where pixel size is determined automatically using
         * @ref pixelFormatSize(), this allows you to specify an
         * implementation-specific pixel format and pixel size directly. Uses
         * @ref pixelFormatWrap() internally to wrap @p format in
         * @ref PixelFormat. The @p pixelSize is expected to be non-zero and
         * less than @cpp 256 @ce.
         *
         * The @p data array is expected to be of proper size for given
         * parameters. For a 3D image, if @p flags contain
         * @ref ImageFlag3D::CubeMap, the @p size is expected to match its
         * restrictions.
         */
        explicit Image(PixelStorage storage, UnsignedInt format, UnsignedInt formatExtra, UnsignedInt pixelSize, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /** @overload
         *
         * Equivalent to the above for @p format already wrapped with
         * @ref pixelFormatWrap().
         */
        explicit Image(PixelStorage storage, PixelFormat format, UnsignedInt formatExtra, UnsignedInt pixelSize, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param pixelSize         Size of a pixel in given format, in bytes
         *
         * Unlike with @ref Image(PixelStorage, PixelFormat), where pixel size
         * is determined automatically using @ref pixelFormatSize(), this
         * allows you to specify an implementation-specific pixel format and
         * pixel size directly. Uses @ref pixelFormatWrap() internally to wrap
         * @p format in @ref PixelFormat. The @p pixelSize is expected to be
         * non-zero and less than @cpp 256 @ce.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        explicit Image(PixelStorage storage, UnsignedInt format, UnsignedInt formatExtra, UnsignedInt pixelSize) noexcept;

        /** @overload
         *
         * Equivalent to the above for @p format already wrapped with
         * @ref pixelFormatWrap().
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        explicit Image(PixelStorage storage, PixelFormat format, UnsignedInt formatExtra, UnsignedInt pixelSize) noexcept;

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Uses ADL to find a corresponding @cpp pixelFormatSize(T, U) @ce
         * overload, then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with determined pixel size.
         */
        template<class T, class U> explicit Image(PixelStorage storage, T format, U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Uses ADL to find a corresponding @cpp pixelFormatSize(T) @ce
         * overload, then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with determined pixel size and @p formatExtra set to @cpp 0 @ce.
         */
        template<class T> explicit Image(PixelStorage storage, T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Equivalent to calling @ref Image(PixelStorage, T, U, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with default-constructed @ref PixelStorage.
         */
        template<class T, class U> explicit Image(T format, U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept: Image{{}, format, formatExtra, size, Utility::move(data), flags} {}

        /**
         * @brief Construct an image with implementation-specific pixel format
         * @param format            Format of pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Equivalent to calling @ref Image(PixelStorage, T, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with default-constructed @ref PixelStorage.
         */
        template<class T> explicit Image(T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept: Image{{}, format, size, Utility::move(data), flags} {}

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         *
         * Uses ADL to find a corresponding @cpp pixelFormatSize(T, U) @ce
         * overload, then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt)
         * with determined pixel size.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        template<class T, class U> /*implicit*/ Image(PixelStorage storage, T format, U formatExtra) noexcept;

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param format            Format of pixel data
         * @param formatExtra       Additional pixel format specifier
         *
         * Equivalent to calling @ref Image(PixelStorage, T, U) with
         * default-constructed @ref PixelStorage.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        template<class T, class U> /*implicit*/ Image(T format, U formatExtra) noexcept: Image{{}, format, formatExtra} {}

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param storage           Storage of pixel data
         * @param format            Format of pixel data
         *
         * Uses ADL to find a corresponding @cpp pixelFormatSize(T) @ce
         * overload, then calls @ref Image(PixelStorage, UnsignedInt, UnsignedInt, UnsignedInt)
         * with determined pixel size and @p formatExtra set to @cpp 0 @ce.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        template<class T> /*implicit*/ Image(PixelStorage storage, T format) noexcept;

        /**
         * @brief Construct an image placeholder with implementation-specific pixel format
         * @param format            Format of pixel data
         *
         * Equivalent to calling @ref Image(PixelStorage, T) with
         * default-constructed @ref PixelStorage.
         */
        template<class T
            #ifndef DOXYGEN_GENERATING_OUTPUT
            /* Otherwise this catches too much, resulting in weird errors */
            , typename std::enable_if<std::is_enum<T>::value || std::is_integral<T>::value, int>::type = 0
            #endif
        > /*implicit*/ Image(T format) noexcept: Image{{}, format} {}

        /** @brief Copying is not allowed */
        Image(const Image<dimensions>&) = delete;

        /** @brief Move constructor */
        Image(Image<dimensions>&& other) noexcept;

        /** @brief Copying is not allowed */
        Image<dimensions>& operator=(const Image<dimensions>&) = delete;

        /** @brief Move assignment */
        Image<dimensions>& operator=(Image<dimensions>&& other) noexcept;

        /** @brief Conversion to a view */
        /*implicit*/ operator BasicImageView<dimensions>() const;

        /**
         * @brief Conversion to a mutable view
         * @m_since{2019,10}
         */
        /* Not restricted to const&, because we might want to pass the view to
           another function in an oneliner (e.g. saving screenshot) */
        /*implicit*/ operator BasicMutableImageView<dimensions>();

        /**
         * @brief Layout flags
         * @m_since_latest
         */
        ImageFlags<dimensions> flags() const { return _flags; }

        /**
         * @brief Raw image data
         *
         * @see @ref release(), @ref pixels(), @ref pixelSize()
         */
        Containers::ArrayView<char> data() & { return _data; }

        /** @overload */
        Containers::ArrayView<const char> data() const & { return _data; }

        /**
         * @brief Raw image data from a r-value
         * @m_since{2019,10}
         *
         * Unlike @ref data(), which returns a view, this is equivalent to
         * @ref release() to avoid a dangling view when the temporary instance
         * goes out of scope.
         * @todoc stupid doxygen can't link to & overloads ffs
         */
        Containers::Array<char> data() && { return release(); }

        /** @overload
         * @m_since{2019,10}
         * @todo what to do here?!
         */
        Containers::Array<char> data() const && = delete;

        /** @brief Storage of pixel data */
        PixelStorage storage() const { return _storage; }

        /**
         * @brief Format of pixel data
         *
         * Returns either a defined value from the @ref PixelFormat enum or a
         * wrapped implementation-specific value. Use
         * @ref isPixelFormatImplementationSpecific() to distinguish the case
         * and @ref pixelFormatUnwrap() to extract an implementation-specific
         * value, if needed.
         * @see @ref formatExtra()
         */
        PixelFormat format() const { return _format; }

        /**
         * @brief Additional pixel format specifier
         *
         * Some implementations (such as OpenGL) define a pixel format using
         * two values. This field contains the second implementation-specific
         * value verbatim, if any. See @ref format() for more information.
         */
        UnsignedInt formatExtra() const { return _formatExtra; }

        /**
         * @brief Size of a pixel in bytes
         *
         * @see @ref size(), @ref pixelFormatSize()
         */
        UnsignedInt pixelSize() const { return _pixelSize; }

        /**
         * @brief Image size in pixels
         *
         * @see @ref pixelSize()
         */
        /* Unlike other getters this one is a const& so it's possible to slice
           to the sizes when all images are in an array, for example for use
           in TextureTools atlas APIs */
        const VectorTypeFor<dimensions, Int>& size() const { return _size; }

        /**
         * @brief Image data properties
         *
         * See @ref PixelStorage::dataProperties() for more information.
         */
        std::pair<VectorTypeFor<dimensions, std::size_t>, VectorTypeFor<dimensions, std::size_t>> dataProperties() const;

        /**
         * @brief Pixel data
         * @m_since{2019,10}
         *
         * Provides direct and easy-to-use access to image pixels. See
         * @ref Image-pixel-access for more information.
         */
        Containers::StridedArrayView<dimensions + 1, char> pixels();
        Containers::StridedArrayView<dimensions + 1, const char> pixels() const; /**< @overload */

        /**
         * @brief Pixel data in a concrete type
         * @m_since{2019,10}
         *
         * Compared to non-templated @ref pixels() in addition casts the pixel
         * data to a specified type. The user is responsible for choosing
         * correct type for given @ref format() --- checking it on the library
         * side is not possible for the general case. See also
         * @ref Image-pixel-access for more information.
         */
        template<class T> Containers::StridedArrayView<dimensions, T> pixels() {
            /* Deliberately not adding a StridedArrayView include, it should
               work without since this is a templated function and we declare
               arrayCast() above to satisfy two-phase lookup. */
            return Containers::arrayCast<dimensions, T>(pixels());
        }

        /**
         * @overload
         * @m_since{2019,10}
         */
        template<class T> Containers::StridedArrayView<dimensions, const T> pixels() const {
            return Containers::arrayCast<dimensions, const T>(pixels());
        }

        /**
         * @brief Release data storage
         *
         * Releases the ownership of the data array and resets @ref size() to
         * zero. The state afterwards is equivalent to moved-from state.
         * @see @ref data()
         */
        Containers::Array<char> release();

    private:
        PixelStorage _storage;
        PixelFormat _format;
        UnsignedInt _formatExtra;
        /** @todo this could be a short, saving 8 bytes for 1D and 3D images on
            64bit and 4 bytes for all dimensions on 32bit. Worth the pain? */
        UnsignedByte _pixelSize;
        /* 1 byte free */
        ImageFlags<dimensions> _flags;
        VectorTypeFor<dimensions, Int> _size;
        Containers::Array<char> _data;
};

/** @brief One-dimensional image */
typedef Image<1> Image1D;

/** @brief Two-dimensional image */
typedef Image<2> Image2D;

/** @brief Three-dimensional image */
typedef Image<3> Image3D;

/**
@brief Compressed image

Stores multi-dimensional compressed image data together with layout and
compressed block format description. See @ref CompressedImageView for a
non-owning alternative.

This class can act as a drop-in replacement for @ref CompressedImageView and
@ref Trade::ImageData APIs and is implicitly convertible to
@ref CompressedImageView. Particular graphics API wrappers provide additional
image classes, for example @ref GL::CompressedBufferImage. See also @ref Image
for equivalent functionality targeted on non-compressed image formats.

@section CompressedImage-usage Basic usage

The image takes ownership of a passed @relativeref{Corrade,Containers::Array},
together with a @ref CompressedPixelFormat and size in pixels:

@snippet Magnum.cpp CompressedImage-usage

It's also possible to create just an image placeholder, storing only the image
properties without data or size. That is useful for example to specify desired
format of image queries in graphics APIs:

@snippet Magnum.cpp CompressedImage-usage-query

As with @ref CompressedImageView, this class supports extra storage parameters
and implementation-specific compressed pixel format specification. See its
documentation for more information.
@see @ref CompressedImage1D, @ref CompressedImage2D, @ref CompressedImage3D
*/
template<UnsignedInt dimensions> class CompressedImage {
    public:
        enum: UnsignedInt {
            Dimensions = dimensions /**< Image dimension count */
        };

        /**
         * @brief Constructor
         * @param storage           Storage of compressed pixel data
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * The @p data array is expected to be of proper size for given
         * parameters. For a 3D image, if @p flags contain
         * @ref ImageFlag3D::CubeMap, the @p size is expected to match its
         * restrictions.
         *
         * The @p format is expected to not be implementation-specific, use the
         * @ref CompressedImage(CompressedPixelStorage, CompressedPixelFormat, const Vector3i&, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * overload to explicitly pass pass an implementation-specific
         * @ref CompressedPixelFormat along with its block properties, or the
         * @ref CompressedImage(CompressedPixelStorage, T, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * overload with the original implementation-specific enum type to have
         * the pixel size determined implicitly.
         *
         * @ref CompressedPixelStorage::compressedBlockSize() and
         * @relativeref{CompressedPixelStorage,compressedBlockDataSize()} in
         * @p storage are expected to be either both zero or exactly matching
         * properties of given @p format.
         */
        explicit CompressedImage(CompressedPixelStorage storage, CompressedPixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Constructor
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Equivalent to calling @ref CompressedImage(CompressedPixelStorage, CompressedPixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with default-constructed @ref CompressedPixelStorage.
         */
        explicit CompressedImage(CompressedPixelFormat format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept: CompressedImage{{}, format, size, Utility::move(data), flags} {}

        /**
         * @brief Construct a compressed image with an implementation-specific pixel format
         * @param storage           Storage of compressed pixel data
         * @param format            Format of compressed pixel data
         * @param blockSize         Size of a compressed block in given format,
         *      in pixels
         * @param blockDataSize     Size of a compressed block in given format,
         *      in bytes
         * @param size              Image size, in pixels
         * @param data              Image data
         * @param flags             Image layout flags
         * @m_since_latest
         *
         * Unlike with @ref CompressedImage(CompressedPixelStorage, CompressedPixelFormat, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>),
         * where block size is determined automatically
         * @ref compressedPixelFormatBlockSize() and
         * @ref compressedPixelFormatBlockDataSize(), this allows you to
         * specify an implementation-specific pixel format and block properties
         * directly. Uses @ref compressedPixelFormatWrap() internally to wrap
         * @p format in @ref CompressedPixelFormat. The @p blockSize and
         * @p blockDataSize is expected to be greater than @cpp 0 @ce and less
         * than @cpp 256 @ce. Note that the blocks can be 3D even for 2D images
         * and 2D or 3D even for 1D images, in which case only the first slice
         * in the extra dimensions is used.
         *
         * @ref CompressedPixelStorage::compressedBlockSize() and
         * @relativeref{CompressedPixelStorage,compressedBlockDataSize()} in
         * @p storage are expected to be either both zero or exactly matching
         * @p blockSize and @p blockDataSize.
         *
         * The @p data array is expected to be of proper size for given
         * parameters. For a 3D image, if @p flags contain
         * @ref ImageFlag3D::CubeMap, the @p size is expected to match its
         * restrictions.
         */
        explicit CompressedImage(CompressedPixelStorage storage, UnsignedInt format, const Vector3i& blockSize, UnsignedInt blockDataSize, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /** @overload
         * @m_since_latest
         *
         * Equivalent to the above for @p format already wrapped with
         * @ref compressedPixelFormatWrap().
         */
        explicit CompressedImage(CompressedPixelStorage storage, CompressedPixelFormat format, const Vector3i& blockSize, UnsignedInt blockDataSize, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Construct a compressed image with implementation-specific format
         * @param storage           Storage of compressed pixel data
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Uses ADL to find a corresponding @cpp compressedPixelFormatBlockSize(T) @ce
         * and @cpp compressedPixelFormatBlockDataSize(T) @ce overloads, then
         * calls @ref CompressedImage(CompressedPixelStorage, UnsignedInt, const Vector3i&, UnsignedInt, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with determined block size properties.
         */
        template<class T> explicit CompressedImage(CompressedPixelStorage storage, T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept;

        /**
         * @brief Construct a compressed image with implementation-specific format
         * @param format            Format of compressed pixel data
         * @param size              Image size
         * @param data              Image data
         * @param flags             Image layout flags
         *
         * Equivalent to calling @ref CompressedImage(CompressedPixelStorage, T, const VectorTypeFor<dimensions, Int>&, Containers::Array<char>&&, ImageFlags<dimensions>)
         * with default-constructed @ref CompressedPixelStorage.
         */
        template<class T> explicit CompressedImage(T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, ImageFlags<dimensions> flags = {}) noexcept: CompressedImage{{}, format, size, Utility::move(data), flags} {}

        /**
         * @brief Construct an image placeholder
         * @param storage           Storage of compressed pixel data
         *
         * Format and block properties are undefined, size is zero, data is
         * @cpp nullptr @ce and data layout flags are empty. Move over a
         * non-empty instance to make it useful.
         *
         * @ref CompressedPixelStorage::compressedBlockSize() and
         * @relativeref{CompressedPixelStorage,compressedBlockDataSize()} in
         * @p storage are expected to be both zero.
         */
        /* No ImageFlags parameter here as this constructor is mainly used to
           query GL textures, and there the flags are forcibly reset */
        /*implicit*/ CompressedImage(CompressedPixelStorage storage) noexcept;

        /**
         * @brief Construct an image placeholder
         *
         * Equivalent to calling @ref CompressedImage(CompressedPixelStorage)
         * with default-constructed @ref CompressedPixelStorage.
         */
        /*implicit*/ CompressedImage() noexcept: CompressedImage{{}} {}

        /** @brief Copying is not allowed */
        CompressedImage(const CompressedImage<dimensions>&) = delete;

        /** @brief Move constructor */
        CompressedImage(CompressedImage<dimensions>&& other) noexcept;

        /** @brief Copying is not allowed */
        CompressedImage<dimensions>& operator=(const CompressedImage<dimensions>&) = delete;

        /** @brief Move assignment */
        CompressedImage<dimensions>& operator=(CompressedImage<dimensions>&& other) noexcept;

        /** @brief Conversion to a view */
        /*implicit*/ operator BasicCompressedImageView<dimensions>() const;

        /**
         * @brief Conversion to a mutable view
         * @m_since{2019,10}
         */
        /* Not restricted to const&, because we might want to pass the view to
           another function in an oneliner (e.g. saving screenshot) */
        /*implicit*/ operator BasicMutableCompressedImageView<dimensions>();

        /**
         * @brief Layout flags
         * @m_since_latest
         */
        ImageFlags<dimensions> flags() const { return _flags; }

        /**
         * @brief Raw image data
         *
         * @see @ref release(), @ref size(), @ref blockSize(),
         *      @ref blockDataSize()
         */
        Containers::ArrayView<char> data() & { return _data; }

        /** @overload */
        Containers::ArrayView<const char> data() const & { return _data; }

        /**
         * @brief Raw image data from a r-value
         * @m_since{2019,10}
         *
         * Unlike @ref data(), which returns a view, this is equivalent to
         * @ref release() to avoid a dangling view when the temporary instance
         * goes out of scope.
         * @todoc stupid doxygen can't link to & overloads ffs
         */
        Containers::Array<char> data() && { return release(); }

        /** @overload
         * @m_since{2019,10}
         * @todo what to do here?!
         */
        Containers::Array<char> data() const && = delete;

        /** @brief Storage of compressed pixel data */
        CompressedPixelStorage storage() const { return _storage; }

        /**
         * @brief Format of compressed pixel data
         *
         * Returns either a defined value from the @ref CompressedPixelFormat
         * enum or a wrapped implementation-specific value. Use
         * @ref isCompressedPixelFormatImplementationSpecific() to distinguish
         * the case and @ref compressedPixelFormatUnwrap() to extract an
         * implementation-specific value, if needed.
         */
        CompressedPixelFormat format() const { return _format; }

        /**
         * @brief Size of a compressed block in pixels
         * @m_since_latest
         *
         * Note that the blocks can be 3D even for 2D images and 2D or 3D even
         * for 1D images, in which case only the first slice in the extra
         * dimensions is used.
         * @see @ref blockDataSize(), @ref size(),
         *      @ref compressedPixelFormatBlockSize()
         */
        Vector3i blockSize() const { return Vector3i{_blockSize}; }

        /**
         * @brief Size of a compressed block in bytes
         * @m_since_latest
         *
         * @see @ref blockSize(), @ref size(),
         *      @ref compressedPixelFormatBlockDataSize()
         */
        UnsignedInt blockDataSize() const { return _blockDataSize; }

        /**
         * @brief Image size in pixels
         *
         * If the size isn't divisible by @ref blockSize(), the edge blocks are
         * still present in full but used only partially.
         * @see @ref blockDataSize()
         */
        /* Unlike other getters this one is a const& so it's possible to slice
           to the sizes when all images are in an array, for example for use
           in TextureTools atlas APIs */
        const VectorTypeFor<dimensions, Int>& size() const { return _size; }

        /**
         * @brief Compressed image data properties
         *
         * See @ref CompressedPixelStorage::dataProperties() for more
         * information.
         */
        std::pair<VectorTypeFor<dimensions, std::size_t>, VectorTypeFor<dimensions, std::size_t>> dataProperties() const;

        /**
         * @brief Release data storage
         *
         * Releases the ownership of the data array and resets @ref size() to
         * zero. The state afterwards is equivalent to moved-from state.
         * @see @ref data()
         */
        Containers::Array<char> release();

    private:
        CompressedPixelStorage _storage;
        CompressedPixelFormat _format;
        ImageFlags<dimensions> _flags;
        /* Largest blocks are 12x12 in ASTC and at most 32 bytes, so an 8-bit
           type should be more than enough. As even 1D images can have 3D
           blocks, the member isn't dependent on dimension count. */
        Vector3ub _blockSize;
        UnsignedByte _blockDataSize;
        VectorTypeFor<dimensions, Int> _size;
        Containers::Array<char> _data;
};

/** @brief One-dimensional compressed image */
typedef CompressedImage<1> CompressedImage1D;

/** @brief Two-dimensional compressed image */
typedef CompressedImage<2> CompressedImage2D;

/** @brief Three-dimensional compressed image */
typedef CompressedImage<3> CompressedImage3D;

template<UnsignedInt dimensions> template<class T, class U> inline Image<dimensions>::Image(const PixelStorage storage, const T format, const U formatExtra, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, const ImageFlags<dimensions> flags) noexcept: Image{storage, UnsignedInt(format), UnsignedInt(formatExtra), pixelFormatSize(format, formatExtra), size, Utility::move(data), flags} {
    static_assert(sizeof(T) <= 4 && sizeof(U) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T> inline  Image<dimensions>::Image(const PixelStorage storage, const T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, const ImageFlags<dimensions> flags) noexcept: Image{storage, UnsignedInt(format), {}, pixelFormatSize(format), size, Utility::move(data), flags} {
    static_assert(sizeof(T) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T, class U> inline Image<dimensions>::Image(const PixelStorage storage, const T format, const U formatExtra) noexcept: Image{storage, UnsignedInt(format), UnsignedInt(formatExtra), pixelFormatSize(format, formatExtra)} {
    static_assert(sizeof(T) <= 4 && sizeof(U) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T> inline Image<dimensions>::Image(const PixelStorage storage, const T format) noexcept: Image{storage, UnsignedInt(format), {}, pixelFormatSize(format)} {
    static_assert(sizeof(T) <= 4,
        "format types larger than 32bits are not supported");
}

template<UnsignedInt dimensions> template<class T> inline CompressedImage<dimensions>::CompressedImage(const CompressedPixelStorage storage, const T format, const VectorTypeFor<dimensions, Int>& size, Containers::Array<char>&& data, const ImageFlags<dimensions> flags) noexcept: CompressedImage{storage, UnsignedInt(format), compressedPixelFormatBlockSize(format), compressedPixelFormatBlockDataSize(format), size, Utility::move(data), flags} {
    static_assert(sizeof(T) <= 4,
        "format types larger than 32bits are not supported");
}

}

#endif
