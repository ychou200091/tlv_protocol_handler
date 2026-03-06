# 專案需求文件 (PRD)：輕量級高效能 TLV 協定解析器

## 專案背景與目標

在嵌入式系統或網路設備之間傳輸資料時，我們需要一種彈性且節省空間的格式。TLV 是一種編碼方案，允許在不破壞相容性的情況下，於資料流中加入新的資訊。

本專案目標是使用 C 實作一個穩健的解析器，能夠將二進制流 (Byte Stream) 轉換為結構化的資料；同時手刻主機的endianess檢查器，展現對於底層知識的學習意願與了解。

---

## TLV 格式定義

* **Type (T):** 這是什麼？（佔 1 Byte）例如：1 代表用戶名，2 代表 IP 地址。
* **Length (L):** 這東西有多長？（佔 1 Byte）例如：10 代表後面的資料有 10 個 Bytes。
* **Value (V):** 實際的內容。

### 範例數據：

假設我們要傳輸：`Name: "Taiwan98765"`, `ID: 54321`

* Type: 0x01 (Name), Length: 0x0B, Value: `Taiwan98765`
* Type: 0x02 (ID), Length: 0x04, Value: 0x0000D431

**完整原始數據 (Hex):** `01 0B 54 61 69 77 61 6E 39 38 37 36 35 02 04 00 00 D4 31`

---

## 功能需求 (Functional Requirements)

- 本專案內採用`uint8_t`類型的 Buffer，會將資料寫入buffer或是從buffer取出資料。
- 所有多位元組整數欄位皆以 big-endian 存放於 buffer。
- Zero-copy：解析時 value 欄位永遠指向原始 buffer，不會 malloc。

### A. 解析功能 (Parsing)

* 輸入：一段 `uint8_t` 類型的 Buffer。
* 輸出：逐一取出每個 TLV 節點，並識別其類型與長度。
* **技術挑戰：** 必須處理「Buffer 結尾預期外中斷」的錯誤（防止 Memory Leak 或 Segmentation Fault）。

### B. 序列化功能 (Serialization)

* 提供 API 讓使用者能手動「組裝」TLV 封包。
* 輸入：一份資料的 type, length, value
* 輸出：一段 `uint8_t` 類型的 Buffer。
* **技術挑戰：** 確保資料類性的擴充性，以及依照裝置Endianess讀取資料並以Big-Endian的格式寫入Buffer。
    * 為什麼統一用Big-Endian：因為符合 network byte order；幾乎所有網路協定（如 TCP/IP）都規定多位元組整數必須用 big-endian（高位在前，低位在後）格式傳輸。

---

## 非功能需求 (Non-functional Requirements)

* **Zero-copy (進階技術點):** 解析時不要額外 malloc 空間存 Value，而是直接指向原始 Buffer 的位址，這對 Switch 效能至關重要。
* **平台無關性:** 不使用特定作業系統的 Library，確保程式碼能在嵌入式環境執行。

---

## API 設計範例 (C Language)

```c
// 定義傳回值的狀態碼
typedef enum {
    TLV_SUCCESS,
    TLV_ERR_BUFFER_TOO_SHORT,
    TLV_ERR_INVALID_LENGTH
} tlv_status_t;

// TLV 結構體
typedef struct {
    uint8_t type;
    uint8_t length;
    const uint8_t *value; // 指向原始數據(Zero-copy)
} tlv_node_t;

// 解析資訊的API
tlv_status_t tlv_parse_next(const uint8_t *buffer, size_t buf_len, size_t *offset, tlv_node_t *out_node);
// 序列化資訊為TLV格式並存入buffer的API
tlv_status_t tlv_serialize_one(uint8_t type, uint8_t length,const uint8_t *value, uint8_t *out_buf, size_t out_len, size_t *written);

```
---
## 目錄架構
1. tlv_def.h: 專門放常數定義（Macro）、列舉（Enum）。
2. tlv_parser.h: 函式原型（Function Prototype）宣告。
3. tlv_parser.c: 解析邏輯實作。
4. main.c: 測試與展示。
