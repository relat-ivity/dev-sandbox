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
#include <chrono>
#include <cstdio>
#include <random>
#include "aio_engine.h"
#include "host_buffer.h"

struct Config {
    std::string workspace;
    aio::HostBuffer::Strategy ioType = aio::HostBuffer::Strategy::MMAP;
    size_t ioSize = 1024 * 1024;
    size_t ioNumber = 512;
    size_t deviceId = 0;
    size_t epochNumber = 32;

    bool Parse(int argc, char const* argv[])
    {
        auto PrintUsage = +[](const char* prog) {
            std::printf(
                "Usage: %s --workspace <path> [--io-type mmap|alloc] [--io-size <bytes>] "
                "[--io-number <n>] [--device-id <id>] [--epoch-number <n>]\n",
                prog ? prog : "aio_main");
        };
        for (int i = 1; i < argc; ++i) {
            std::string opt = argv[i];
            if (opt == "-h" || opt == "--help") {
                PrintUsage(argv[0]);
                return false;
            }

            if (opt == "--workspace") {
                if (i + 1 >= argc) {
                    std::printf("Missing value for --workspace\n");
                    return false;
                }
                workspace = argv[++i];
                continue;
            }

            if (opt == "--io-type") {
                if (i + 1 >= argc) {
                    std::printf("Missing value for --io-type\n");
                    return false;
                }
                std::string v = argv[++i];
                for (auto& c : v) {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                if (v == "mmap") {
                    ioType = aio::HostBuffer::Strategy::MMAP;
                } else if (v == "alloc") {
                    ioType = aio::HostBuffer::Strategy::ALLOC;
                } else {
                    std::printf("Unknown io-type: %s. Supported: mmap, alloc\n", v.c_str());
                    return false;
                }
                continue;
            }

            if (opt == "--io-size") {
                if (i + 1 >= argc) {
                    std::printf("Missing value for --io-size\n");
                    return false;
                }
                try {
                    ioSize = std::stoull(argv[++i]);
                } catch (const std::exception& e) {
                    std::printf("Invalid number for --io-size: %s\n", e.what());
                    return false;
                }
                continue;
            }

            if (opt == "--io-number") {
                if (i + 1 >= argc) {
                    std::printf("Missing value for --io-number\n");
                    return false;
                }
                try {
                    ioNumber = std::stoull(argv[++i]);
                } catch (const std::exception& e) {
                    std::printf("Invalid number for --io-number: %s\n", e.what());
                    return false;
                }
                continue;
            }

            if (opt == "--device-id") {
                if (i + 1 >= argc) {
                    std::printf("Missing value for --device-id\n");
                    return false;
                }
                try {
                    deviceId = std::stoull(argv[++i]);
                } catch (const std::exception& e) {
                    std::printf("Invalid number for --device-id: %s\n", e.what());
                    return false;
                }
                continue;
            }

            if (opt == "--epoch-number") {
                if (i + 1 >= argc) {
                    std::printf("Missing value for --epoch-number\n");
                    return false;
                }
                try {
                    epochNumber = std::stoull(argv[++i]);
                } catch (const std::exception& e) {
                    std::printf("Invalid number for --epoch-number: %s\n", e.what());
                    return false;
                }
                continue;
            }

            std::printf("Unknown option: %s\n", opt.c_str());
            return false;
        }

        if (workspace.empty()) {
            std::printf("Error: --workspace is required\n");
            PrintUsage(argv[0]);
            return false;
        }

        return true;
    }
    void Show() const
    {
        std::printf("Set Config::Workspace = %s.\n", workspace.c_str());
        std::printf("Set Config::IoType = %s.\n",
                    ioType == aio::HostBuffer::Strategy::ALLOC ? "alloc" : "mmap");
        std::printf("Set Config::IoSize = %zu.\n", ioSize);
        std::printf("Set Config::IoNumber = %zu.\n", ioNumber);
        std::printf("Set Config::DeviceId = %zu.\n", deviceId);
        std::printf("Set Config::EpochNumber = %zu.\n", epochNumber);
    }
};

std::vector<aio::BlockId> MakeBlockIdsRandomly(size_t number)
{
    auto makeBlockIdRandomly = +[] {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<std::uint8_t> dist(0, 255);
        aio::BlockId id;
        for (std::size_t i = 0; i < id.size(); ++i) { id[i] = static_cast<std::byte>(dist(gen)); }
        return id;
    };
    std::vector<aio::BlockId> blockIds;
    blockIds.reserve(number);
    for (size_t i = 0; i < number; i++) { blockIds.push_back(makeBlockIdRandomly()); }
    return blockIds;
}

template <bool write>
void Run(aio::AioEngine& ioEngine, aio::AioEngine::IoTask& task, size_t epochNumber)
{
    using namespace std::chrono;
    const char* taskType = write ? "Write" : "Read";
    const auto size = task.front().length;
    const auto number = task.size();
    const auto total = size * number;
    for (size_t i = 0; i < epochNumber; i++) {
        auto tp = steady_clock::now();
        if constexpr (write) {
            ioEngine.SubmitWrite(task)->Wait();
        } else {
            ioEngine.SubmitRead(task)->Wait();
        }
        auto cost = duration<double>(steady_clock::now() - tp).count() * 1e3;
        auto bandwidth = total / cost / 1e6;
        std::printf("[%04zu/%04zu] %s task(%zu x %zu) finish, cost=%.3fms, bw=%.3fGB/s.\n", i + 1,
                     epochNumber, taskType, size, number, cost, bandwidth);
    }
}

int main(int argc, char const* argv[])
{
    Config config;
    if (config.Parse(argc, argv)) {
        config.Show();

        aio::SpaceLayout layout{config.workspace};
        aio::AioEngine ioEngine{&layout, 32};
        aio::HostBuffer buffers{config.ioType, (int32_t)config.deviceId, config.ioSize,
                                config.ioNumber};
        auto blockIds = MakeBlockIdsRandomly(config.ioNumber);
        aio::AioEngine::IoTask task;
        task.reserve(config.ioNumber);
        for (size_t i = 0; i < config.ioNumber; i++) {
            task.push_back(aio::AioEngine::IoShard{blockIds[i], buffers[i], config.ioSize});
        }

        Run<true>(ioEngine, task, config.epochNumber);
        Run<false>(ioEngine, task, config.epochNumber);
        return 0;
    }
    return -1;
}
