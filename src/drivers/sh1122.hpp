#pragma once

#if defined(PICON_PLATFORM_PICO)
#include "graphics/image.hpp"

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/pio_instructions.h>
#include <hardware/spi.h>
#include <hardware/structs/io_bank0.h>
#include <pico.h>
#include <pico/platform/common.h>
#include <pico/time.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace picon::drivers
{

    enum class SH1122Commands : std::uint8_t
    {
        set_lower_column_addr = 0x00,
        set_higher_column_addr = 0x10,
        set_row_addr = 0xB0,
        display_off = 0xAE,
        display_on = 0xAF,
        set_display_clock_div = 0xD5,
        set_multiplex = 0xA8,
        set_display_offset = 0xD3,
        set_start_line = 0x40,
        charge_pump = 0xAD,
        memory_mode = 0xA0,
        segment_remap = 0xA0,
        com_scan_dir = 0xC0,
        set_contrast = 0x81,
        set_precharge = 0xD9,
        set_vcom_deselect = 0xDB,
        set_vsegm = 0xDC,
        set_discharge_vsl = 0x30,
        display_all_on_resume = 0xA4,
        normal_display = 0xA6,
    };

    constexpr auto operator+(const SH1122Commands cmd) noexcept
    {
        return static_cast<std::uint8_t>(cmd);
    }

    template <
        std::size_t t_width,
        std::size_t t_height,
        std::uint8_t t_sck,
        std::uint8_t t_mosi,
        std::uint8_t t_dc,
        std::uint8_t t_cs,
        std::uint8_t t_rst>
    class SH1122Driver
    {
    private:
        static constexpr auto width = t_width;
        static constexpr auto height = t_height;
        static constexpr auto sck = t_sck;
        static constexpr auto mosi = t_mosi;
        static constexpr auto dc = t_dc;
        static constexpr auto cs = t_cs;
        static constexpr auto rst = t_rst;

    public:
        using FrameBufferData = graphics::ImageData<graphics::color::GS4, width, height>;
        using FrameBuffer = graphics::Image<graphics::color::GS4>;

    private:
        std::array<FrameBufferData, 2> frame_buffer_data{};
        std::array<FrameBuffer, 2> frame_buffers{
            frame_buffer_data[0],
            frame_buffer_data[1],
        };
        std::size_t front_buffer_idx{};

        std::uint8_t dma_channel{};
        dma_channel_config dma_config{};

        PIO pio{};
        uint pio_sm{};
        uint pio_offset{};

        uint pio_command_start{};
        uint pio_command_end{};
        uint pio_data_start{};
        uint pio_data_end{};

    public:
        SH1122Driver(PIO p_pio): pio{p_pio} {}

        void init()
        {
            initSpiPio(pio);
            initDma();
            initGpio();
            initDisplay();
        }

        void deinit()
        {
            // TODO: add proper deinit stuff
        }

        FrameBuffer& getFrontBuffer()
        {
            return frame_buffers[front_buffer_idx];
        }

        FrameBuffer& getBackBuffer()
        {
            return frame_buffers[(front_buffer_idx + 1) % frame_buffers.size()];
        }

        void swapBuffers()
        {
            if (dma_channel_is_busy(dma_channel))
            {
                dma_channel_wait_for_finish_blocking(dma_channel);
            }

            deselectDevice();
            selectDevice();
            
            // setCmdMode();
            // writeCmd(+SH1122Commands::set_row_addr, 0x00);
            // writeCmd(+SH1122Commands::set_lower_column_addr, +SH1122Commands::set_higher_column_addr);
            // setDataMode();

            const auto &back_buffer = getBackBuffer();
            front_buffer_idx = (front_buffer_idx + 1) % frame_buffers.size();

            pioSpiWrite(std::bit_cast<const std::uint8_t*>(back_buffer.data()), back_buffer.size());

        }

    private:
        void selectDevice()
        {
            gpio_put(cs, 0);
        }

        void deselectDevice()
        {
            gpio_put(cs, 1);
        }

        void setCmdMode()
        {
            pio_sm_set_enabled(pio, pio_sm, false);
            pio_sm_set_wrap(pio, pio_sm, pio_offset + pio_command_start, pio_offset + pio_command_end);
            pio->sm[pio_sm].instr = pio_offset + pio_command_start;
            pio_sm_set_enabled(pio, pio_sm, true);
            gpio_put(dc, 0);
        }

        void setDataMode()
        {
            pio_sm_set_enabled(pio, pio_sm, false);
            pio_sm_set_wrap(pio, pio_sm, pio_offset + pio_data_start, pio_offset + pio_data_end);
            pio->sm[pio_sm].instr = pio_offset + pio_data_start;
            pio_sm_set_enabled(pio, pio_sm, true);
            gpio_put(dc, 1);
        }

        void writeCmd(std::uint8_t p_cmd)
        {
            pioSpiWriteBlocking(&p_cmd, 1);
        }

        void writeCmd(std::uint8_t p_cmd, std::uint8_t p_value)
        {
            writeCmd(p_cmd);
            writeCmd(p_value);
        }

        void initDisplay()
        {
            selectDevice();
            setCmdMode();
            writeCmd(+SH1122Commands::display_off);
            // writeCmd(+SH1122Commands::set_display_clock_div, 0xF0);
            writeCmd(+SH1122Commands::set_display_clock_div, 0x50);
            // writeCmd(+SH1122Commands::set_display_offset, 0x00);
            writeCmd(+SH1122Commands::set_start_line, 0x00);
            writeCmd(+SH1122Commands::charge_pump, 0x8F);
            writeCmd(+SH1122Commands::segment_remap | 0x00);
            writeCmd(+SH1122Commands::com_scan_dir);
            writeCmd(+SH1122Commands::set_contrast, 0x10);
            writeCmd(+SH1122Commands::set_contrast, 0x70);
            writeCmd(+SH1122Commands::set_multiplex, 0x3F);
            // writeCmd(+SH1122Commands::set_precharge, 0x81);
            writeCmd(+SH1122Commands::set_precharge, 0x92);

            writeCmd(+SH1122Commands::set_vcom_deselect, 0x00);
            writeCmd(+SH1122Commands::set_vsegm, 0x00);
            // writeCmd(+SH1122Commands::set_vcom_deselect, 0x35);
            // writeCmd(+SH1122Commands::set_vsegm, 0x35);
            writeCmd(+SH1122Commands::set_discharge_vsl | 0x06);
            // writeCmd(+SH1122Commands::set_discharge_vsl | 0x00);
            
            writeCmd(+SH1122Commands::display_all_on_resume);
            writeCmd(+SH1122Commands::normal_display);
            writeCmd(+SH1122Commands::display_on);

            setDataMode();

            deselectDevice();
        }

        void initGpio()
        {
            gpio_init(dc);
            gpio_init(cs);
            gpio_init(rst);
            gpio_set_dir(dc, GPIO_OUT);
            gpio_set_dir(cs, GPIO_OUT);
            gpio_set_dir(rst, GPIO_OUT);
            gpio_put(rst, 1);
        }
        
        void initDma()
        {
            dma_channel = dma_claim_unused_channel(true);
            dma_config = dma_channel_get_default_config(dma_channel);
            channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
            channel_config_set_dreq(&dma_config, pio_get_dreq(pio, pio_sm, true));
        }

        void initSpiPio(PIO p_pio)
        {
            pio = p_pio;
            
            static const std::array<std::uint16_t, 8> instructions{
                // command program
                static_cast<std::uint16_t>(
                    pio_encode_set(pio_src_dest::pio_x, 7) |
                    pio_encode_sideset(1, 0)),
                static_cast<std::uint16_t>(
                    pio_encode_out(pio_src_dest::pio_pins, 1) |
                    pio_encode_sideset(1, 0)),
                static_cast<std::uint16_t>(
                    pio_encode_jmp_x_dec(1) |
                    pio_encode_sideset(1, 1)),
                static_cast<std::uint16_t>(
                    pio_encode_irq_set(false, 0) |
                    pio_encode_sideset(1, 0)),
                // data program
                static_cast<std::uint16_t>(
                    pio_encode_out(pio_src_dest::pio_null, 4) |
                    pio_encode_sideset(1, 0)),
                static_cast<std::uint16_t>(
                    pio_encode_set(pio_src_dest::pio_x, 3) |
                    pio_encode_sideset(1, 0)),
                static_cast<std::uint16_t>(
                    pio_encode_out(pio_src_dest::pio_pins, 1) |
                    pio_encode_sideset(1, 0)),
                static_cast<std::uint16_t>(
                    pio_encode_jmp_x_dec(6) |
                    pio_encode_sideset(1, 1)),
            };

            pio_command_start = 0;
            pio_command_end = 3;
            pio_data_start = 4;
            pio_data_end = 7;

            // printf("out: 0x%x\n", instructions[0]);
            // printf("in: 0x%x\n", instructions[1]);

            const pio_program program {
                .instructions = instructions.data(),
                .length = static_cast<std::uint8_t>(instructions.size()),
                .origin = -1,
                .pio_version = 0,
            };

            pio_sm = pio_claim_unused_sm(pio, true);

            pio_offset = pio_add_program(pio, &program);
            auto config{pio_get_default_sm_config()};

            sm_config_set_wrap(&config, pio_offset + pio_command_start, pio_offset + pio_command_end);
            // sm_config_set_wrap(&config, pio_offset + pio_data_start, pio_offset + pio_data_end);

            sm_config_set_sideset(&config, 1, false, false);
            sm_config_set_out_shift(&config, false, true, 8);
            sm_config_set_in_shift(&config, false, false, 8);
            // sm_config_set_clkdiv_int_frac8(&config, 5, 0);
            sm_config_set_clkdiv_int_frac8(&config, 4, 0);
            // sm_config_set_clkdiv_int_frac8(&config, 3, 0);
            // sm_config_set_clkdiv_int_frac8(&config, 2, 0);

            sm_config_set_out_pins(&config, mosi, 1);
            sm_config_set_sideset_pins(&config, sck);
            
            pio_sm_set_pins_with_mask(pio, pio_sm, 0, (1u << sck) | (1u << mosi));
            pio_sm_set_pindirs_with_mask(pio, pio_sm, -1, (1u << sck) | (1u << mosi));
            pio_gpio_init(pio, mosi);
            pio_gpio_init(pio, sck);

            pio_sm_init(pio, pio_sm, pio_offset, &config);

            pio_sm_set_enabled(pio, pio_sm, true);
        }

        void __time_critical_func(pioSpiWriteBlocking)(const std::uint8_t* p_src, std::size_t p_len)
        {
            pioSpiWrite(p_src, p_len);

            dma_channel_wait_for_finish_blocking(dma_channel);

            while (!pio_interrupt_get(pio, 0))
            {
                tight_loop_contents();
            }
            pio_interrupt_clear(pio, 0);
        }
        
        void __time_critical_func(pioSpiWrite)(const std::uint8_t* p_src, std::size_t p_len)
        {
            dma_channel_configure(
                dma_channel,
                &dma_config,
                &pio->txf[pio_sm],
                p_src,
                p_len,
                true);
        }


    };

} // namespace picon::drivers

#endif