#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>

#include "src/common.h"
#include "src/communication/ble.h"

int device_state = DEVICE_STATE::IDLE;

void setupCommissioning() {
	Serial.println("Setting up commissioning");

	setupServices();
	device_state = DEVICE_STATE::COMMISSIONING;
	startBle();

	Serial.println("Commissioning setup complete");
}

void setupDevice() {
	Serial.println("All services initialized, setting up device");
	device_state = DEVICE_STATE::COMMISSIONED;
	digitalWrite(BLUE_LED, LOW);
	digitalWrite(GREEN_LED, HIGH);
}

void setup()
{
	Serial.begin(9600);
	while (!Serial);

	// initialize pins
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);
	pinMode(BLUE_LED, OUTPUT);

	initializeServices();

	if (servicesInitialized()) {
		setupDevice();
	} else {
		setupCommissioning();
	}

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
				}
			}

			delay(500);
			digitalWrite(BLUE_LED, LOW);
			delay(500);
			break;
		}
		case DEVICE_STATE::COMMISSIONED:
			Serial.println("Commisioned loop");
			delay(5000);
			break;
		default:
			break;
	}
}