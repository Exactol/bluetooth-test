#include <Arduino.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>

#include "common.h"
#include "bluetooth_commissioner.h"
#include "communication/ble.h"

void onBLEConnected(BLEDevice central) {
	Serial.print("Connected event, central: ");
	Serial.println(central.address());

	for (auto service : commissioner.getServices())
		service->onBLEConnected();

	pinMode(BLUE_LED, HIGH);
}

void onBLEDisconnected(BLEDevice central) {
		Serial.print("Disconnected event, central: ");
		Serial.println(central.address());
		// TODO: cleanup

		for (auto service : commissioner.getServices())
			service->onBLEDisconnected();
}

void BluetoothCommissioner::setupServices() {
	// setup characteristics
	for (auto service : services)
		service->registerAttributes();

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
}

void BluetoothCommissioner::registerServices() {
	for (auto service : services)
		service->registerService();
}

void BluetoothCommissioner::startCommissioning() {
	Serial.println("Setting up commissioning");

	this->setupServices();
	deviceState = DEVICE_STATE::COMMISSIONING;
	// start BLE will register the services
	startBle();

	Serial.println("Commissioning setup complete");
}

int BluetoothCommissioner::completeCommissioning() {
	Serial.println("Commissioning complete");

	this->initialize();

	if (this->isInitialized()) {
		setupDevice();
		return 0;
	}

	return 1;
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