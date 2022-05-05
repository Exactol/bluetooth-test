#include <Arduino.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>

#include "common.h"
#include "bluetooth_commissioner.h"

// BLEDeviceEventHandler onBLEConnected(BluetoothCommissioner &commissioner) {
// 	// void onConnected(BLEDevice central) {
// 	// 	Serial.print("Connected event, central: ");
// 	// 	Serial.println(central.address());

// 	// 	for (auto service : commissioner->)
// 	// 		service->onBLEConnected();

// 	// 	pinMode(BLUE_LED, HIGH);
// 	// }

// 	return [commissioner](BLEDevice central) {
// 		Serial.print("Connected event, central: ");
// 		Serial.println(central.address());

// 		for (auto service : commissioner.services)
// 			service->onBLEConnected();

// 		pinMode(BLUE_LED, HIGH);
// 	};
// }
// void onBLEConnected(BLEDevice central) {
// 	Serial.print("Connected event, central: ");
// 	Serial.println(central.address());

// 	for (auto service : services)
// 		service->onBLEConnected();

// 	pinMode(BLUE_LED, HIGH);
// }

void BluetoothCommissioner::setupServices() {
	// setup characteristics
	for (auto service : services)
		service->registerAttributes();

	// setup event handlers
	// BLE.setEventHandler(BLEConnected, [=](BLEDevice central) {
	// 	Serial.print("Connected event, central: ");
	// 	Serial.println(central.address());

	// 	for (auto service : this->services)
	// 		service->onBLEConnected();

	// 	pinMode(BLUE_LED, HIGH);
	// });

	// BLE.setEventHandler(BLEDisconnected, [&](BLEDevice central) {
	// 	Serial.print("Disconnected event, central: ");
	// 	Serial.println(central.address());
	// 	// TODO: cleanup

	// 	for (auto service : services)
	// 		service->onBLEDisconnected();
	// });
}

void BluetoothCommissioner::registerServices() {
	for (auto service : services)
		service->registerService();
}

int BluetoothCommissioner::execute() {
	int result = 0;
	for (auto service : services) {
		result |= service->execute();
	}

	return result == 0 ? 0 : 1;
}

int BluetoothCommissioner::initialize() {
	int result = 0;
	for (auto service : services) {
		result |= service->initialize();
	}

	return result == 0 ? 0 : 1;
}

bool BluetoothCommissioner::isInitialized() {
	// makes sure every service is initialized
	for (auto service : services) {
		if (!service->isInitialized())
			return false;
	}

	return true;
}