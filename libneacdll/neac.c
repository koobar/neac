#include "pch.h"
#include "neac.h"
#include "neac_decoder.h"
#include "neac_encoder.h"

static uint64_t cast_to_uint64(neac_decoder* decoder) {
    return (uint64_t)decoder;
}

static neac_decoder* cast_to_decoder(uint64_t hDecoder) {
    return (neac_decoder*)hDecoder;
}

 void NeacInitLibrary() {

}

uint64_t NeacOpenFile(const char* path) {
    neac_decoder* decoder = neac_decoder_create(path);

    return cast_to_uint64(decoder);
}

int32_t NeacReadNextSample(uint64_t hDecoder){
    neac_decoder* decoder = cast_to_decoder(hDecoder);

    return neac_decoder_read_sample(decoder);
}

uint32_t NeacGetSampleRate(uint64_t hDecoder) {
    neac_decoder* decoder = cast_to_decoder(hDecoder);

    return (uint32_t)decoder->sample_rate;
}

uint32_t NeacGetBitsPerSample(uint64_t hDecoder) {
    neac_decoder* decoder = cast_to_decoder(hDecoder);

    return (uint32_t)decoder->bits_per_sample;
}

uint32_t NeacGetNumChannels(uint64_t hDecoder) {
    neac_decoder* decoder = cast_to_decoder(hDecoder);

    return (uint32_t)decoder->num_channels;
}

void NeacCloseFile(uint64_t hDecoder) {
    neac_decoder* decoder = cast_to_decoder(hDecoder);

    neac_decoder_close(decoder);
    neac_decoder_free(decoder);
}