#include <Arduino.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>
#include <vector>

#include "wifi.h"
#include "../common.h"
#include "../services/misc_services.h"

// BatteryService batteryService;
std::vector<FlourishService> services = { BatteryService() };

void onBLEConnected(BLEDevice central) {
	Serial.print("Connected event, central: ");
	Serial.println(central.address());

	// commissioningState.writeValue(COMMISSIONING_STATE::IDLE);
	// wifiState.writeValue(WIFI_COMMISSIONING_STATE::IDLE);
	pinMode(BLUE_LED, HIGH);
}

void onBLEDisconnected(BLEDevice central) {
	Serial.print("Disconnected event, central: ");
	Serial.println(central.address());
	// TODO: cleanup
}

void initializeServices() {
	// setup characteristics
	for (auto service : services) {
	 	Serial.println("HERE");
		service.initialize();
	}
	// commissioningService.addCharacteristic(commissioningState);
	// commissioningService.addCharacteristic(commissioningDeviceID);
	// commissioningService.addCharacteristic(commissioningDeviceToken);
	// commissioningService.addCharacteristic(commissioningDeviceName);
	// commissioningDeviceName.setEventHandler(BLEWritten, [](BLEDevice device, BLECharacteristic characteristic) {
	// 	// update advertised localname if name is updated
	// 	Serial.println("Name updated to " + commissioningDeviceName.value());
	// 	BLE.setLocalName(commissioningDeviceName.value().c_str());
	// 	BLE.advertise();
	// });

	// deviceInformationService.addCharacteristic(deviceManufacturerName);
	// deviceInformationService.addCharacteristic(deviceModelNumber);
	// deviceInformationService.addCharacteristic(deviceSerialNumber);
	// deviceInformationService.addCharacteristic(deviceHardwareRevision);
	// deviceInformationService.addCharacteristic(deviceFirmwareRevision);

	// initializeWifiServices();

	// commissioningState.writeValue(COMMISSIONING_STATE::IDLE);

	// deviceManufacturerName.writeValue("Flourish");
	// deviceModelNumber.writeValue(MODEL);
	// deviceSerialNumber.writeValue(SERIAL_NUMBER);
	// deviceHardwareRevision.writeValue(HARDWARE_REVISION);
	// deviceFirmwareRevision.writeValue(FIRMWARE_REVISION);

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
}

void registerBleServices() {
	for (auto service : services)
		service.registerService();
	// BLE.addService(commissioningService);
	// BLE.addService(deviceInformationService);
}

void startBle() {
	// stop wifi
	Serial.println("Stopping WiFi");
	WiFi.end();

	Serial.println("Initializing BLE");

	// initialize BLE
	// if (!BLE.begin()) {
	// 	Serial.println("Failed to start BLE");
	// 	while (1);
	// }
	while (!BLE.begin()) {
		Serial.println("Failed to start BLE");
		delay(5000);
	}

	// TODO: user configurable name?
	BLE.setLocalName("Flourish Device");
	BLE.setDeviceName("Flourish Device");

	// BLE.setAdvertisedService(commissioningService);

	BLE.setAppearance(0x0540); // set appearance to Generic Sensor (from BLE appearance values)

	registerBleServices();
	// registerWifiServices();

	BLE.advertise();
	Serial.println("BLE Initialized");
}

void executeServices() {
	for (auto service : services)
		service.execute();
}