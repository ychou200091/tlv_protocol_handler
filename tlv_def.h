#ifndef TLV_DEF_H
#define TLV_DEF_H

#include <stdint.h>

#define TLV_TYPE_SIZE (1U)
#define TLV_LENGTH_SIZE (1U)
#define TLV_HEADER_SIZE (TLV_TYPE_SIZE + TLV_LENGTH_SIZE)

// TLV Type Define
#define TLV_TYPE_NAME (0x01U)
#define TLV_TYPE_ID (0x02U)
#define TLV_TYPE_IP_ADDR (0x03U)
#define TLV_TYPE_DEPARTMENT (0x04U)
#define TLV_TYPE_AGE (0x05U)
#define TLV_TYPE_NOTE (0x06U)

typedef enum {
    TLV_SUCCESS = 0,
    TLV_ERR_BUFFER_TOO_SHORT,
    TLV_ERR_INVALID_LENGTH,
    TLV_ERR_INVALID_ARG,
    TLV_ERR_NOT_FOUND
} tlv_status_t;

// 依照不同type決定value的解讀方式
// 方便擴充，未來要加新 type 可以做對應的解析方式。
typedef enum {
    TLV_VALUE_KIND_TEXT_UTF8 = 0, // 直接當字串印出
    TLV_VALUE_KIND_UINT8, // 印出十進位數字
    TLV_VALUE_KIND_UINT32_BE, // 解析成 4-byte 整數再印出
    TLV_VALUE_KIND_BINARY // 用十六進位顯示
} tlv_value_kind_t;


// TLV 結構體
typedef struct {
    uint8_t type;
    uint8_t length;
    const uint8_t *value; // 指向原始數據(Zero-copy)
} tlv_node_t;

// 宣告函數接口
// 將type 轉換成可讀的字串，方便印出時顯示
// convert hex type to human-readable string for type
const char *tlv_type_name(uint8_t type); 

// 依照不同type決定value的解讀方式。 
// Determine how to interpret the value based on the type.
// Ex: id is uint32, age is uint8, name is text, etc.
tlv_value_kind_t tlv_type_value_kind(uint8_t type); 

/* [Endianness Check (手刻，不使用 library)]
 * Runtime detection: returns 1 if host is little-endian, 0 if big-endian.
 * Wire format is always big-endian (network byte order).
 * 
 * 原理：將 0x0001 存入 uint16_t，若記憶體最低地址存放的是 0x01（LSB），
 * 則表示 host 為 little-endian。
 *
 * Principle: store 0x0001 in a uint16_t; if the byte at the lowest address
 * is 0x01 (the LSB), the host is little-endian.
 *
 */
static inline int host_is_le(void) {
    const uint16_t probe = 0x0001U;
    // [0] = 0x01, [1] = 0x00 -> little-endian
    // [0] = 0x00, [1] = 0x01 -> big-endian
    return *((const uint8_t *)&probe) == 0x01U;
}

/*
Write a 32-bit host-order integer into 4 big-endian wire bytes.
*/
static inline void tlv_u32_to_be(uint32_t val, uint8_t out[4]) {
    out[0] = (uint8_t)((val >> 24U) & 0xFFU);
    out[1] = (uint8_t)((val >> 16U) & 0xFFU);
    out[2] = (uint8_t)((val >>  8U) & 0xFFU);
    out[3] = (uint8_t)( val         & 0xFFU);
}

/*
Read 4 big-endian wire bytes and return a host-order uint32_t.

*/
static inline uint32_t tlv_be_to_u32(const uint8_t be[4]) {
    return ((uint32_t)be[0] << 24U) |
           ((uint32_t)be[1] << 16U) |
           ((uint32_t)be[2] <<  8U) |
            (uint32_t)be[3]; // using math, the machine's endianness doesn't matter. It will always produce the correct host-order uint32_t. 一定是主機的endian
}

#endif