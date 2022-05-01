#pragma once

extern const unsigned int BLE_MAX_CHARACTERISTIC_SIZE;
extern const int RED_LED;
extern const int GREEN_LED;
extern const int BLUE_LED;
extern const String MODEL;
extern const String SERIAL_NUMBER;
extern const String HARDWARE_REVISION;
extern const String FIRMWARE_REVISION;

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

namespace COMMISSIONING_STATE {
	enum {
		IDLE = 1,

		SAVE = 2,
		SAVING = 4,
		SAVED = 8,

		COMPLETE = 16,

		ERROR = 1024
	};
}

struct DeviceInfo
{
	uint32_t deviceId;
	String deviceToken;
	String name;
};
