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

#include <Corrade/Containers/ArrayView.h> /* arraySize() */
#include <Corrade/Containers/String.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/Format.h>

#include "Magnum/Shaders/FlatGL.h"

namespace Magnum { namespace Shaders { namespace Test { namespace {

/* There's an underscore between GL and Test to disambiguate from GLTest, which
   is a common suffix used to mark tests that need a GL context. Ugly, I know. */
struct FlatGL_Test: TestSuite::Tester {
    explicit FlatGL_Test();

    #ifndef MAGNUM_TARGET_GLES2
    template<UnsignedInt dimensions> void configurationSetJointCountInvalid();
    #endif

    template<UnsignedInt dimensions> void constructNoCreate();
    template<UnsignedInt dimensions> void constructCopy();

    void debugFlag();
    void debugFlags();
    void debugFlagsSupersets();
};

#ifndef MAGNUM_TARGET_GLES2
const struct {
    const char* name;
    UnsignedInt jointCount, perVertexJointCount, secondaryPerVertexJointCount;
    const char* message;
} ConfigurationSetJointCountInvalidData[] {
    {"per-vertex joint count too large",
        10, 5, 0,
        "expected at most 4 per-vertex joints, got 5"},
    {"secondary per-vertex joint count too large",
        10, 0, 5,
        "expected at most 4 secondary per-vertex joints, got 5"},
    {"joint count but no per-vertex joint count",
        10, 0, 0,
        "count has to be zero if per-vertex joint count is zero"},
    /* The rest depends on flags being set and is thus verified in constructor,
       tested in FlatGLTest::constructInvalid() and
       constructUniformBuffersInvalid() */
};
#endif

FlatGL_Test::FlatGL_Test() {
    #ifndef MAGNUM_TARGET_GLES2
    addInstancedTests<FlatGL_Test>({
        &FlatGL_Test::configurationSetJointCountInvalid<2>,
        &FlatGL_Test::configurationSetJointCountInvalid<3>},
        Containers::arraySize(ConfigurationSetJointCountInvalidData));
    #endif

    addTests({&FlatGL_Test::constructNoCreate<2>,
              &FlatGL_Test::constructNoCreate<3>,

              &FlatGL_Test::constructCopy<2>,
              &FlatGL_Test::constructCopy<3>,

              &FlatGL_Test::debugFlag,
              &FlatGL_Test::debugFlags,
              &FlatGL_Test::debugFlagsSupersets});
}

#ifndef MAGNUM_TARGET_GLES2
template<UnsignedInt dimensions> void FlatGL_Test::configurationSetJointCountInvalid() {
    auto&& data = ConfigurationSetJointCountInvalidData[testCaseInstanceId()];
    setTestCaseDescription(data.name);
    setTestCaseTemplateName(Utility::format("{}", dimensions));

    CORRADE_SKIP_IF_NO_ASSERT();

    typename FlatGL<dimensions>::Configuration configuration;

    Containers::String out;
    Error redirectError{&out};
    configuration.setJointCount(data.jointCount, data.perVertexJointCount, data.secondaryPerVertexJointCount);
    CORRADE_COMPARE(out, Utility::format("Shaders::FlatGL::Configuration::setJointCount(): {}\n", data.message));
}
#endif

template<UnsignedInt dimensions> void FlatGL_Test::constructNoCreate() {
    setTestCaseTemplateName(Utility::format("{}", dimensions));

    {
        FlatGL<dimensions> shader{NoCreate};
        CORRADE_COMPARE(shader.id(), 0);
        CORRADE_COMPARE(shader.flags(), typename FlatGL<dimensions>::Flags{});
    }

    CORRADE_VERIFY(true);
}

template<UnsignedInt dimensions> void FlatGL_Test::constructCopy() {
    setTestCaseTemplateName(Utility::format("{}", dimensions));

    CORRADE_VERIFY(!std::is_copy_constructible<FlatGL<dimensions>>{});
    CORRADE_VERIFY(!std::is_copy_assignable<FlatGL<dimensions>>{});
}

void FlatGL_Test::debugFlag() {
    Containers::String out;

    Debug{&out} << FlatGL3D::Flag::Textured << FlatGL3D::Flag(0xf00d);
    CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::Textured Shaders::FlatGL::Flag(0xf00d)\n");
}

void FlatGL_Test::debugFlags() {
    Containers::String out;

    Debug{&out} << (FlatGL3D::Flag::Textured|FlatGL3D::Flag::AlphaMask) << FlatGL3D::Flags{};
    CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::Textured|Shaders::FlatGL::Flag::AlphaMask Shaders::FlatGL::Flags{}\n");
}

void FlatGL_Test::debugFlagsSupersets() {
    #ifndef MAGNUM_TARGET_GLES2
    /* InstancedObjectId and ObjectIdTexture are a superset of ObjectId so only
       one should be printed, but if there are both then both should be */
    {
        Containers::String out;
        Debug{&out} << (FlatGL3D::Flag::ObjectId|FlatGL3D::Flag::InstancedObjectId);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::InstancedObjectId\n");
    } {
        Containers::String out;
        Debug{&out} << (FlatGL3D::Flag::ObjectId|FlatGL3D::Flag::ObjectIdTexture);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::ObjectIdTexture\n");
    } {
        Containers::String out;
        Debug{&out} << (FlatGL3D::Flag::ObjectId|FlatGL3D::Flag::InstancedObjectId|FlatGL3D::Flag::ObjectIdTexture);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::InstancedObjectId|Shaders::FlatGL::Flag::ObjectIdTexture\n");
    }
    #endif

    /* InstancedTextureOffset is a superset of TextureTransformation so only
       one should be printed */
    {
        Containers::String out;
        Debug{&out} << (FlatGL3D::Flag::InstancedTextureOffset|FlatGL3D::Flag::TextureTransformation);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::InstancedTextureOffset\n");
    }

    #ifndef MAGNUM_TARGET_GLES2
    /* MultiDraw and ShaderStorageBuffers are a superset of UniformBuffers so
       only one should be printed, but if there are both then both should be */
    {
        Containers::String out;
        Debug{&out} << (FlatGL3D::Flag::MultiDraw|FlatGL3D::Flag::UniformBuffers);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::MultiDraw\n");
    }
    #ifndef MAGNUM_TARGET_WEBGL
    {
        Containers::String out;
        Debug{&out} << (FlatGL2D::Flag::ShaderStorageBuffers|FlatGL2D::Flag::UniformBuffers);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::ShaderStorageBuffers\n");
    } {
        Containers::String out;
        Debug{&out} << (FlatGL3D::Flag::MultiDraw|FlatGL3D::Flag::ShaderStorageBuffers|FlatGL3D::Flag::UniformBuffers);
        CORRADE_COMPARE(out, "Shaders::FlatGL::Flag::MultiDraw|Shaders::FlatGL::Flag::ShaderStorageBuffers\n");
    }
    #endif
    #endif
}

}}}}

CORRADE_TEST_MAIN(Magnum::Shaders::Test::FlatGL_Test)
