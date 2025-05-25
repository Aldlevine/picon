#pragma once

#include "graphics/graphics.hpp"

#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/time.h>

#include <array>
#include <limits>

namespace pg
{

    enum class SH1122Commands : std::uint8_t
    {
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
        using FrameBuffer = Image<ImageFormat::GS4_HMSB, width, height>;

        spi_inst_t *spi;

    private:
        std::array<FrameBuffer, 2> frame_buffers;
        std::size_t front_buffer_idx = 0;
        std::uint8_t dma_channel = 0;
        dma_channel_config dma_config;

    public:
        void init(spi_inst_t* p_spi)
        {
            spi = p_spi;

            const auto bd = spi_init(spi, std::numeric_limits<uint>::max());
            spi_set_baudrate(spi, bd / 2);
            // spi_set_baudrate(spi, bd / 4);

            spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

            dma_channel = dma_claim_unused_channel(true);
            dma_config = dma_channel_get_default_config(dma_channel);
            channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
            channel_config_set_high_priority(&dma_config, true);
            channel_config_set_dreq(&dma_config, spi_get_dreq(spi, true));

            gpio_init(dc);
            gpio_init(cs);
            gpio_init(rst);
            gpio_set_dir(dc, GPIO_OUT);
            gpio_set_dir(cs, GPIO_OUT);
            gpio_set_dir(rst, GPIO_OUT);

            gpio_set_function(sck, GPIO_FUNC_SPI);
            gpio_set_function(mosi, GPIO_FUNC_SPI);

            gpio_put(rst, 1);

            selectDevice();
            setCmdMode();
            writeCmd(+SH1122Commands::display_off);
            // writeCmd(+SH1122Commands::set_display_clock_div, 0xF0);
            writeCmd(+SH1122Commands::set_display_clock_div, 0x50);
            writeCmd(+SH1122Commands::set_display_offset, 0x00);
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

            deselectDevice();
        }

        FrameBuffer &getFrontBuffer()
        {
            return frame_buffers[front_buffer_idx];
        }

        FrameBuffer &getBackBuffer()
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
            
            setCmdMode();
            writeCmd(0xB0, 0x00); // reset row 0
            writeCmd(0x00, 0x10); // reset to column 0
            setDataMode();

            const auto &back_buffer = getBackBuffer().getBuffer();
            dma_channel_configure(
                dma_channel,
                &dma_config,
                &spi_get_hw(spi)->dr,
                back_buffer.data(),
                back_buffer.size(),
                true);


            front_buffer_idx = (front_buffer_idx + 1) % frame_buffers.size();
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
            gpio_put(dc, 0);
        }

        void setDataMode()
        {
            gpio_put(dc, 1);
        }

        void writeCmd(std::uint8_t p_cmd)
        {
            spi_write_blocking(spi, &p_cmd, 1);
        }

        void writeCmd(std::uint8_t p_cmd, std::uint8_t p_value)
        {
            writeCmd(p_cmd);
            writeCmd(p_value);
        }

        void writeData(const std::uint8_t *p_data, std::size_t p_len)
        {
            spi_write_blocking(spi, p_data, p_len);
            return;
        }
    };

} // namespace pg