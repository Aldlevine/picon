#pragma once

#include "color.hpp"

#include <array>
#include <cassert>
#include <iterator>


namespace picon::graphics {

    /// owning container of image data.
    template <color::ColorType T_Format, std::size_t t_width, std::size_t t_height>
    struct ImageData
    {
        using Format = T_Format;

        constexpr static auto width = t_width;
        constexpr static auto height = t_height;

        std::array<Format, t_width * t_height> storage{};
        constexpr ImageData()  = default;
        constexpr ImageData(std::array<Format, t_width * t_height> p_storage) : storage{p_storage} {}

        // ImageData(const ImageData&) = delete;
        ImageData& operator=(const ImageData&) = delete;
        ImageData(ImageData&&) noexcept = default;
        ImageData& operator=(ImageData&&) noexcept = default;
        ~ImageData() = default;

        ImageData clone() const
        {
            return *this;
        }

        private:
            ImageData(const ImageData&) = default;

    };


    /// non-owning view of image data, runtime size.
    template <color::ColorType T_Format>
    struct Image
    {
        using Format = T_Format;

        std::size_t width;
        std::size_t height;
        Format* addr;

        constexpr Image(std::size_t p_width, std::size_t p_height, Format* p_addr) :
            width{p_width}, height{p_height}, addr{p_addr}
        {}

        template <std::size_t t_width, std::size_t t_height>
        constexpr Image(ImageData<Format, t_width, t_height> &p_image_data) :
            width{t_width}, height{t_height}, addr{p_image_data.storage.data()}
        {}

        template <std::size_t t_width, std::size_t t_height>
        constexpr Image(const ImageData<Format, t_width, t_height> &p_image_data) :
            width{t_width}, height{t_height}, addr{p_image_data.storage.data()}
        {}
        
        constexpr Image(const Image&) = default;
        constexpr Image& operator=(const Image&) = default;
        Image(Image&&) noexcept = default;
        Image& operator=(Image&&) noexcept = default;
        ~Image() = default;

        constexpr std::size_t size() const { return width * height; }
        constexpr std::size_t bytes() const { return size() * sizeof(Format); }

        constexpr Format* data() { return addr; }
        constexpr const Format* data() const { return addr; }

        constexpr Format* begin() { return addr; }
        constexpr const Format* begin() const { return addr; }

        constexpr Format* end() { return std::next(addr, size()); }
        constexpr const Format* end() const { return std::next(addr, size()); }

        constexpr Format* rowBegin(std::size_t p_y)
        {
            assert(p_y < height);
            return std::next(addr, p_y * width);
        }
        constexpr const Format* rowBegin(std::size_t p_y) const
        {
            return const_cast<const Format*>(const_cast<Image*>(this)->rowBegin(p_y));
        }

        constexpr const Format* rowEnd(std::size_t p_y) const
        {
            assert(p_y < height);
            return std::next(addr, p_y * (width + 1));
        }
        constexpr Format* rowEnd(std::size_t p_y)
        {
            return const_cast<Format*>(const_cast<const Image*>(this)->rowEnd(p_y));
        }

        constexpr const Format& at(std::size_t p_x, std::size_t p_y) const
        {
            assert(p_x < width && p_y < height);
            return addr[p_y * width + p_x];
        }
        constexpr Format& at(std::size_t x, std::size_t y)
        {
            return const_cast<Format&>(const_cast<const Image*>(this)->at(x, y));
        }
    };

} // namespace picon::graphics