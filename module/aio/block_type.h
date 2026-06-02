/**
 * MIT License
 *
 * Copyright (c) 2026 Mag1c.H
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * */
#ifndef AIO_BLOCK_TYPE_H
#define AIO_BLOCK_TYPE_H

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

namespace aio {

using BlockId = std::array<std::byte, 16>;
struct BlockIdHasher {
    size_t operator()(const BlockId& blockId) const noexcept
    {
        std::string_view sv(reinterpret_cast<const char*>(blockId.data()), blockId.size());
        return std::hash<std::string_view>{}(sv);
    }
};

inline std::string BlockIdToString(const BlockId& blockId)
{
    constexpr char hexChars[] = "0123456789abcdef";
    std::string result;
    result.reserve(blockId.size() * 2);
    for (const auto byte : blockId) {
        const auto value = std::to_integer<unsigned char>(byte);
        result.push_back(hexChars[value >> 4]);
        result.push_back(hexChars[value & 0x0F]);
    }
    return result;
}

}  // namespace aio

#endif
