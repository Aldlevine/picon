#pragma once

#include <hardware/timer.h>
#include <pico/types.h>

#include <cstdint>

namespace picon::time
{

    template <std::uint64_t t_us_per_tick, auto t_callback>
    struct RepeatingTimer
    {
        private:
            constexpr static auto us_per_tick = t_us_per_tick;
            constexpr static void (*callback)(std::uint64_t) = t_callback; 
            static uint hw_alarm;

        public:
            static void init()
            {
                hw_alarm = hardware_alarm_claim_unused(true);
                hardware_alarm_set_callback(hw_alarm, timer_callback);
                timer_callback(hw_alarm);
            }

        private:
            static void timer_callback(uint p_hw_alarm)
            {
                assert(hw_alarm == p_hw_alarm);

                auto tick_start = time_us_64();
                hardware_alarm_set_target(hw_alarm, from_us_since_boot(tick_start + us_per_tick));
                callback(tick_start);
            }
    };

    template <std::uint64_t t_us_per_tick, auto t_callback>
    uint RepeatingTimer<t_us_per_tick, t_callback>::hw_alarm{};

    struct DeltaTimer
    {
    public:
        std::uint64_t min_delta_us;
        
    private:
        std::uint64_t last_tick_us{};
        std::uint64_t delta_us{};
        std::uint64_t delta_acc_us{};
        
    public:
        DeltaTimer(std::uint64_t p_min_delta_us) : min_delta_us(p_min_delta_us) {}

        void tick()
        {
            auto cur_tick {time_us_64()};
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
            auto cur_tick{time_us_64()};
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
