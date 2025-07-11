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

#include "RendererState.h"

#include <Corrade/Containers/StringView.h>

#include "Magnum/PixelStorage.h"
#include "Magnum/GL/Context.h"
#include "Magnum/GL/Extensions.h"

/* The __EMSCRIPTEN_major__ etc macros used to be passed implicitly, version
   3.1.4 moved them to a version header and version 3.1.23 dropped the
   backwards compatibility. To work consistently on all versions, including the
   header only if the version macros aren't present.
   https://github.com/emscripten-core/emscripten/commit/f99af02045357d3d8b12e63793cef36dfde4530a
   https://github.com/emscripten-core/emscripten/commit/f76ddc702e4956aeedb658c49790cc352f892e4c */
#if defined(CORRADE_TARGET_EMSCRIPTEN) && !defined(__EMSCRIPTEN_major__)
#include <emscripten/version.h>
#endif

namespace Magnum { namespace GL { namespace Implementation {

using namespace Containers::Literals;

RendererState::RendererState(Context& context, ContextState& contextState, Containers::StaticArrayView<Implementation::ExtensionCount, const char*> extensions)
    #ifndef MAGNUM_TARGET_WEBGL
    : resetNotificationStrategy()
    #endif
{
    /* Depth clear value / range implementation. If the NV_depth_buffer_float
       extension is present, prefer it for both the float and double overloads
       to avoid accidents. Otherwise use the float variant if available, and
       fall back to the double variant otherwise. */
    #ifndef MAGNUM_TARGET_GLES
    if(context.isExtensionSupported<Extensions::NV::depth_buffer_float>()) {
        extensions[Extensions::NV::depth_buffer_float::Index] =
                   Extensions::NV::depth_buffer_float::string();

        clearDepthImplementation = glClearDepthdNV;
        depthRangeImplementation = glDepthRangedNV;
        clearDepthfImplementation = &Renderer::clearDepthfImplementationNV;
        depthRangefImplementation = &Renderer::depthRangefImplementationNV;
    } else
    #endif
    {
        #ifndef MAGNUM_TARGET_GLES
        clearDepthImplementation = glClearDepth;
        depthRangeImplementation = glDepthRange;
        #endif

        #ifndef MAGNUM_TARGET_GLES
        if(context.isExtensionSupported<Extensions::ARB::ES2_compatibility>())
        #endif
        {
            #ifndef MAGNUM_TARGET_GLES
            extensions[Extensions::ARB::ES2_compatibility::Index] =
                    Extensions::ARB::ES2_compatibility::string();
            #endif

            clearDepthfImplementation = glClearDepthf;
            depthRangefImplementation = glDepthRangef;
        }
        #ifndef MAGNUM_TARGET_GLES
        else {
            clearDepthfImplementation = &Renderer::clearDepthfImplementationDefault;
            depthRangefImplementation = &Renderer::depthRangefImplementationDefault;
        }
        #endif
    }

    #ifndef MAGNUM_TARGET_WEBGL
    /* Graphics reset status implementation */
    #ifndef MAGNUM_TARGET_GLES
    if(context.isExtensionSupported<Extensions::ARB::robustness>())
    #else
    if(context.isExtensionSupported<Extensions::EXT::robustness>())
    #endif
    {
        #ifndef MAGNUM_TARGET_GLES
        extensions[Extensions::ARB::robustness::Index] =
                   Extensions::ARB::robustness::string();
        graphicsResetStatusImplementation = glGetGraphicsResetStatusARB;
        #else
        extensions[Extensions::EXT::robustness::Index] =
                   Extensions::EXT::robustness::string();
        graphicsResetStatusImplementation = glGetGraphicsResetStatusEXT;
        #endif

    } else graphicsResetStatusImplementation = &Renderer::graphicsResetStatusImplementationDefault;
    #else
    static_cast<void>(context);
    static_cast<void>(extensions);
    #endif

    /* In case the extensions are not supported on ES2, row length is
       constantly 0 to avoid modifying that state */
    #if !(defined(MAGNUM_TARGET_GLES2) && defined(MAGNUM_TARGET_WEBGL))
    unpackPixelStorage.disengagedRowLength = PixelStorage::DisengagedValue;
    packPixelStorage.disengagedRowLength = PixelStorage::DisengagedValue;
    #ifdef MAGNUM_TARGET_GLES2
    if(!context.isExtensionSupported<Extensions::EXT::unpack_subimage>())
        unpackPixelStorage.disengagedRowLength = 0;
    if(!context.isExtensionSupported<Extensions::NV::pack_subimage>())
        packPixelStorage.disengagedRowLength = 0;
    #endif
    #endif

    /* Similarly, in case the compressed pixel storage isn't supported (which
       is the case on macOS), all block properties are constantly 0 to avoid
       modifying that state */
    #ifndef MAGNUM_TARGET_GLES
    unpackPixelStorage.disengagedBlockSize = PixelStorage::DisengagedValue;
    packPixelStorage.disengagedBlockSize = PixelStorage::DisengagedValue;
    if(!context.isExtensionSupported<Extensions::ARB::compressed_texture_pixel_storage>()) {
        unpackPixelStorage.disengagedBlockSize = 0;
        packPixelStorage.disengagedBlockSize = 0;
    }
    #endif

    #ifndef MAGNUM_TARGET_GLES
    if((context.detectedDriver() & Context::DetectedDriver::Mesa) &&
       (context.flags() & Context::Flag::ForwardCompatible) &&
        !context.isDriverWorkaroundDisabled("mesa-forward-compatible-line-width-range"_s))
        lineWidthRangeImplementation = &Renderer::lineWidthRangeImplementationMesaForwardCompatible;
    else
    #endif
    {
        lineWidthRangeImplementation = &Renderer::lineWidthRangeImplementationDefault;
    }

    #ifndef MAGNUM_TARGET_GLES
    minSampleShadingImplementation = glMinSampleShading;
    #elif !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
    if(context.isVersionSupported(Version::GLES320)) {
        minSampleShadingImplementation = glMinSampleShading;
    } else if(context.isExtensionSupported<Extensions::OES::sample_shading>()) {
        extensions[Extensions::OES::sample_shading::Index] =
                   Extensions::OES::sample_shading::string();

        minSampleShadingImplementation = glMinSampleShadingOES;
    } else {
        minSampleShadingImplementation = nullptr;
    }
    #endif

    #if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
    #ifdef MAGNUM_TARGET_GLES
    if(context.isVersionSupported(Version::GLES320))
    #endif
    {
        patchParameteriImplementation = glPatchParameteri;
    }
    #ifdef MAGNUM_TARGET_GLES
    else {
        /* Not checking for the extension (nor adding it to the extension list)
           as this is not any optional feature -- it can be only used when
           the extension is present, and if it's not, the pointers are null */
        patchParameteriImplementation = glPatchParameteriEXT;
    }
    #endif
    #endif

    #if !(defined(MAGNUM_TARGET_WEBGL) && defined(MAGNUM_TARGET_GLES2))
    #if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
    #ifdef MAGNUM_TARGET_GLES
    if(context.isVersionSupported(Version::GLES320))
    #endif
    {
        enableiImplementation = glEnablei;
        disableiImplementation = glDisablei;
        colorMaskiImplementation = glColorMaski;
        blendFunciImplementation = glBlendFunci;
        blendFuncSeparateiImplementation = glBlendFuncSeparatei;
        blendEquationiImplementation = glBlendEquationi;
        blendEquationSeparateiImplementation = glBlendEquationSeparatei;
    }
    #endif
    #ifdef MAGNUM_TARGET_GLES
    #if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
    else
    #endif
    {
        /* Not checking for the extension (nor adding it to the extension list)
           as this is not any optional feature -- it can be only used when
           the extension is present, and if it's not, the pointers are null */
        #ifndef MAGNUM_TARGET_WEBGL
        enableiImplementation = glEnableiEXT;
        disableiImplementation = glDisableiEXT;
        colorMaskiImplementation = glColorMaskiEXT;
        blendFunciImplementation = glBlendFunciEXT;
        blendFuncSeparateiImplementation = glBlendFuncSeparateiEXT;
        blendEquationiImplementation = glBlendEquationiEXT;
        blendEquationSeparateiImplementation = glBlendEquationSeparateiEXT;
        #else
        /* Emscripten doesn't support these yet (last checked Feb 2020) */
        enableiImplementation = nullptr;
        disableiImplementation = nullptr;
        colorMaskiImplementation = nullptr;
        blendFunciImplementation = nullptr;
        blendFuncSeparateiImplementation = nullptr;
        blendEquationiImplementation = nullptr;
        blendEquationSeparateiImplementation = nullptr;
        #endif
    }
    #endif
    #endif

    #ifdef MAGNUM_TARGET_GLES
    #ifndef MAGNUM_TARGET_WEBGL
    if(context.isExtensionSupported<Extensions::NV::polygon_mode>()) {
        extensions[Extensions::NV::polygon_mode::Index] =
                   Extensions::NV::polygon_mode::string();
        polygonModeImplementation = glPolygonModeNV;
    } else if(context.isExtensionSupported<Extensions::ANGLE::polygon_mode>()) {
        extensions[Extensions::ANGLE::polygon_mode::Index] =
                   Extensions::ANGLE::polygon_mode::string();
        polygonModeImplementation = glPolygonModeANGLE;
    } else
    #elif __EMSCRIPTEN_major__*10000 + __EMSCRIPTEN_minor__*100 + __EMSCRIPTEN_tiny__ >= 30166
    if(context.isExtensionSupported<Extensions::WEBGL::polygon_mode>()) {
        extensions[Extensions::WEBGL::polygon_mode::Index] =
                   Extensions::WEBGL::polygon_mode::string();
        polygonModeImplementation = glPolygonModeWEBGL;
    } else
    #endif
    {
        polygonModeImplementation = nullptr;
    }
    #endif

    #ifndef MAGNUM_TARGET_GLES
    /* On compatibility profile we need to explicitly enable GL_POINT_SPRITE
       in order to have gl_PointCoord working (on NVidia at least, Mesa behaves
       as if it was always enabled). On core profile this is enabled
       implicitly, thus GL_POINT_SPRITE is not even in headers and calling
       glEnable(GL_POINT_SPRITE) would cause a GL error. See
       RendererGLTest::pointCoord() for more information. */
    if(!context.isCoreProfileInternal(contextState)) {
        glEnable(0x8861 /*GL_POINT_SPRITE*/);
    }
    #else
    static_cast<void>(contextState);
    #endif
}

RendererState::PixelStorage::PixelStorage():
    alignment{4}
    #if !(defined(MAGNUM_TARGET_GLES2) && defined(MAGNUM_TARGET_WEBGL))
    , rowLength{0}
    #endif
    #ifndef MAGNUM_TARGET_GLES2
    , imageHeight{0},
    skip{0}
    #endif
    #ifndef MAGNUM_TARGET_GLES
    , compressedBlockSize{0},
    compressedBlockDataSize{0}
    #endif
    {}

void RendererState::PixelStorage::reset() {
    alignment = DisengagedValue;
    #if !(defined(MAGNUM_TARGET_GLES2) && defined(MAGNUM_TARGET_WEBGL))
    /* Resets to 0 instead of DisengagedValue in case the EXT_unpack_subimage/
       NV_pack_image ES2 extension is not supported to avoid modifying that
       state */
    rowLength = disengagedRowLength;
    #endif
    #ifndef MAGNUM_TARGET_GLES2
    imageHeight = DisengagedValue;
    skip = Vector3i{DisengagedValue};
    #endif
    #ifndef MAGNUM_TARGET_GLES
    compressedBlockSize = Vector3i{disengagedBlockSize};
    compressedBlockDataSize = disengagedBlockSize;
    #endif
}

void RendererState::applyPixelStorageInternal(const Magnum::PixelStorage& storage, const bool isUnpack) {
    PixelStorage& state = isUnpack ? unpackPixelStorage : packPixelStorage;

    /* Alignment */
    if(state.alignment == PixelStorage::DisengagedValue ||
       state.alignment != storage.alignment())
        glPixelStorei(isUnpack ? GL_UNPACK_ALIGNMENT : GL_PACK_ALIGNMENT,
            state.alignment = storage.alignment());

    /* Row length */
    #if !(defined(MAGNUM_TARGET_GLES2) && defined(MAGNUM_TARGET_WEBGL))
    if(state.rowLength == PixelStorage::DisengagedValue ||
       state.rowLength != storage.rowLength())
    {
        #ifndef MAGNUM_TARGET_GLES2
        glPixelStorei(isUnpack ? GL_UNPACK_ROW_LENGTH : GL_PACK_ROW_LENGTH,
            state.rowLength = storage.rowLength());
        #elif !defined(MAGNUM_TARGET_WEBGL)
        glPixelStorei(isUnpack ? GL_UNPACK_ROW_LENGTH_EXT : GL_PACK_ROW_LENGTH_NV,
            state.rowLength = storage.rowLength());
        #endif
    }
    #else
    CORRADE_ASSERT(!storage.rowLength(),
        "GL: non-default PixelStorage::rowLength() is not supported in WebGL 1.0", );
    #endif

    /* Image height (not on ES2, on ES3 for unpack only) */
    #ifndef MAGNUM_TARGET_GLES2
    if(state.imageHeight == PixelStorage::DisengagedValue ||
       state.imageHeight != storage.imageHeight())
    {
        #ifndef MAGNUM_TARGET_GLES
        glPixelStorei(isUnpack ? GL_UNPACK_IMAGE_HEIGHT : GL_PACK_IMAGE_HEIGHT,
            state.imageHeight = storage.imageHeight());
        #else
        if(isUnpack) glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,
            state.imageHeight = storage.imageHeight());
        else CORRADE_ASSERT(!storage.imageHeight(),
            "GL: non-default PixelStorage::imageHeight() for pack is not supported in OpenGL ES", );
        #endif
    }
    #else
    CORRADE_ASSERT(!storage.imageHeight(),
        "GL: non-default PixelStorage::imageHeight() is not supported in OpenGL ES 2", );
    #endif

