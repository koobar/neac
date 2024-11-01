#include "./include/neacenc.h"
#include "neac_error.h"

HENCODER CreateEncoderFromPath(
    LPCSTR output,
    uint32_t sample_rate,
    uint8_t bits_per_sample,
    uint8_t num_channels,
    uint32_t num_samples,
    uint16_t block_size,
    bool use_mid_side_stereo,
    uint8_t filter_taps,
    neac_tag* tag) {
    set_on_error_exit(false);
    return neac_encoder_create_from_path(
        output,
        sample_rate,
        bits_per_sample,
        num_channels,
        num_samples,
        block_size,
        use_mid_side_stereo,
        filter_taps,
        tag);
}

HENCODER CreateEncoderFromFile(
    FILE* file,
    uint32_t sample_rate,
    uint8_t bits_per_sample,
    uint8_t num_channels,
    uint32_t num_samples,
    uint16_t block_size,
    bool use_mid_side_stereo,
    uint8_t filter_taps,
    neac_tag* tag) {
    set_on_error_exit(false);
    return neac_encoder_create(
        file,
        sample_rate,
        bits_per_sample,
        num_channels,
        num_samples,
        block_size,
        use_mid_side_stereo,
        filter_taps,
        tag);
}

void FreeEncoder(HENCODER encoder) {
    neac_encoder_free(encoder);
    set_on_error_exit(true);
}

void EncoderWriteSample(HENCODER encoder, int32_t sample) {
    neac_encoder_write_sample(encoder, sample);
}

void EncoderEndWrite(HENCODER encoder) {
    neac_encoder_end_write(encoder);
}