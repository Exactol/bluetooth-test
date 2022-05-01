#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>

#include "utility/wifi_drv.h"

#include "src/communication/wifi.h"
#include "src/communication/ble.h"
#include "src/common.h"

FlashStorage(deviceStorage, DeviceInfo);

int device_state = DEVICE_STATE::IDLE;

void setupCommissioning() {
	Serial.println("Setting up commissioning");

	initializeServices();
	device_state = DEVICE_STATE::COMMISSIONING;

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

	// check if device has been commissioned already
	DeviceInfo deviceInfo = deviceStorage.read();
	if (deviceInfo.deviceId == 0 && deviceInfo.name == NULL) {
		setupCommissioning();
	} else {
		Serial.println("Device Information");
		Serial.println("Name: " + deviceInfo.name);
		Serial.println("ID: " + String( deviceInfo.deviceId ));

		// read WiFi info from storage
		// WiFiInfo wifiInfo = wifiStorage.read();
		// Serial.println("WiFi SSID: " + wifiInfo.ssid);
	}

	Serial.println("Setup complete");
}

int saveDeviceInformation() {
	Serial.println("Saving information for device " + String(commissioningDeviceID.value()));

	if (commissioningDeviceID.value() == -1) {
		Serial.println("Failed to save information, device ID null");
		return -1;
	}

	DeviceInfo info = {
		commissioningDeviceID.value(),
		commissioningDeviceToken.value(),
		commissioningDeviceName.value(),
	};

	deviceStorage.write(info);
	Serial.println("Device Information Saved");

	return 0;
}

int commissionDevice() {
	if (commissioningState.written()) {
		switch (commissioningState.value())
		{
			case COMMISSIONING_STATE::SAVE:
				saveDeviceInformation();
				break;

			case COMMISSIONING_STATE::COMPLETE:
				// TODO: start wifi and stuff
				device_state = DEVICE_STATE::COMMISSIONED;
				break;

			case COMMISSIONING_STATE::ERROR:
				digitalWrite(RED_LED, HIGH);
				break;

			// idle
			default:
				break;
		}
	}
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
					commissionDevice();
					commissionWifi();
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