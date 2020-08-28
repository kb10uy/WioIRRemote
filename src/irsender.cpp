#include "IRSender.hpp"

/**
 * 内容を初期化する。
 */
void IRSender::reset() {
  this->type = IRType::NEC;
  this->customerCode = 0x0000;
  this->written = 0;
}

/**
 * 送出する信号のタイプを設定する。
 */
void IRSender::setType(IRType type) {
  this->type = type;
  this->written = 0;
}

/**
 * カスタマーコードを設定する。
 * Sony の場合は無視される。
 */
void IRSender::setCustomerCode(uint16_t customerCode) {
  this->customerCode = customerCode;
}

/**
 * 1byte データを追加する。
 * AEHA の場合、先頭 1byte の上位 4bit は無視される。
 */
size_t IRSender::pushData(uint8_t data) {
  if (this->written >= 256) {
    return 0;
  } else {
    this->data[this->written++] = data;
    return 1;
  }
}

/**
 * 複数のデータを追加する。
 * AEHA の場合、先頭 1byte の上位 4bit は無視される。
 */
size_t IRSender::pushData(uint8_t *data, size_t length) {
  if (this->written >= 256) {
    return 0;
  }

  size_t lengthToCopy = min(length, 256 - this->written);
  memcpy(this->data, data, lengthToCopy);
  this->written += lengthToCopy;
  return lengthToCopy;
}

void IRSender::send() {}
