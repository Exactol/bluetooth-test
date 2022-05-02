#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>

#include "utility/wifi_drv.h"

// #include "src/communication/wifi.h"
#include "src/common.h"
#include "src/communication/ble.h"
// #include "src/services/wifi_service.h"
// #include "src/services/flourish_service.h"

int device_state = DEVICE_STATE::IDLE;

// BatteryService batteryService;

void setupCommissioning() {
	Serial.println("Setting up commissioning");

	initializeServices();

	// batteryService.initialize();
	device_state = DEVICE_STATE::COMMISSIONING;
	startBle();

	Serial.println("Commissioning setup complete");
}

void setup()
{
	Serial.begin(9600);
	while (!Serial);

	// initialize pins
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);
	pinMode(BLUE_LED, OUTPUT);

	setupCommissioning();

	// check if device has been commissioned already
	// DeviceInfo deviceInfo = deviceStorage.read();
	// if (deviceInfo.deviceId == 0 && deviceInfo.name == NULL) {
	// 	setupCommissioning();
	// } else {
	// 	Serial.println("Device Information");
	// 	Serial.println("Name: " + deviceInfo.name);
	// 	Serial.println("ID: " + String( deviceInfo.deviceId ));

	// 	// read WiFi info from storage
	// 	// WiFiInfo wifiInfo = wifiStorage.read();
	// 	// Serial.println("WiFi SSID: " + wifiInfo.ssid);
	// }

	Serial.println("Setup complete");
}

void loop()
{
	switch (device_state)
	{
		case DEVICE_STATE::COMMISSIONING: {
			BLEDevice central = BLE.central();

			digitalWrite(BLUE_LED, HIGH);

			if (central) {
				while (central.connected()) {
					executeServices();
					// commissionDevice();
					// commissionWifi();

				}
			}

			delay(500);
			digitalWrite(BLUE_LED, LOW);
			delay(500);
			break;
		}

		default:
			break;
	}
}