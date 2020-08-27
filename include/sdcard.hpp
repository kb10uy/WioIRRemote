#pragma once

#include <Arduino.h>
#include <Seeed_FS.h>
#include <Seeed_SD.h>

void sdInitialize();
void sdListFiles(void (*callback)(File*));
