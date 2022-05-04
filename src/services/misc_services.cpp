#include <Arduino.h>
#include <ArduinoBLE.h>
#include "../common.h"
#include "misc_services.h"

BLEService batteryService("180F");
BLEByteCharacteristic batteryPercentage("2A19", BLERead);

void BatteryService::registerService() {
	Serial.println("Registering Battery Service");
	BLE.addService(batteryService);
}

void BatteryService::registerAttributes() {
	Serial.println("Registering Battery Service Attributes");
	batteryService.addCharacteristic(batteryPercentage);
	// TODO: actual battery readings
	batteryPercentage.writeValue(80);
}



BLEService deviceInformationService("180A");
BLEStringCharacteristic deviceManufacturerName("2A29", BLERead, 20);
BLEStringCharacteristic deviceModelNumber("2A24", BLERead, 20);
BLEStringCharacteristic deviceSerialNumber("2A25", BLERead, 20);
BLEStringCharacteristic deviceHardwareRevision("2A27", BLERead, 20);
BLEStringCharacteristic deviceFirmwareRevision("2A26", BLERead, 20);

void DeviceInformationService::registerService() {
	Serial.println("Registering Device Information Service");
	BLE.addService(deviceInformationService);
}

void DeviceInformationService::registerAttributes() {
	Serial.println("Registering Device Information Service Attributes");
	deviceInformationService.addCharacteristic(deviceManufacturerName);
	deviceInformationService.addCharacteristic(deviceModelNumber);
	deviceInformationService.addCharacteristic(deviceSerialNumber);
	deviceInformationService.addCharacteristic(deviceHardwareRevision);
	deviceInformationService.addCharacteristic(deviceFirmwareRevision);

	deviceManufacturerName.writeValue("Flourish");
	deviceModelNumber.writeValue(MODEL);
	deviceSerialNumber.writeValue(SERIAL_NUMBER);
	deviceHardwareRevision.writeValue(HARDWARE_REVISION);
	deviceFirmwareRevision.writeValue(FIRMWARE_REVISION);
}