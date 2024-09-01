#ifndef NEAC_CODE_H
#define NEAC_CODE_H

#define ENTROPY_PARAMETER_MIN 0
#define ENTROPY_PARAMETER_MAX ENTROPY_PARAMETER_MIN + 31
#define ENTROPY_PARAMETER_NEED_BITS 5

#include "bit_stream.h"
#include "neac_block.h"
#include "neac_sub_block.h"
#include "signal.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct nea_code {
	bit_stream* bitstream;
} neac_code;

neac_code* neac_code_create(bit_stream* stream);
void neac_code_write_block(neac_code* coder, neac_block* block, uint16_t block_size);

void neac_code_read_block(neac_code* coder, neac_block* block, uint16_t block_size);

#endif
