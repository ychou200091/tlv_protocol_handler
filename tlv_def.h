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
#endif