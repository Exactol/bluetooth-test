#pragma once
#include <Arduino.h>

static const unsigned int BLE_MAX_CHARACTERISTIC_SIZE = 512;

const int RED_LED = 2;
const int GREEN_LED = 3;
const int BLUE_LED = 4;

const String MODEL = "Flourish Device";
const String SERIAL_NUMBER = "abcde";
const String HARDWARE_REVISION = "0.1";
const String FIRMWARE_REVISION = "0.1";

enum class FLOURISH_EXCEPTION {
	NO_WIFI_SSID
};

namespace DEVICE_STATE {
	enum {
		IDLE = 1,
		COMMISSIONING = 2,
		COMMISSIONED = 4,
		ERROR = 8
	};
}


struct DeviceInfo
{
	uint32_t deviceId;
	String deviceToken;
	String name;
};
