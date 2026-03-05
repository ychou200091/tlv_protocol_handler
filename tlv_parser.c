#include "tlv_parser.h"

#include <string.h>

const char *tlv_type_name(uint8_t type) {
	/* English-only names */
	switch (type) {
		case TLV_TYPE_NAME: return "Name";
		case TLV_TYPE_ID: return "ID";
		case TLV_TYPE_IP_ADDR: return "IP Address";
		case TLV_TYPE_DEPARTMENT: return "Department";
		case TLV_TYPE_AGE: return "Age";
		case TLV_TYPE_NOTE: return "Note";
		default: return "Unknown";
	}
}
/*
    依照不同type決定value的解讀方式。 
    Determine how to interpret the value based on the type.
    Ex: id is uint32, age is uint8, name is text, etc.
*/
tlv_value_kind_t tlv_type_value_kind(uint8_t type) {
	switch (type) {
		case TLV_TYPE_NAME:
		case TLV_TYPE_IP_ADDR:
		case TLV_TYPE_DEPARTMENT:
		case TLV_TYPE_NOTE:
			return TLV_VALUE_KIND_TEXT_UTF8; // 直接當字串印出
		case TLV_TYPE_ID:
			return TLV_VALUE_KIND_UINT32_BE; //解析成 4-byte 整數再印出
		case TLV_TYPE_AGE:
			return TLV_VALUE_KIND_UINT8; // 印出十進位數字
		default:
			return TLV_VALUE_KIND_BINARY; // 用十六進位顯示
	}
}
// 
/*
從目前 offset 位置解析「下一個 TLV 節點」。

確認 buffer 裡的 value 長度足夠，把 *offset 往後移到下一個 TLV 起點。
Return Value: TLV_SUCCESS, TLV_ERR_BUFFER_TOO_SHORT, TLV_ERR_INVALID_LENGTH, TLV_ERR_INVALID_ARG
*/
tlv_status_t tlv_parse_next(const uint8_t *buffer,
							size_t buf_len,
							size_t *offset,
							tlv_node_t *out_node) {
	size_t pos;
	size_t value_start;
	size_t remaining;
	uint8_t length;

	if (buffer == NULL || offset == NULL || out_node == NULL) {
		return TLV_ERR_INVALID_ARG;
	}

	pos = *offset;
	if (pos >= buf_len) {
		return TLV_ERR_BUFFER_TOO_SHORT;
	}

	remaining = buf_len - pos;
	if (remaining < TLV_HEADER_SIZE) {
		return TLV_ERR_BUFFER_TOO_SHORT;
	}

	length = buffer[pos + 1U];
	value_start = pos + TLV_HEADER_SIZE;
	if ((buf_len - value_start) < (size_t)length) {
		return TLV_ERR_INVALID_LENGTH;
	}

	out_node->type = buffer[pos];
	out_node->length = length;
	out_node->value = &buffer[value_start];
	*offset = value_start + (size_t)length;

	return TLV_SUCCESS;
}

/*
把一個 TLV（type/length/value）寫入輸出 buffer。回填 *written = needed。

Return Value: TLV_SUCCESS, TLV_ERR_BUFFER_TOO_SHORT, TLV_ERR_INVALID_ARG
*/
tlv_status_t tlv_serialize_one(uint8_t type,
							   uint8_t length,
							   const uint8_t *value,
							   uint8_t *out_buf,
							   size_t out_len,
							   size_t *written) {
	size_t needed = TLV_HEADER_SIZE + (size_t)length;

	if (out_buf == NULL || written == NULL) {
		return TLV_ERR_INVALID_ARG;
	}

	if (length > 0U && value == NULL) {
		return TLV_ERR_INVALID_ARG;
	}

	if (out_len < needed) {
		return TLV_ERR_BUFFER_TOO_SHORT;
	}

	out_buf[0] = type;
	out_buf[1] = length;
	if (length > 0U) {
		memcpy(&out_buf[TLV_HEADER_SIZE], value, (size_t)length);
	}
	*written = needed;

	return TLV_SUCCESS;
}
