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
				unsigned int number = ((unsigned int)node->value[0] << 24U) |
									 ((unsigned int)node->value[1] << 16U) |
									 ((unsigned int)node->value[2] << 8U) |
									 (unsigned int)node->value[3];
				printf("%u", number);
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
	const uint8_t id_be[4] = {0x00U, 0x00U, 0xD4U, 0x31U}; // 54321 的 big-endian 表示
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
	status = tlv_serialize_one(TLV_TYPE_ID, (uint8_t)sizeof(id_be), id_be,
						   packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;

    // 寫入buffer3 department
	status = tlv_serialize_one(TLV_TYPE_DEPARTMENT, (uint8_t)(sizeof(department) - 1U), department,
						   packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;

    // 寫入buffer4 age
	status = tlv_serialize_one(TLV_TYPE_AGE, (uint8_t)sizeof(age), age,
						   packet + used, sizeof(packet) - used, &written);
	if (status != TLV_SUCCESS) {
		printf("Serialize failed: %d\n", (int)status);
		return 1;
	}
	used += written;

    // 寫入buffer5 note
	status = tlv_serialize_one(TLV_TYPE_NOTE, (uint8_t)(sizeof(note) - 1U), note,
						   packet + used, sizeof(packet) - used, &written);
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
