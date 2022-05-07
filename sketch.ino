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

BluetoothCommissioner commissioner = BluetoothCommissioner({new CommissioningService(COMMISSIONING_DEVICE_TYPE::SENSOR), new WiFiService(), new BatteryService(), new DeviceInformationService()});

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
		commissioner.startCommissioning();
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