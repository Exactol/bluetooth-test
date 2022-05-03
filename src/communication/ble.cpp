#include <Arduino.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>
#include <vector>

#include "wifi.h"
#include "../common.h"
#include "../services/base_service.h"
#include "../services/misc_services.h"
#include "../services/commissioning_service.h"
#include "../services/wifi_service.h"

std::vector<BaseService*> services = { new CommissioningService(COMMISSIONING_DEVICE_TYPE::SENSOR), new WiFiService(), new BatteryService(), new DeviceInformationService(), };

void onBLEConnected(BLEDevice central) {
	Serial.print("Connected event, central: ");
	Serial.println(central.address());

	for (auto service : services)
		service->onBLEConnected();

	pinMode(BLUE_LED, HIGH);
}

void onBLEDisconnected(BLEDevice central) {
	Serial.print("Disconnected event, central: ");
	Serial.println(central.address());
	// TODO: cleanup

	for (auto service : services)
		service->onBLEDisconnected();
}

void setupServices() {
	// setup characteristics
	for (auto service : services)
		service->registerAttributes();

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
}

void registerBleServices() {
	for (auto service : services)
		service->registerService();
}

void startBle() {
	// stop wifi
	Serial.println("Stopping WiFi");
	WiFi.end();

	Serial.println("Initializing BLE");

	// initialize BLE
	if (!BLE.begin()) {
		Serial.println("Failed to start BLE");
		while (1);
	}

	BLE.setLocalName("Flourish Device");
	BLE.setDeviceName("Flourish Device");
	BLE.setAppearance(0x0540); // set appearance to Generic Sensor (from BLE appearance values)

	registerBleServices();

	BLE.advertise();
	Serial.println("BLE Initialized");
}

void executeServices() {
	for (auto service : services) {
		service->execute();
	}
}

int initializeServices() {
	for (auto service : services) {
		service->initialize();
	}
}

bool servicesInitialized() {
	// makes sure every service is initialized
	for (auto service : services) {
		if (!service->isInitialized())
			return false;
	}

	return true;
}