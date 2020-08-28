#include "IRSender.hpp"

/**
 * 内容を初期化する。
 */
void IRSender::reset(uint32_t pinIR) {
  this->pinIR = pinIR;
  pinMode(this->pinIR, OUTPUT);
  digitalWrite(this->pinIR, LOW);

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
  memcpy(this->data + this->written, data, lengthToCopy);
  this->written += lengthToCopy;
  return lengthToCopy;
}

/**
 * 信号を送信する。
 */
void IRSender::send() {
  Serial.printf("Seinding Signal...\n");
  Serial.printf("Type: %d\n", this->type);
  Serial.printf("Customer Code: %04X\n", this->customerCode);
  Serial.printf("Data: ");
  for (size_t i = 0; i < this->written; ++i) {
    Serial.printf("%02X ", this->data[i]);
  }
  Serial.printf("\n");

  sendLeader();
  sendCustomerCode();
  sendData();
}

/**
 * リーダー信号を送信する。
 */
void IRSender::sendLeader() {
  switch (this->type) {
    case IRType::NEC: {
      for (int i = 0; i < 21 * 16; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(17);
      }
      delayMicroseconds(562 * 8);
      break;
    }
    case IRType::AEHA: {
      for (int i = 0; i < 16 * 8; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(17);
      }
      delayMicroseconds(425 * 4);
      break;
    }
    case IRType::Sony: {
      delayMicroseconds(600);
      for (int i = 0; i < 24; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(16);
      }
      break;
    }
  }
}

/**
 * カスタマーコードを送信する。
 */
void IRSender::sendCustomerCode() {
  if (this->type == IRType::Sony) {
    return;
  }

  for (size_t i = 0; i < 16; ++i) {
    if (this->customerCode & (1 << i)) {
      this->sendOne();
    } else {
      this->sendZero();
    }
  }

  if (this->type == IRType::AEHA) {
    uint8_t parity = (this->customerCode & 0x000F) ^
                     ((this->customerCode & 0x00F0) >> 4) ^
                     ((this->customerCode & 0x0F00) >> 8) ^
                     ((this->customerCode & 0xF000) >> 12);

    for (size_t i = 0; i < 4; ++i) {
      if (parity & (1 << i)) {
        this->sendOne();
      } else {
        this->sendZero();
      }
    }
  }
}

/**
 * データ部を送信する。
 */
void IRSender::sendData() {
  switch (this->type) {
    case IRType::NEC: {
      uint8_t data = this->data[0];
      for (size_t i = 0; i < 8; ++i) {
        if (data & (1 << i)) {
          this->sendOne();
        } else {
          this->sendZero();
        }
      }

      // flip
      uint8_t flipData = ~data;
      for (size_t i = 0; i < 8; ++i) {
        if (flipData & (1 << i)) {
          this->sendOne();
        } else {
          this->sendZero();
        }
      }
      break;
    }
    case IRType::AEHA: {
      for (size_t i = 0; i < this->written; ++i) {
        uint8_t sendingBits = i == 0 ? 4 : 8;
        for (size_t b = 0; b < sendingBits; ++b) {
          if (this->data[i] & (1 << b)) {
            this->sendOne();
          } else {
            this->sendZero();
          }
        }
      }

      // trailer
      this->sendOne();
      delay(10);
      break;
    }
    default:
      Serial.printf("Type %d is not implemented.\n", this->type);
      break;
  }
}

/**
 * ビット 0 を送信する。
 */
void IRSender::sendZero() {
  switch (this->type) {
    case IRType::NEC: {
      for (int i = 0; i < 21; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(17);
      }
      delayMicroseconds(562);
      break;
    }
    case IRType::AEHA: {
      for (int i = 0; i < 16; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(17);
      }
      delayMicroseconds(425);
      break;
    }
    case IRType::Sony: {
      delayMicroseconds(600);
      for (int i = 0; i < 24; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(16);
      }
      break;
    }
  }
}

/**
 * ビット 1 を送信する。
 */
void IRSender::sendOne() {
  switch (this->type) {
    case IRType::NEC: {
      for (int i = 0; i < 21; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(17);
      }
      delayMicroseconds(562 * 3);
      break;
    }
    case IRType::AEHA: {
      for (int i = 0; i < 16; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(17);
      }
      delayMicroseconds(425 * 3);
      break;
    }
    case IRType::Sony: {
      delayMicroseconds(600);
      for (int i = 0; i < 24 * 2; ++i) {
        digitalWrite(this->pinIR, HIGH);
        delayMicroseconds(9);
        digitalWrite(this->pinIR, LOW);
        delayMicroseconds(16);
      }
      break;
    }
  }
}
