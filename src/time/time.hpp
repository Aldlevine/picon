#pragma once

#include <cstdint>

#if defined(PICON_PLATFORM_LINUX)
#include <chrono>
#elif defined(PICON_PLATFORM_PICO)
#include <hardware/timer.h>
#endif

namespace picon::time
{

    inline std::uint64_t getEpochTimeUs64()
    #if defined(PICON_PLATFORM_LINUX)
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
    #elif defined(PICON_PLATFORM_PICO)
    {
        return time_us_64();
    }
    #endif

    struct DeltaTimer
    {
    public:
        std::uint64_t min_delta_us;
        
    private:
        std::uint64_t last_tick_us{ getEpochTimeUs64() };
        std::uint64_t delta_us{};
        std::uint64_t delta_acc_us{};
        
    public:
        DeltaTimer(std::uint64_t p_min_delta_us) : min_delta_us(p_min_delta_us) {}

        void tick()
        {
            auto cur_tick { getEpochTimeUs64() };
            delta_acc_us += (cur_tick - last_tick_us);
            last_tick_us = cur_tick;
        }

        bool step()
        {
            if (delta_acc_us > min_delta_us)
            {
                delta_us = delta_acc_us;
                delta_acc_us = 0;
                return true;
            }
            return false;
        }
        
        const std::uint64_t getDeltaLeft() const
        {
            return min_delta_us - delta_acc_us;
        }

        const std::uint64_t& getDelta() const
        {
            return delta_us;
        }

        const std::uint64_t& getTime() const
        {
            return last_tick_us;
        }
    };

    struct FixedDeltaTimer
    {
    public:
        std::uint64_t step_delta_us;

    private:
        std::uint64_t last_tick_us{};
        std::uint64_t delta_acc_us{};

    public:
        FixedDeltaTimer(std::uint64_t p_step_delta_us) : step_delta_us(p_step_delta_us) {}

        void tick()
        {
            auto cur_tick { getEpochTimeUs64() };
            delta_acc_us += (cur_tick - last_tick_us);
            last_tick_us = cur_tick;
        }

        bool step()
        {
            if (delta_acc_us > step_delta_us)
            {
                delta_acc_us -= step_delta_us;
                return true;
            }
            return false;
        }

        const std::uint64_t getDeltaLeft() const
        {
            return step_delta_us - delta_acc_us;
        }

        const std::uint64_t& getDelta() const
        {
            return step_delta_us;
        }

        const std::uint64_t& getTime() const
        {
            return last_tick_us;
        }
        
    };

} // namespace picon::time
