#pragma once
#include <stdint.h>

extern void NeacInitLibrary();
extern uint64_t NeacOpenFile(const char* path);
extern int32_t NeacReadNextSample(uint64_t decoder_id);
extern uint32_t NeacGetSampleRate(uint64_t decoder_id);
extern uint32_t NeacGetBitsPerSample(uint64_t decoder_id);
extern uint32_t NeacGetNumChannels(uint64_tdecoder_id);
extern void NeacCloseFile(uint64_t decoder_id);