#ifndef TLV_PARSER_H
#define TLV_PARSER_H

#include <stddef.h>
#include <stdint.h>

#include "tlv_def.h"

tlv_status_t tlv_parse_next(const uint8_t *buffer,
							size_t buf_len,
							size_t *offset,
							tlv_node_t *out_node);

tlv_status_t tlv_serialize_one(uint8_t type,
							   uint8_t length,
							   const uint8_t *value,
							   uint8_t *out_buf,
							   size_t out_len,
							   size_t *written);

#endif
