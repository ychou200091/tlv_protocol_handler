/**
 ***************************************************
 * @file    main.c
 * @author  Taylor Chou
 * @version 1.0.0
 * @date    2026-03-05
 * @brief   TLV (Type-Length-Value) Protocol Parser and Serializer
 *
 * ==========================================
 * 描述
 * ==========================================
 * 本模組提供 TLV 協議的解析與序列化功能，支援多種資料型態的編碼與解碼。
 * 適用於嵌入式設備通訊、無線協議、以及任何需要輕量級結構化資料傳輸的場景。
 *
 * 功能：
 *   - 逐個解析 TLV 節點，並自動跳至下一個節點
 *   - 將資料依照 TLV 格式輸入到緩衝區
 *   - 零複製 (Zero-Copy) 方式指向原始資料，降低記憶體開銷
 *
 * ==========================================
 * DESCRIPTION
 * ==========================================
 * This module provides TLV (Type-Length-Value) parsing and serialization for embedded systems, wireless protocols, and lightweight structured data transfer.
 *
 * Features:
 *   - Parse TLV nodes sequentially with automatic offset tracking
 *   - Serialize data into TLV format and write to buffer
 *   - Zero-copy design pointing to original data for minimal memory overhead
 *
 * ==========================================
 * USAGE EXAMPLE (使用範例)
 * ==========================================
 * Input Packet (46 bytes):
 *   01 0B 4A 61 73 6F 6E 20 48 61 75 6E 67  02 04 00 00 D4 31
 *   04 03 52 26 44  05 01 1E  06 11 48 65 6C 6C 6F 20 54 61 69 77 61 6E 20 32 30 32 36
 *
 * Parse Output:
 *   Type=0x01 (Name),       Length=11,  Value="Jason Haung"
 *   Type=0x02 (ID),         Length=4,   Value=54321
 *   Type=0x04 (Department), Length=3,   Value="R&D"
 *   Type=0x05 (Age),        Length=1,   Value=30
 *   Type=0x06 (Note),       Length=17,  Value="Hello Taiwan 2026"
 *
 * ***************************************************
 */

#include <stdio.h>
#include <locale.h>
#include "tlv_parser.h"

#ifdef _WIN32
#include <windows.h>
#endif

static void init_runtime(void) {
#ifdef _WIN32
    /* Windows console: enable UTF-8 code page */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    /* Prefer system locale; fallback to "C" */
    if (setlocale(LC_ALL, "") == NULL) {
        (void)setlocale(LC_ALL, "C");
    }

    /* 偵測並印出 host byte order，在主邏輯執行前確認平台字節序
     * Detect and report host byte order before any protocol logic runs. */
    printf("[Init] Host byte order: %s-endian\n",
           host_is_le() ? "Little" : "Big");
}

static void print_hex(const uint8_t *data, uint8_t len) {
	uint8_t i;
	for (i = 0U; i < len; ++i) {
		printf("%02X", data[i]);
		if ((uint8_t)(i + 1U) < len) {
			printf(" ");
		}
	}
}

static void print_value(const tlv_node_t *node) {
	tlv_value_kind_t kind = tlv_type_value_kind(node->type);

	switch (kind) {
		case TLV_VALUE_KIND_TEXT_UTF8:
			printf("\"");
			fwrite(node->value, 1U, node->length, stdout);
			printf("\"");
			break;
		case TLV_VALUE_KIND_UINT8:
			if (node->length == 1U) {
				printf("%u", (unsigned int)node->value[0]);
			} else {
				print_hex(node->value, node->length);
			}
			break;
		case TLV_VALUE_KIND_UINT32_BE:
			if (node->length == 4U) {
				// 從 wire 讀出 big-endian bytes，還原成 host-order 整數後印出Read big-endian wire bytes back to a host-order integer.
				printf("%u", (unsigned int)tlv_be_to_u32(node->value));
			} else {
				print_hex(node->value, node->length);
			}
			break;
		case TLV_VALUE_KIND_BINARY:
		default:
			print_hex(node->value, node->length);
			break;
	}
}

int main(void) {
	uint8_t packet[256];
	size_t used = 0U;
	size_t written = 0U;
	size_t offset = 0U;
	tlv_node_t node;
	tlv_status_t status;

	const uint8_t name[] = "Jason Haung";
	const uint32_t id_val = 543210U; 
	uint8_t id_be[4];  // will hold big-endian wire bytes
	                                
	const uint8_t department[] = "R&D";
	const uint8_t age[1] = {30U};
	const uint8_t note[] = "Hello Taiwan 2026";

	init_runtime(); 

    // 寫入buffer1 name
	status = tlv_serialize_one(TLV_TYPE_NAME, (uint8_t)(sizeof(name) - 1U), name,packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;
    
	// 寫入buffer2 id
	tlv_u32_to_be(id_val, id_be); // Convert uint32 to big-endian wire bytes
	// 寫入buffer時採用 big-endian bytes，確保跨平台一致性。
	status = tlv_serialize_one(TLV_TYPE_ID, (uint8_t)sizeof(id_be), id_be,packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;

    // 寫入buffer3 department
	status = tlv_serialize_one(TLV_TYPE_DEPARTMENT, (uint8_t)(sizeof(department) - 1U), department, packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;

    // 寫入buffer4 age
	status = tlv_serialize_one(TLV_TYPE_AGE, (uint8_t)sizeof(age), age,packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;

    // 寫入buffer5 note
	status = tlv_serialize_one(TLV_TYPE_NOTE, (uint8_t)(sizeof(note) - 1U), note,packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;
    
    // 印出整個 packet 的內容 (hex)
	printf("Total Packet Length: %u bytes\n", (unsigned int)used);
    for(int i = 0; i < (int)used; ++i) {
        printf("%02X ", packet[i]);
    }
    
    // 從 packet 開頭開始解析，直到解析完buffer內所有 bytes
	while (offset < used) {
		status = tlv_parse_next(packet, used, &offset, &node);
		if (status != TLV_SUCCESS) {
			printf("Parse failed: %d\n", (int)status);
			return 1;
		}

		  printf("Type=0x%02X (%s), Length=%u, Value=",
			  node.type,
			  tlv_type_name(node.type),
			  (unsigned int)node.length);
		print_value(&node);
		printf("\n");
	}

	return 0;
}
