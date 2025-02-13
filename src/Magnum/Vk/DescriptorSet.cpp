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

#include "DescriptorSet.h"

#include "Magnum/Vk/Assert.h"
#include "Magnum/Vk/Device.h"
#include "Magnum/Vk/Result.h"

namespace Magnum { namespace Vk {

DescriptorSet DescriptorSet::wrap(Device& device, const VkDescriptorPool pool, const VkDescriptorSet handle, const HandleFlags flags) {
    DescriptorSet out{NoCreate};
    out._device = &device;
    out._pool = pool;
    out._handle = handle;
    out._flags = flags;
    return out;
}

DescriptorSet::DescriptorSet(NoCreateT): _device{}, _pool{}, _handle{} {}

DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept: _device{other._device}, _pool{other._pool}, _handle{other._handle}, _flags{other._flags} {
    other._handle = {};
}

DescriptorSet::~DescriptorSet() {
    if(_handle && (_flags & HandleFlag::DestroyOnDestruction))
        (**_device).FreeDescriptorSets(*_device, _pool, 1, &_handle);
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept {
    using Utility::swap;
    swap(other._device, _device);
    swap(other._pool, _pool);
    swap(other._handle, _handle);
    swap(other._flags, _flags);
    return *this;
}

VkDescriptorSet DescriptorSet::release() {
    const VkDescriptorSet handle = _handle;
    _handle = {};
    return handle;
}

}}
