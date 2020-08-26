#pragma once

#include <functional>

#include <Arduino.h>
#include <Seeed_FS.h>
#include <Seeed_SD.h>

void sdInitialize();
void sdListFiles(std::function<void(File*)> callback);
