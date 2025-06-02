#pragma once

#include <pico/stdlib.h>

#include <array>
#include <cstdint>

namespace pg {

    /// An enum of all possible image formats
    enum class ImageFormat
    {
        /// 4 bit grayscale, with upper-nibble on the left, and lower-nibble on the right
        GS4_HMSB,
        /// 4 bit grayscale, with 1 byte per pixel
        GS4,
    };

    /// Used to specify at what ratio an image's `Data` maps to pixels.
    /// For example, if `Data = uint8_t` and each pixel is 4 bits,
    /// we can specify a ratio of 1/2 or `ImagePxRatio{1, 2}` (1 byte / 2 pixels).
    struct ImagePxRatio
    {
        std::uint8_t numerator{1};
        std::uint8_t denominator{1};
    };


    /// Should be specialized for each image format
    template <ImageFormat t_image_format>
    struct ImageFormatTraits
    {
        // using BlendMode = void;
        using Data = uint8_t;
        static constexpr ImagePxRatio px_ratio{};
        static constexpr Data max_value{};
    };

    
    /// Represents the core image logic.
    /// Uses CRTP to support specialization.
    /// Additional image formats must implement getPixel and setPixel.
    /// Other methods may be specialized as well.
    template <ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived = void>
    struct Image
    {
        using Derived = T_Derived;
        using Data = ImageFormatTraits<t_image_format>::Data;
        using BlendMode = Data (*)(Data, Data);
        static constexpr auto image_format = t_image_format;
        static constexpr auto px_ratio = ImageFormatTraits<t_image_format>::px_ratio;
        static constexpr auto max_value = ImageFormatTraits<t_image_format>::max_value;
        static constexpr auto width = t_width;
        static constexpr auto height = t_height;

        std::array<Data, (width * height * px_ratio.numerator) / px_ratio.denominator> buffer{};

        constexpr Image() = default;
        constexpr Image(const Image &) = delete;
        constexpr Image &operator=(const Image &) = delete;
        constexpr Image(Image &&) = default;
        constexpr Image &operator=(Image &&) = default;
        constexpr ~Image() = default;

        template <typename... VT_Args>
        constexpr Image(VT_Args... p_args) : buffer{p_args...} {}

        constexpr Derived clone() const { return Derived(buffer); }

        constexpr Data getPixel(std::size_t p_x, std::size_t p_y) const = delete;
        Data setPixel(std::size_t p_x, std::size_t p_y, Data p_value, BlendMode p_blend_mode = {}) const = delete;

        constexpr const auto &getBuffer() const { return buffer; }
        auto &getBuffer() { return buffer; }

        void clear();
        void fill(Data p_value, BlendMode p_blend_mode = {});
        
        void flip(bool p_x, bool p_y);
        Derived flipped(bool p_x, bool p_y) const;

        template<std::size_t t_other_width, std::size_t t_other_height>
        void blit(
            std::size_t p_dst_x, std::size_t p_dst_y,
            const Image<image_format, t_other_width, t_other_height> &p_src,
            std::size_t p_src_x, std::size_t p_src_y,
            std::size_t p_src_w, std::size_t p_src_h,
            BlendMode p_blend_mode = {});

        template<std::size_t t_other_width, std::size_t t_other_height>
        void blitSafe(
            std::int64_t p_dst_x, std::int64_t p_dst_y,
            const Image<image_format, t_other_width, t_other_height> &p_src,
            std::size_t p_src_x, std::size_t p_src_y,
            std::size_t p_src_w, std::size_t p_src_h,
            BlendMode p_blend_mode = {});

    };

    
    template<ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived>
    inline void Image<t_image_format, t_width, t_height, T_Derived>::clear()
    {
        static_cast<Derived *>(this)->fill(Data{});
    }

