#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>
#include <vector>

#include "src/common.h"
#include "src/communication/ble.h"
#include "src/bluetooth_commissioner.h"
#include "src/services/commissioning_service.h"
#include "src/services/misc_services.h"
#include "src/services/commissioning_service.h"
#include "src/services/wifi_service.h"

int deviceState = DEVICE_STATE::IDLE;
auto commissioner = BluetoothCommissioner({new CommissioningService(COMMISSIONING_DEVICE_TYPE::SENSOR), new WiFiService(), new BatteryService(), new DeviceInformationService()});

void setupCommissioning() {
	Serial.println("Setting up commissioning");

	setupServices();
	deviceState = DEVICE_STATE::COMMISSIONING;
	startBle();

	Serial.println("Commissioning setup complete");
}

int completeCommissioning() {
	Serial.println("Commissioning complete");

	initializeServices();

	if (servicesInitialized()) {
		setupDevice();
		return 0;
	}

	return 1;
}

void setupDevice() {
	Serial.println("All services initialized, setting up device");
	deviceState = DEVICE_STATE::COMMISSIONED;
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

	commissioner.initialize();

	if (commissioner.isInitialized()) {
		setupDevice();
	} else {
		setupCommissioning();
	}

	Serial.println("Setup complete");
}

void loop()
{
	switch (deviceState)
	{
		case DEVICE_STATE::COMMISSIONING: {
			BLEDevice central = BLE.central();

			digitalWrite(BLUE_LED, HIGH);

			if (central) {
				while (central.connected()) {
					commissioner.execute();
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