    /* On ES2 done by modifying data pointer */
    #ifndef MAGNUM_TARGET_GLES2
    /* Skip pixels */
    if(state.skip.x() == PixelStorage::DisengagedValue ||
       state.skip.x() != storage.skip().x())
        glPixelStorei(isUnpack ? GL_UNPACK_SKIP_PIXELS : GL_PACK_SKIP_PIXELS,
            state.skip.x() = storage.skip().x());

    /* Skip rows */
    if(state.skip.y() == PixelStorage::DisengagedValue ||
       state.skip.y() != storage.skip().y())
        glPixelStorei(isUnpack ? GL_UNPACK_SKIP_ROWS : GL_PACK_SKIP_ROWS,
            state.skip.y() = storage.skip().y());

    /* Skip images (on ES3 for unpack only) */
    if(state.skip.z() == PixelStorage::DisengagedValue ||
       state.skip.z() != storage.skip().z())
    {
        #ifndef MAGNUM_TARGET_GLES
        glPixelStorei(isUnpack ? GL_UNPACK_SKIP_IMAGES : GL_PACK_SKIP_IMAGES,
            state.skip.z() = storage.skip().z());
        #else
        if(isUnpack) glPixelStorei(GL_UNPACK_SKIP_IMAGES,
            state.skip.z() = storage.skip().z());
        else CORRADE_ASSERT(!storage.skip().z(),
            "GL: non-default PixelStorage::skip().z() for pack is not supported in OpenGL ES", );
        #endif
    }
    #endif
}

void RendererState::applyCompressedPixelStorageInternal(const CompressedPixelStorage& storage, const Vector3i& blockSize, const Int blockDataSize, const bool isUnpack) {
    #ifdef MAGNUM_TARGET_GLES
    CORRADE_ASSERT(storage == CompressedPixelStorage{},
        "GL: non-default CompressedPixelStorage parameters are not supported in OpenGL ES or WebGL", );
    static_cast<void>(blockSize);
    static_cast<void>(blockDataSize);
    /* Reset the image height & skip parameters back to zero. While the ES spec
       seems to say that these are all ignored when uploading a compressed
       image (and so resetting them shouldn't be needed), with a WebGL 2 build
       Chrome is complaining that the pixel unpack parameters are invalid if
       they're not explicitly reset to zero before the compressed upload.
       Firefox doesn't mind. PixelStorageGLTest::compressedResetParameters()
       has a repro case. */
    applyPixelStorageInternal(Magnum::PixelStorage{}, isUnpack);
    #else
    /* The block properties should always be non-zero, either coming from an
       Image(View) constructed with a particular format or from properties for
       a format that was queried from GL */
    CORRADE_INTERNAL_ASSERT(blockSize != Vector3i{} && blockDataSize != 0);

    applyPixelStorageInternal(static_cast<const Magnum::PixelStorage&>(storage), isUnpack);

    PixelStorage& state = isUnpack ? unpackPixelStorage : packPixelStorage;

    /* If we have the default skip, row length and image height, we can keep
       the state at 0 as well, so if the state is all 0s in that case, don't
       set anything. It cannot happen that some state is 0 and some isn't, so
       it's not branched individually for each state. Also not doing
       `storage == CompressedPixelStorage{}` as the (unused) block size
       parameters could be set as well, causing the comparison to fail.

       On platforms that don't support ARB_compressed_texture_pixel_storage
       (such as macOS) this also ensures that for default storage parameters
       none of this state is being set as the default state there is always 0.
       For non-default skip etc. it *is* set, thus causing a GL error, but
       that's treated as a user error. */
    if(!(storage.skip() == Vector3i{} && storage.rowLength() == 0 && storage.imageHeight() == 0 && state.compressedBlockSize == Vector3i{} && state.compressedBlockDataSize == 0)) {
        /** @todo This could potentially also set the block size back to 0 if
            default skip etc. is used. Assuming that most uses would be with
            whole images it would mean the block sizes aren't set at all, OTOH
            if they're mixed with sub-image uploads then they get repeatedly
            set to a concrete value and then back to 0, making it worse than
            now. */

        /* Compressed block width */
        if(state.compressedBlockSize.x() == PixelStorage::DisengagedValue ||
           state.compressedBlockSize.x() != blockSize.x())
            glPixelStorei(isUnpack ? GL_UNPACK_COMPRESSED_BLOCK_WIDTH : GL_PACK_COMPRESSED_BLOCK_WIDTH,
                state.compressedBlockSize.x() = blockSize.x());

        /* Compressed block height */
        if(state.compressedBlockSize.y() == PixelStorage::DisengagedValue ||
           state.compressedBlockSize.y() != blockSize.y())
            glPixelStorei(isUnpack ? GL_UNPACK_COMPRESSED_BLOCK_HEIGHT : GL_PACK_COMPRESSED_BLOCK_HEIGHT,
                state.compressedBlockSize.y() = blockSize.y());

        /* Compressed block depth */
        if(state.compressedBlockSize.z() == PixelStorage::DisengagedValue ||
           state.compressedBlockSize.z() != blockSize.z())
            glPixelStorei(isUnpack ? GL_UNPACK_COMPRESSED_BLOCK_DEPTH : GL_PACK_COMPRESSED_BLOCK_DEPTH,
                state.compressedBlockSize.z() = blockSize.z());

        /* Compressed block size */
        if(state.compressedBlockDataSize == PixelStorage::DisengagedValue ||
           state.compressedBlockDataSize != blockDataSize)
            glPixelStorei(isUnpack ? GL_UNPACK_COMPRESSED_BLOCK_SIZE : GL_PACK_COMPRESSED_BLOCK_SIZE,
                state.compressedBlockDataSize = blockDataSize);
    }
    #endif
}

}}}
