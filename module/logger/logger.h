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
#ifndef LOGGER_H
#define LOGGER_H

#include <cstdint>
#include <memory>
#include <string>

#if defined(__GNUC__) || defined(__clang__)
#define LOGGER_PRINTF_FORMAT(format_index, first_arg) \
    __attribute__((format(printf, format_index, first_arg)))
#else
#define LOGGER_PRINTF_FORMAT(format_index, first_arg)
#endif

class LoggerImpl;
class Logger {
public:
    enum class Level : uint8_t { DEBUG, INFO, WARN, ERROR, CRITICAL };
    struct SourceLocation {
        const char* file = "";
        const char* func = "";
        const int32_t line = 0;
    };

    static Logger& Instance()
    {
        static Logger instance;
        return instance;
    }
    ~Logger();
    void Log(Level lv, const SourceLocation& loc, const char* format, ...) LOGGER_PRINTF_FORMAT(4, 5);

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    void LogInternal(Level lv, const SourceLocation& loc, const std::string& message);

    std::unique_ptr<LoggerImpl> impl_;
};

#define LOG_SOURCE_LOCATION {__FILE__, __FUNCTION__, __LINE__}
#define __LOG(lv, format, ...) \
    Logger::Instance().Log(lv, LOG_SOURCE_LOCATION, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) __LOG(Logger::Level::DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) __LOG(Logger::Level::INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) __LOG(Logger::Level::WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) __LOG(Logger::Level::ERROR, format, ##__VA_ARGS__)
#define LOG_CRITICAL(format, ...) __LOG(Logger::Level::CRITICAL, format, ##__VA_ARGS__)

#undef LOGGER_PRINTF_FORMAT

#endif  // LOGGER_H
