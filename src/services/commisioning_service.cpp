// #include <Arduino.h>
// #include <WiFiNINA.h>
// #include <ArduinoBLE.h>
// #include <FlashStorage.h>

// #include "../common.h"

// BLEService commissioningService("00000000-1254-4046-81d7-676ba8909661");
// BLEIntCharacteristic commissioningState("00000001-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
// BLEUnsignedIntCharacteristic commissioningDeviceID("00000002-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
// BLEStringCharacteristic commissioningDeviceToken("00000003-1254-4046-81d7-676ba8909661", BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
// BLEStringCharacteristic commissioningDeviceName("00000004-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);

// FlashStorage(deviceStorage, DeviceInfo);

// int saveDeviceInformation() {
// 	Serial.println("Saving information for device " + String(commissioningDeviceID.value()));

// 	if (commissioningDeviceID.value() == -1) {
// 		Serial.println("Failed to save information, device ID null");
// 		return -1;
// 	}

// 	DeviceInfo info = {
// 		commissioningDeviceID.value(),
// 		commissioningDeviceToken.value(),
// 		commissioningDeviceName.value(),
// 	};

// 	deviceStorage.write(info);
// 	Serial.println("Device Information Saved");

// 	return 0;
// }

// int commissionDevice() {
// 	if (commissioningState.written()) {
// 		switch (commissioningState.value())
// 		{
// 			case COMMISSIONING_STATE::SAVE:
// 				saveDeviceInformation();
// 				break;

// 			case COMMISSIONING_STATE::COMPLETE:
// 				// TODO: start wifi and stuff
// 				device_state = DEVICE_STATE::COMMISSIONED;
// 				break;

// 			case COMMISSIONING_STATE::ERROR:
// 				digitalWrite(RED_LED, HIGH);
// 				break;

// 			// idle
// 			default:
// 				break;
// 		}
// 	}
// }
