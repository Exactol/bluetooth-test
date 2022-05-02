#include <Arduino.h>
#include <ArduinoBLE.h>
#include "flourish_service.h"

BLEService deviceInformationService("180A");
BLEStringCharacteristic deviceManufacturerName("2A29", BLERead, 20);
BLEStringCharacteristic deviceModelNumber("2A24", BLERead, 20);
BLEStringCharacteristic deviceSerialNumber("2A25", BLERead, 20);
BLEStringCharacteristic deviceHardwareRevision("2A27", BLERead, 20);
BLEStringCharacteristic deviceFirmwareRevision("2A26", BLERead, 20);

BLEService batteryService("180F");
BLEByteCharacteristic batteryPercentage("2A19", BLERead);
class BatteryService : public FlourishService {
	public:
		void registerService() {
			Serial.println("Registering Battery Service");
			BLE.addService(batteryService);
		}

		void initialize() {
			Serial.println("Initializing Battery Service");
			batteryService.addCharacteristic(batteryPercentage);
			batteryPercentage.writeValue(80);
		}
};