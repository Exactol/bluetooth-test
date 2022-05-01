#include <Arduino.h>
#include <ArduinoBLE.h>
#include <WiFiNINA.h>

#include "wifi.h"
#include "../common.h"

BLEService commissioningService("00000000-1254-4046-81d7-676ba8909661");
BLEIntCharacteristic commissioningState("00000001-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
BLEUnsignedIntCharacteristic commissioningDeviceID("00000002-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic commissioningDeviceToken("00000003-1254-4046-81d7-676ba8909661", BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
BLEStringCharacteristic commissioningDeviceName("00000004-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);

void onBLEConnected(BLEDevice central) {
	Serial.print("Connected event, central: ");
	Serial.println(central.address());

	commissioningState.writeValue(COMMISSIONING_STATE::IDLE);
	wifiState.writeValue(WIFI_COMMISSIONING_STATE::IDLE);

	batteryPercentage.writeValue(80);
	pinMode(BLUE_LED, HIGH);
}

void onBLEDisconnected(BLEDevice central) {
	Serial.print("Disconnected event, central: ");
	Serial.println(central.address());
	// TODO: cleanup
}

void initializeServices() {
	// setup characteristics
	commissioningService.addCharacteristic(commissioningState);
	commissioningService.addCharacteristic(commissioningDeviceID);
	commissioningService.addCharacteristic(commissioningDeviceToken);
	commissioningService.addCharacteristic(commissioningDeviceName);
	commissioningDeviceName.setEventHandler(BLEWritten, [](BLEDevice device, BLECharacteristic characteristic) {
		// update advertised localname if name is updated
		Serial.println("Name updated to " + commissioningDeviceName.value());
		BLE.setLocalName(commissioningDeviceName.value().c_str());
		BLE.advertise();
	});

	batteryService.addCharacteristic(batteryPercentage);

	deviceInformationService.addCharacteristic(deviceManufacturerName);
	deviceInformationService.addCharacteristic(deviceModelNumber);
	deviceInformationService.addCharacteristic(deviceSerialNumber);
	deviceInformationService.addCharacteristic(deviceHardwareRevision);
	deviceInformationService.addCharacteristic(deviceFirmwareRevision);

	initializeWifiServices();

	commissioningState.writeValue(COMMISSIONING_STATE::IDLE);

	deviceManufacturerName.writeValue("Flourish");
	deviceModelNumber.writeValue(MODEL);
	deviceSerialNumber.writeValue(SERIAL_NUMBER);
	deviceHardwareRevision.writeValue(HARDWARE_REVISION);
	deviceFirmwareRevision.writeValue(FIRMWARE_REVISION);

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
}

void registerBleServices() {
	BLE.addService(commissioningService);
	BLE.addService(batteryService);
	BLE.addService(deviceInformationService);
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

	// TODO: user configurable name?
	BLE.setLocalName("Flourish Device");
	BLE.setDeviceName("Flourish Device");

	BLE.setAdvertisedService(commissioningService);

	BLE.setAppearance(0x0540); // set appearance to Generic Sensor (from BLE appearance values)

	registerBleServices();
	registerWifiServices();

	BLE.advertise();
	Serial.println("BLE Initialized");
}