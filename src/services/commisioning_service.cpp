#include <Arduino.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>
#include "../common.h"
#include "commissioning_service.h"

BLEService commissioningService("00000000-1254-4046-81d7-676ba8909661");
BLEUnsignedShortCharacteristic commissioningState("00000001-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
BLEUnsignedIntCharacteristic commissioningDeviceID("00000002-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic commissioningDeviceToken("00000003-1254-4046-81d7-676ba8909661", BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
BLEStringCharacteristic commissioningDeviceName("00000004-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
BLEByteCharacteristic commissioningDeviceType("00000005-1254-4046-81d7-676ba8909661", BLERead | BLEIndicate);

CommissioningService::CommissioningService(int deviceType) {
	this->deviceType = deviceType;
}

void CommissioningService::registerService() {
	Serial.println("Registering Commisioning Service");
	BLE.addService(commissioningService);
	BLE.setAdvertisedService(commissioningService);
}

void CommissioningService::registerAttributes() {
	Serial.println("Registering Commisioning Service Attributes");
	commissioningService.addCharacteristic(commissioningState);
	commissioningService.addCharacteristic(commissioningDeviceID);
	commissioningService.addCharacteristic(commissioningDeviceToken);
	commissioningService.addCharacteristic(commissioningDeviceName);
	commissioningService.addCharacteristic(commissioningDeviceType);

	commissioningDeviceName.setEventHandler(BLEWritten, [](BLEDevice device, BLECharacteristic characteristic) {
		// update advertised localname if name is updated
		Serial.println("CommisioningService: Name updated to " + commissioningDeviceName.value());
		BLE.setLocalName(commissioningDeviceName.value().c_str());
		BLE.advertise();
	});

	commissioningState.writeValue(COMMISSIONING_STATE::IDLE);
	commissioningDeviceType.writeValue(deviceType);
}

FlashStorage(deviceStorage, DeviceInfo);

int CommissioningService::saveDeviceInformation() {
	Serial.println("Saving information for device " + String(commissioningDeviceID.value()));

	if (commissioningDeviceID.value() == 0) {
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

int CommissioningService::initialize() {
	Serial.println("Initializing Commissioning Service");
	Serial.println("Loading Device Information");
	deviceInfo = deviceStorage.read();

	if (isInitialized()) {
		Serial.println("Loaded Device Information");
		Serial.println("Name: " + deviceInfo.name);
		Serial.println("ID: " + String( deviceInfo.deviceId ));
		// TODO: setup device
	}

	Serial.println("Commissioning Service Initialized");
	return 0;
}

bool CommissioningService::isInitialized() {
	// TODO: add token
	return deviceInfo.deviceId != 0;
}

int CommissioningService::execute() {
	if (!commissioningState.written()) {
		return 0;
	}

	switch (commissioningState.value())
	{
		case COMMISSIONING_STATE::SAVE:
			commissioningState.writeValue(COMMISSIONING_STATE::SAVING);
			if (!saveDeviceInformation()) {
				commissioningState.writeValue(COMMISSIONING_STATE::ERROR);
			}
			commissioningState.writeValue(COMMISSIONING_STATE::SAVED);
			break;

		case COMMISSIONING_STATE::COMPLETE:
			// TODO: start wifi and stuff
			// device_state = DEVICE_STATE::COMMISSIONED;
			break;

		case COMMISSIONING_STATE::ERROR:
			digitalWrite(RED_LED, HIGH);
			break;

		// idle
		default:
			break;
	}

	return 0;
}
