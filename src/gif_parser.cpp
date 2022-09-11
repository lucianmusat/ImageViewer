#include "gif_parser.hpp"


KauaiGifFilterData::KauaiGifFilterData(const uint8_t* data, const size_t size)
        : mData(data)
        , mSize(size)
{}

int32_t KauaiGifFilterData::get_field(GifHeaderMetadata metadata) const {
    const auto& field = metadata_fields.at(metadata);
    switch (field.size) {
        case 1:
            return read_byte_field(field.offset);
        case 2:
            return read_word_field(field.offset);
        case 3:
            return read_three_byte_field(field.offset);
        default:
            return std::numeric_limits<int32_t>::max();
    }
    return std::numeric_limits<int32_t>::max();
}

bool KauaiGifFilterData::check_header() const
{
    return memcmp(mData, "GIF", 3) == 0;
}

bool KauaiGifFilterData::check_version() const
{
    return ((memcmp(mData + 3, "87a", 3) == 0) || (memcmp(mData + 3, "89a", 3) == 0));
}

int32_t KauaiGifFilterData::bitDepth() const {
    return ((get_field(GifHeaderMetadata::kGlobalColorTable) >> 4) & 7) + 1;
}

int32_t KauaiGifFilterData::nrColors() const {
    return 1 << ((get_field(GifHeaderMetadata::kGlobalColorTable) & 0x07) + 1);
}

uint32_t KauaiGifFilterData::width() const {
    return get_field(GifHeaderMetadata::kWidth);
}

uint32_t KauaiGifFilterData::height() const {
    return get_field(GifHeaderMetadata::kHeight);
}

int32_t KauaiGifFilterData::loopCount() const {
    size_t offset = kDataAfterHeader;
    char app_id[8];
    char app_auth_code[3];
    do {
        if (offset > (mSize - kAEBlockSize)) {
            return kDefaultLoopCount;
        }
        // Found Application Extension Block
        if (get24le(&mData[offset]) == get24le(application_extension_start_bytes)) {
            memcpy(&app_id, &mData[offset + 3], 8);
            memcpy(&app_auth_code, &mData[offset + 11], 3);
            offset += 16; // Skip over the block bytes (3), app id (8), version (3) and sub block (2) bytes
            if ((strncmp(app_id, "NETSCAPE", 8) == 0) && (strncmp(app_auth_code, "2.0", 3) == 0)) {
                return read_byte_field(offset);
            }
        }
        offset++;
    } while (offset < mSize);
    return kDefaultLoopCount;
}

void KauaiGifFilterData::getAllColors() {
    size_t offset = kDataAfterGCT;
    for (auto i=0; i < nrColors(); i++) {
        colors.emplace_back(Color{mData[offset], mData[offset + 1], mData[offset + 2]});
        offset += 3;
    }
}

void KauaiGifFilterData::decodeAllFrames() {
    size_t offset = kDataAfterHeader;
    GifFrame frame{};
    getAllColors();
    do {
        // if (offset > (mSize - kGCEBlockSize)) {
        //     return;
        // }
        // Found a Graphic Control Extension Block
        if ((get16le(&mData[offset]) == get16le(GCE_start_bytes)) ){
            // Ignore if no image data follows
            // && (mData[offset + kGCEBlockSize] == 0x2c)) {
            frame = get_frame(offset);
            if (frame.gce.size > 0) {
                frame.transparent_color = colors[frame.gce.transparent_color_index];
                frames.emplace_back(frame);
            }
            offset += frame.gce.size + frame.data_size;
        }
        offset++;
    } while (offset < mSize);
}

GCE KauaiGifFilterData::get_GCE(const size_t offset)
{
    GCE gce{};
    const int32_t header_offset = offset + 2;
    gce.size = mData[header_offset];
    gce.is_transparent = mData[header_offset + 1] & 1;
    gce.animation_delay_cs = get16le(&mData[header_offset + 2]);
    gce.transparent_color_index = mData[header_offset + 4];
    if (mData[header_offset + gce.size + 1] != 0x00) {
        std::cout << "Did not detect GCE block end\n";
    } 
    std::cout << "Frame GCE size: " << gce.size << " animation delay: " << gce.animation_delay_cs << "\n";
    return gce;
}

GifFrame KauaiGifFilterData::get_frame(const size_t offset)
{
    GifFrame frame;
    frame.gce = get_GCE(offset);
    const int32_t frame_offset = offset + frame.gce.size + 4;
    if (mData[frame_offset] != 0x2c) {
        std::cout << "Did not detect image descriptor start\n";
        return {};
    }
    frame.position.x  = get16le(&mData[frame_offset + 1]);
    frame.position.y  = get16le(&mData[frame_offset + 3]);
    frame.size.width  = get16le(&mData[frame_offset + 5]);
    frame.size.height = get16le(&mData[frame_offset + 7]);
    // Jump over the Local color table bit and the Start of image field
    frame.data_size = mData[frame_offset + 11];
    frame.data.reserve(frame.data_size);
    for (size_t byte=0; byte < frame.data_size; byte++) {
        frame.data.emplace_back(mData[frame_offset + 11 + byte]);
    }
    std::cout << "Found a frame with " << frame.data.size() << " bytes " << 
                " Position: " << frame.position.x << "x" << frame.position.y <<
                " Size: " << frame.size.width << "x" << frame.size.height << 
                "\n";
    return frame;
}
