#ifndef Magnum_GL_Implementation_State_h
#define Magnum_GL_Implementation_State_h
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

#include <iosfwd>
#include <Corrade/Containers/Containers.h>

#include "Magnum/Magnum.h"
#include "Magnum/GL/GL.h"

namespace Magnum { namespace GL { namespace Implementation {

struct BufferState;
struct ContextState;
#ifndef MAGNUM_TARGET_WEBGL
struct DebugState;
#endif
struct FramebufferState;
struct MeshState;
struct QueryState;
struct RendererState;
struct ShaderState;
struct ShaderProgramState;
struct TextureState;
#ifndef MAGNUM_TARGET_GLES2
struct TransformFeedbackState;
#endif

struct State {
    /* Initializes context-based functionality together with all nested classes
       in a single allocation */
    static Containers::Pair<Containers::ArrayTuple, Containers::Reference<State>> allocate(Context& context, std::ostream* out);

    enum: GLuint { DisengagedBinding = ~0u };

    BufferState& buffer;
    ContextState& context;
    #ifndef MAGNUM_TARGET_WEBGL
    DebugState& debug;
    #endif
    FramebufferState& framebuffer;
    MeshState& mesh;
    QueryState& query;
    RendererState& renderer;
    ShaderState& shader;
    ShaderProgramState& shaderProgram;
    TextureState& texture;
    #ifndef MAGNUM_TARGET_GLES2
    TransformFeedbackState& transformFeedback;
    #endif
};

}}}

#endif