    template<ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived>
    inline void Image<t_image_format, t_width, t_height, T_Derived>::fill(Data p_value, BlendMode p_blend_mode)
    {
        for (std::size_t x = 0; x < t_width; ++x)
        {
            for (std::size_t y = 0; y < t_height; ++y)
            {
                static_cast<Derived *>(this)->setPixel(x, y, p_value, p_blend_mode);
            }
        }
    }

    
    template<ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived>
    inline void Image<t_image_format, t_width, t_height, T_Derived>::flip(bool p_x, bool p_y)
    {
        if (p_x)
        {
            for (auto y = 0; y < height; ++y)
            {
                std::size_t left = 0;
                std::size_t right = width - 1;
                while (left < right)
                {
                    const auto left_prev = static_cast<Derived *>(this)->getPixel(left, y);
                    const auto right_prev = static_cast<Derived *>(this)->getPixel(right, y);
                    static_cast<Derived *>(this)->setPixel(left++, y, right_prev);
                    static_cast<Derived *>(this)->setPixel(right--, y, left_prev);
                }
            }
        }

        if (p_y)
        {
            for (auto x = 0; x < width; ++x)
            {
                std::size_t top = 0;
                std::size_t bottom = height - 1;
                while (top < bottom)
                {
                    const auto top_prev = static_cast<Derived *>(this)->getPixel(x, top);
                    const auto bottom_prev = static_cast<Derived *>(this)->getPixel(x, bottom);
                    static_cast<Derived *>(this)->setPixel(x, top++, bottom_prev);
                    static_cast<Derived *>(this)->setPixel(x, bottom--, top_prev);
                }
            }
        }
    }

    template<ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived>
    inline T_Derived Image<t_image_format, t_width, t_height, T_Derived>::flipped(bool p_x, bool p_y) const
    {
        auto result = this->clone();
        static_cast<Derived *>(&result)->flip(p_x, p_y);
        return result;
    }

    template <ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived>
    template <std::size_t t_other_width, std::size_t t_other_height>
    inline void Image<t_image_format, t_width, t_height, T_Derived>::blit(
        std::size_t p_dst_x, std::size_t p_dst_y,
        const Image<image_format, t_other_width, t_other_height> &p_src,
        std::size_t p_src_x, std::size_t p_src_y,
        std::size_t p_src_w, std::size_t p_src_h,
        BlendMode p_blend_mode)
    {
        // clang-format off
        if (p_src_x == -1) { p_src_x = 0; }
        if (p_src_y == -1) { p_src_y = 0; }
        if (p_src_w == -1) { p_src_w = p_src.width; }
        if (p_src_h == -1) { p_src_h = p_src.height; }
        // clang-format on

        assert(p_dst_x + p_src_w <= width);
        assert(p_dst_y + p_src_h <= height);

        for (size_t sy = 0; sy < p_src_h; ++sy)
        {
            for (size_t sx = 0; sx < p_src_w; ++sx)
            {
                auto src_pixel = p_src.getPixel(p_src_x + sx, p_src_y + sy);
                static_cast<Derived *>(this)->setPixel(p_dst_x + sx, p_dst_y + sy, src_pixel, p_blend_mode);
            }
        }
    }

    template <ImageFormat t_image_format, std::size_t t_width, std::size_t t_height, typename T_Derived>
    template <std::size_t t_other_width, std::size_t t_other_height>
    inline void Image<t_image_format, t_width, t_height, T_Derived>::blitSafe(
        std::int64_t p_dst_x, std::int64_t p_dst_y,
        const Image<image_format, t_other_width, t_other_height> &p_src,
        std::size_t p_src_x, std::size_t p_src_y,
        std::size_t p_src_w, std::size_t p_src_h,
        BlendMode p_blend_mode)
    {
        // clang-format off
        if (p_src_x == -1) { p_src_x = 0; }
        if (p_src_y == -1) { p_src_y = 0; }
        if (p_src_w == -1) { p_src_w = p_src.width; }
        if (p_src_h == -1) { p_src_h = p_src.height; }
        // clang-format on
        
        // completely left of image
        if (-p_dst_x > p_src_w) { return; }
        // completely right of image
        if (p_dst_x > width) { return; }
        // completely above of image
        if (-p_dst_y > p_src_h) { return; }
        // completely below of image
        if (p_dst_y > height) { return; }
        

        // left of image
        if (p_dst_x < 0)
        {
            p_src_x = -p_dst_x;
            p_src_w = p_src_w + p_dst_x;
            p_dst_x = 0;
        }

        // right of image
        if (p_dst_x + p_src_w > width)
        {
            p_src_w = width - p_dst_x;
        }

        // above of image
        if (p_dst_y < 0)
        {
            p_src_y = -p_dst_y;
            p_src_h = p_src_h + p_dst_y;
            p_dst_y = 0;
        }

        // below image
        if (p_dst_y + p_src_h > height)
        {
            p_src_h = height - p_dst_y;
        }

        static_cast<Derived *>(this)->blit(p_dst_x, p_dst_y, p_src, p_src_x, p_src_y, p_src_w, p_src_h, p_blend_mode);
    }


} // namespace pg