//
// Copyright (C) 2022 ActiveVideo Inc. Reproduction in whole or in part
// without written permission is prohibited. All rights reserved.
//
#pragma once

#include "endianess.hpp"

#include <vector>
#include <iostream>
#include <array>


constexpr uint8_t GCE_start_bytes[] = {0x21, 0xf9};
constexpr uint8_t GCE_end_byte = {0x00};
constexpr uint8_t application_extension_start_bytes[] = {0x21, 0xff, 0x0b};
constexpr uint32_t kDefaultLoopCount = 1;
constexpr uint32_t kDataAfterHeader = 6;
constexpr uint32_t kDataAfterGCT = 13; // Start of the GTC color list
constexpr uint32_t kGCEHeaderSize = 2;
constexpr uint32_t kGCEBlockSize = 8; // Including header
constexpr uint32_t kAEBlockSize = 2;
constexpr uint32_t kGCEBlockAndDescriptorSize = 3;

struct Position
{
    int32_t x{};
    int32_t y{};
};

struct Size
{
    int32_t width{};
    int32_t height{};
};

struct Color
{
    Color(uint8_t red, uint8_t green, uint8_t blue)
        : r(red)
         ,g(green)
         ,b(blue)
    {};
    Color() = default;
    uint8_t r{};
    uint8_t g{};
    uint8_t b{};
};

struct GCE
{
    int32_t size{};
    int32_t transparent_color{};
    bool is_transparent{};
    int32_t animation_delay_cs{}; // in hundredths of a second
    int32_t transparent_color_index{}; // Index of the color from the Global Color Table
};

struct GifFrame
{
    GCE gce{};
    Position position{};
    Size size{};
    Color transparent_color{};
    int32_t data_size{};
    std::vector<uint8_t> data{};
};

enum GifHeaderMetadata {
    kHeader,
    kVersion,
    kWidth,
    kHeight,
    kGlobalColorTable,
    kBackgroundColor,
    kAspectRatio,
    kGifHeaderMetadataSize
};

class KauaiGifFilterData
{
public:
    KauaiGifFilterData(const uint8_t* data, const size_t size);

    int32_t get_field(GifHeaderMetadata metadata) const;

    bool check_header() const;
    bool check_version() const;
    int32_t bitDepth() const;
    int32_t nrColors() const;
    int32_t loopCount() const;
    uint32_t width() const;
    uint32_t height() const;
    
    void getAllColors();
    void decodeAllFrames();


    std::vector<Color> colors; // All the colors in the GCT
    std::vector<GifFrame> frames;

private:
    struct FieldInfo {
            int offset;
            int size;
    };

    const std::array<FieldInfo, kGifHeaderMetadataSize> metadata_fields = {{
            // offset size
            {0,  3}, // Header
            {3,  3}, // Version
            {6,  2}, // Width
            {8,  2}, // Height
            {10, 1}, // GCT
            {11, 1}, // Background color index
            {12, 1}  // Aspect ratio
    }};

    int32_t read_byte_field(const size_t offset) const { return mData[offset]; }
    int32_t read_word_field(const size_t offset) const { return get16le(mData + offset); }
    int32_t read_three_byte_field(const size_t offset) const { return get24le(mData + offset); }

    GifFrame get_frame(const size_t offset);
    GCE get_GCE(const size_t offset);

    const uint8_t* mData;
    const size_t mSize;
};

inline std::unique_ptr<KauaiGifFilterData> parse_gif(const uint8_t* data, const size_t size)
{
    auto gif = std::unique_ptr<KauaiGifFilterData>(new KauaiGifFilterData(data, size));

    if (!gif->check_header()) {
        std::cout << "GIF header corrupt\n";
        return nullptr;
    }

    if (!gif->check_version()) {
        std::cout << "Invalid GIF version\n";
        return nullptr;
    }

    gif->decodeAllFrames();
    return gif;
}
