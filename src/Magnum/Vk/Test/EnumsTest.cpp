/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021 Vladimír Vondruš <mosra@centrum.cz>

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

#include <sstream>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/DebugStl.h>

#include "Magnum/Mesh.h"
#include "Magnum/PixelFormat.h"
#include "Magnum/Sampler.h"
#include "Magnum/VertexFormat.h"
#include "Magnum/Vk/Enums.h"

namespace Magnum { namespace Vk { namespace Test { namespace {

struct EnumsTest: TestSuite::Tester {
    explicit EnumsTest();

    void mapVkPrimitiveTopology();
    void mapVkPrimitiveTopologyImplementationSpecific();
    void mapVkPrimitiveTopologyUnsupported();
    void mapVkPrimitiveTopologyInvalid();

    void mapVkIndexType();
    void mapVkIndexTypeUnsupported();
    void mapVkIndexTypeInvalid();

    void mapVkFilter();
    void mapVkFilterInvalid();

    void mapVkSamplerMipmapMode();
    void mapVkSamplerMipmapModeInvalid();

    void mapVkSamplerAddressMode();
    void mapVkSamplerAddressModeArray();
    void mapVkSamplerAddressModeUnsupported();
    void mapVkSamplerAddressModeInvalid();
};

EnumsTest::EnumsTest() {
    addTests({&EnumsTest::mapVkPrimitiveTopology,
              &EnumsTest::mapVkPrimitiveTopologyImplementationSpecific,
              &EnumsTest::mapVkPrimitiveTopologyUnsupported,
              &EnumsTest::mapVkPrimitiveTopologyInvalid,

              &EnumsTest::mapVkIndexType,
              &EnumsTest::mapVkIndexTypeUnsupported,
              &EnumsTest::mapVkIndexTypeInvalid,

              &EnumsTest::mapVkFilter,
              &EnumsTest::mapVkFilterInvalid,

              &EnumsTest::mapVkSamplerMipmapMode,
              &EnumsTest::mapVkSamplerMipmapModeInvalid,

              &EnumsTest::mapVkSamplerAddressMode,
              &EnumsTest::mapVkSamplerAddressModeArray,
              &EnumsTest::mapVkSamplerAddressModeUnsupported,
              &EnumsTest::mapVkSamplerAddressModeInvalid});
}

void EnumsTest::mapVkPrimitiveTopology() {
    CORRADE_VERIFY(hasVkPrimitiveTopology(Magnum::MeshPrimitive::Points));
    CORRADE_COMPARE(vkPrimitiveTopology(Magnum::MeshPrimitive::Points), VK_PRIMITIVE_TOPOLOGY_POINT_LIST);

    CORRADE_VERIFY(hasVkPrimitiveTopology(Magnum::MeshPrimitive::Lines));
    CORRADE_COMPARE(vkPrimitiveTopology(Magnum::MeshPrimitive::Lines), VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

    CORRADE_VERIFY(hasVkPrimitiveTopology(Magnum::MeshPrimitive::LineStrip));
    CORRADE_COMPARE(vkPrimitiveTopology(Magnum::MeshPrimitive::LineStrip), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);

    CORRADE_VERIFY(hasVkPrimitiveTopology(Magnum::MeshPrimitive::Triangles));
    CORRADE_COMPARE(vkPrimitiveTopology(Magnum::MeshPrimitive::Triangles), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    CORRADE_VERIFY(hasVkPrimitiveTopology(Magnum::MeshPrimitive::TriangleStrip));
    CORRADE_COMPARE(vkPrimitiveTopology(Magnum::MeshPrimitive::TriangleStrip), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

    CORRADE_VERIFY(hasVkPrimitiveTopology(Magnum::MeshPrimitive::TriangleFan));
    CORRADE_COMPARE(vkPrimitiveTopology(Magnum::MeshPrimitive::TriangleFan), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);

    /* Ensure all generic primitives are handled. This goes through the first
       16 bits, which should be enough. Going through 32 bits takes 8 seconds,
       too much. */
    for(UnsignedInt i = 1; i <= 0xffff; ++i) {
        const auto primitive = Magnum::MeshPrimitive(i);
        #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic error "-Wswitch"
        #endif
        switch(primitive) {
            #define _c(primitive) \
                case Magnum::MeshPrimitive::primitive: \
                    if(hasVkPrimitiveTopology(Magnum::MeshPrimitive::primitive))  \
                        CORRADE_VERIFY(UnsignedInt(vkPrimitiveTopology(Magnum::MeshPrimitive::primitive)) >= 0); \
                    break;
            #include "Magnum/Implementation/meshPrimitiveMapping.hpp"
            #undef _c
        }
        #ifdef __GNUC__
        #pragma GCC diagnostic pop
        #endif
    }
}

void EnumsTest::mapVkPrimitiveTopologyImplementationSpecific() {
    CORRADE_VERIFY(hasVkPrimitiveTopology(meshPrimitiveWrap(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY)));
    CORRADE_COMPARE(vkPrimitiveTopology(meshPrimitiveWrap(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY)),
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY);
}

void EnumsTest::mapVkPrimitiveTopologyUnsupported() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    CORRADE_VERIFY(!hasVkPrimitiveTopology(Magnum::MeshPrimitive::LineLoop));

    std::ostringstream out;
    {
        Error redirectError{&out};
        vkPrimitiveTopology(Magnum::MeshPrimitive::LineLoop);
    }
    CORRADE_COMPARE(out.str(),
        "Vk::vkPrimitiveTopology(): unsupported primitive MeshPrimitive::LineLoop\n");
}

void EnumsTest::mapVkPrimitiveTopologyInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    hasVkPrimitiveTopology(Magnum::MeshPrimitive{});
    hasVkPrimitiveTopology(Magnum::MeshPrimitive(0x12));
    vkPrimitiveTopology(Magnum::MeshPrimitive{});
    vkPrimitiveTopology(Magnum::MeshPrimitive(0x12));
    CORRADE_COMPARE(out.str(),
        "Vk::hasVkPrimitiveTopology(): invalid primitive MeshPrimitive(0x0)\n"
        "Vk::hasVkPrimitiveTopology(): invalid primitive MeshPrimitive(0x12)\n"
        "Vk::vkPrimitiveTopology(): invalid primitive MeshPrimitive(0x0)\n"
        "Vk::vkPrimitiveTopology(): invalid primitive MeshPrimitive(0x12)\n");
}

void EnumsTest::mapVkIndexType() {
    CORRADE_VERIFY(hasVkIndexType(Magnum::MeshIndexType::UnsignedShort));
    CORRADE_COMPARE(vkIndexType(Magnum::MeshIndexType::UnsignedShort), VK_INDEX_TYPE_UINT16);

    CORRADE_VERIFY(hasVkIndexType(Magnum::MeshIndexType::UnsignedInt));
    CORRADE_COMPARE(vkIndexType(Magnum::MeshIndexType::UnsignedInt), VK_INDEX_TYPE_UINT32);

    /* Ensure all generic index types are handled. This goes through the first
       16 bits, which should be enough. Going through 32 bits takes 8 seconds,
       too much. */
    for(UnsignedInt i = 1; i <= 0xffff; ++i) {
        const auto type = Magnum::MeshIndexType(i);
        #ifdef __GNUC__
        #pragma GCC diagnostic push
        #pragma GCC diagnostic error "-Wswitch"
        #endif
        switch(type) {
            #define _c(type) \
                case Magnum::MeshIndexType::type: \
                    CORRADE_VERIFY(UnsignedInt(vkIndexType(Magnum::MeshIndexType::type)) >= 0); \
                    break;
            #include "Magnum/Implementation/meshIndexTypeMapping.hpp"
            #undef _c
        }
        #ifdef __GNUC__
        #pragma GCC diagnostic pop
        #endif
    }
}

void EnumsTest::mapVkIndexTypeUnsupported() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    #if 1
    CORRADE_SKIP("All index formats are supported.");
    #else
    CORRADE_VERIFY(!hasVkIndexType(Magnum::MeshIndexType::UnsignedByte));
    std::ostringstream out;
    {
        Error redirectError{&out};
        vkIndexType(Magnum::MeshIndexType::UnsignedByte);
    }
    CORRADE_COMPARE(out.str(),
        "Vk::vkIndexType(): unsupported type MeshIndexType::UnsignedByte\n");
    #endif
}

void EnumsTest::mapVkIndexTypeInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    hasVkIndexType(Magnum::MeshIndexType(0x0));
    hasVkIndexType(Magnum::MeshIndexType(0x12));
    vkIndexType(Magnum::MeshIndexType(0x0));
    vkIndexType(Magnum::MeshIndexType(0x12));
    CORRADE_COMPARE(out.str(),
        "Vk::hasVkIndexType(): invalid type MeshIndexType(0x0)\n"
        "Vk::hasVkIndexType(): invalid type MeshIndexType(0x12)\n"
        "Vk::vkIndexType(): invalid type MeshIndexType(0x0)\n"
        "Vk::vkIndexType(): invalid type MeshIndexType(0x12)\n");
}

void EnumsTest::mapVkFilter() {
    CORRADE_COMPARE(vkFilter(SamplerFilter::Nearest), VK_FILTER_NEAREST);
    CORRADE_COMPARE(vkFilter(SamplerFilter::Linear), VK_FILTER_LINEAR);
}

void EnumsTest::mapVkFilterInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    vkFilter(Magnum::SamplerFilter(0x123));
    CORRADE_COMPARE(out.str(),
        "Vk::vkFilter(): invalid filter SamplerFilter(0x123)\n");
}

void EnumsTest::mapVkSamplerMipmapMode() {
    CORRADE_COMPARE(vkSamplerMipmapMode(SamplerMipmap::Base), VK_SAMPLER_MIPMAP_MODE_NEAREST); /* deliberate */
    CORRADE_COMPARE(vkSamplerMipmapMode(SamplerMipmap::Nearest), VK_SAMPLER_MIPMAP_MODE_NEAREST);
    CORRADE_COMPARE(vkSamplerMipmapMode(SamplerMipmap::Linear), VK_SAMPLER_MIPMAP_MODE_LINEAR);
}

void EnumsTest::mapVkSamplerMipmapModeInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    vkSamplerMipmapMode(Magnum::SamplerMipmap(0x123));
    CORRADE_COMPARE(out.str(),
        "Vk::vkSamplerMipmapMode(): invalid mode SamplerMipmap(0x123)\n");
}

void EnumsTest::mapVkSamplerAddressMode() {
    CORRADE_VERIFY(hasVkSamplerAddressMode(SamplerWrapping::Repeat));
    CORRADE_COMPARE(vkSamplerAddressMode(SamplerWrapping::Repeat), VK_SAMPLER_ADDRESS_MODE_REPEAT);

    CORRADE_VERIFY(hasVkSamplerAddressMode(SamplerWrapping::MirroredRepeat));
    CORRADE_COMPARE(vkSamplerAddressMode(SamplerWrapping::MirroredRepeat), VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);

    CORRADE_VERIFY(hasVkSamplerAddressMode(SamplerWrapping::ClampToEdge));
    CORRADE_COMPARE(vkSamplerAddressMode(SamplerWrapping::ClampToEdge), VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

    CORRADE_VERIFY(hasVkSamplerAddressMode(SamplerWrapping::ClampToBorder));
    CORRADE_COMPARE(vkSamplerAddressMode(SamplerWrapping::ClampToBorder), VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
}

void EnumsTest::mapVkSamplerAddressModeArray() {
    CORRADE_COMPARE(vkSamplerAddressMode<2>({SamplerWrapping::Repeat, SamplerWrapping::ClampToBorder}), (Array2D<VkSamplerAddressMode>{VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER}));
}

void EnumsTest::mapVkSamplerAddressModeUnsupported() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    #if 1
    CORRADE_SKIP("All sampler address modes are supported.");
    #else
    CORRADE_VERIFY(!hasVkSamplerAddressMode(Magnum::SamplerWrapping::MirrorClampToEdge));
    std::ostringstream out;
    Error redirectError{&out};
    vkSamplerAddressMode(Magnum::SamplerWrapping::MirrorClampToEdge);
    CORRADE_COMPARE(out.str(),
        "Vk::vkSamplerAddressMode(): unsupported wrapping SamplerWrapping::MirrorClampToEdge\n");
    #endif
}

void EnumsTest::mapVkSamplerAddressModeInvalid() {
    #ifdef CORRADE_NO_ASSERT
    CORRADE_SKIP("CORRADE_NO_ASSERT defined, can't test assertions");
    #endif

    std::ostringstream out;
    Error redirectError{&out};

    vkSamplerAddressMode(Magnum::SamplerWrapping(0x123));
    CORRADE_COMPARE(out.str(),
        "Vk::vkSamplerAddressMode(): invalid wrapping SamplerWrapping(0x123)\n");
}

}}}}

CORRADE_TEST_MAIN(Magnum::Vk::Test::EnumsTest)
