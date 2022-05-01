#pragma once
#include <ArduinoBLE.h>

extern BLEService wifiService;
extern BLEIntCharacteristic wifiState;

namespace WIFI_COMMISSIONING_STATE {
	enum {
		IDLE = 1,

		SCAN = 2,
		SCANNING = 4,
		SCANNED = 8,

		JOIN = 16,
		JOINING = 32,
		JOINED = 64,

		SAVE = 128,
		SAVING = 256,
		SAVED = 512,

		ERROR = 1024
	};
}

struct WiFiInfo
{
	String ssid;
	String password;
};

void initializeWifiServices();

int commissionWifi();

void registerWifiServices();