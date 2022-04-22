#include <ArduinoBLE.h>
// #include <

const String model = "Flourish Device";
const String serialNumber = "abcde";
const String hardwareRevision = "0.1";
const String firmwareRevision = "0.1";

BLEService batteryService("180F");
BLEByteCharacteristic batteryPercentage("2A19", BLERead);

BLEService deviceInformationService("180A");
BLEStringCharacteristic deviceManufacturerName("2A29", BLERead, 20);
BLEStringCharacteristic deviceModelNumber("2A24", BLERead, 20);
BLEStringCharacteristic deviceSerialNumber("2A25", BLERead, 20);
BLEStringCharacteristic deviceHardwareRevision("2A27", BLERead, 20);
BLEStringCharacteristic deviceFirmwareRevision("2A26", BLERead, 20);

// from https://community.silabs.com/s/share/a5U1M000000ko4IUAQ/how-to-use-bluetooth-lowenergy-for-wifi-commissioning?language=en_US
// TODO: create proper UUID https://devzone.nordicsemi.com/guides/short-range-guides/b/bluetooth-low-energy/posts/ble-services-a-beginners-tutorial
BLEService wifiScannerService("00000000-b50b-48b7-87e2-a6d52eb9cc9c");
BLEByteCharacteristic wifiScannerScanState("00000001-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEWrite | BLEIndicate);

BLEService wifiConfiguratorService("00000000-dabd-4a32-8e63-7631272ab6e3");
BLEByteCharacteristic wifiConfigState("00000001-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic wifiConfigSsid("00000002-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate, 32);
BLEStringCharacteristic wifiConfigPassword("00000003-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate, 16);
// BLEByteCharacteristic wifiConfigSecurity("00000001-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate, 16);
// BLEByteCharacteristic wifiSSDCharacteristic("", BLEWrite);

namespace WIFI_SCANNER_STATE {
	enum {
		IDLE = 1,
		SCAN = 2,
		SCANNING = 4,
		SCANNED = 8,
		ERROR = 16
	};
}

namespace WIFI_SCANNER_COMMANDS {
	enum {
		SCAN = 2,
	};
}

namespace WIFI_CONFIG_STATE {
	enum {
		IDLE = 1,
		SAVE = 2,
		SAVING = 4,
		SAVED = 8,
		JOIN = 16,
		JOINING = 32,
		JOINED = 64,
		ERROR = 128
	};
}

// namespace FLOURISH_DEVICE_STATE {
// 	enum {
// 		IDLE = 1,
// 		COMMISSIONING = 2,
// 		COMMISSIONED = 4,
// 		ERROR = 8
// 	};
// }

void onBLEConnected(BLEDevice central) {
	Serial.print("Connected event, central: ");
	Serial.println(central.address());

	wifiScannerScanState.writeValue(WIFI_CONFIG_STATE::IDLE);
	wifiConfigState.writeValue(WIFI_SCANNER_STATE::IDLE);

	// TODO:
	batteryPercentage.writeValue(80);
}

void onBLEDisconnected(BLEDevice central) {
	Serial.print("Disconnected event, central: ");
	Serial.println(central.address());
	// TODO: cleanup
}

void setup()
{
	Serial.begin(9600);
	while (!Serial);

	Serial.println("Initializing BLE");

	// initialize BLE
	if (!BLE.begin()) {
		Serial.println("Failed to start BLE");

		while (1);
	}

	Serial.println("BLE Initialized");

	// TODO: user configurable name?
	BLE.setLocalName("Flourish Device");
	BLE.setDeviceName("Flourish Device");
	BLE.setAdvertisedService(wifiScannerService);

	// setup BLE services and characteristics
	wifiScannerService.addCharacteristic(wifiScannerScanState);
	BLE.addService(wifiScannerService);

	wifiConfiguratorService.addCharacteristic(wifiConfigState);
	BLE.addService(wifiConfiguratorService);

	batteryService.addCharacteristic(batteryPercentage);
	BLE.addService(batteryService);

	deviceInformationService.addCharacteristic(deviceManufacturerName);
	deviceInformationService.addCharacteristic(deviceModelNumber);
	deviceInformationService.addCharacteristic(deviceSerialNumber);
	deviceInformationService.addCharacteristic(deviceHardwareRevision);
	deviceInformationService.addCharacteristic(deviceFirmwareRevision);

	deviceManufacturerName.writeValue("Flourish");
	deviceModelNumber.writeValue(model);
	deviceSerialNumber.writeValue(serialNumber);
	deviceHardwareRevision.writeValue(hardwareRevision);
	deviceFirmwareRevision.writeValue(firmwareRevision);
	BLE.addService(deviceInformationService);

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

	BLE.setAppearance(0x0540); // set appearance to Generic Sensor (from BLE appearance values)

	BLE.advertise();
}

void scanner() {
	while (wifiScannerScanState.value() != WIFI_SCANNER_STATE::SCANNED) {
		if (wifiScannerScanState.written()) {
			Serial.println("Written" + String(wifiScannerScanState.value()));
			switch (wifiScannerScanState.value())
			{
				case WIFI_SCANNER_STATE::IDLE:
					Serial.println("IDLE");
					delay(1000);
					break;
				case WIFI_SCANNER_STATE::SCAN:
					Serial.println("SAVE");
					delay(1000);
					break;

				default:
					break;
			}
		}
	}
}

void configurator() {
	while (wifiConfigState.value() != WIFI_CONFIG_STATE::JOINED) {
		if (wifiConfigState.written()) {
			Serial.println("WiFi Config Written " + String(wifiConfigState.value()));
			switch (wifiConfigState.value())
			{
				case WIFI_CONFIG_STATE::IDLE:
					Serial.println("IDLE");
					delay(1000);
					break;
				case WIFI_CONFIG_STATE::JOIN:
					Serial.println("JOIN");

					delay(1000);
					break;

				default:
					break;
			}
		}
	}
}

void loop()
{
	BLEDevice central = BLE.central();

	if (central) {
		while (central.connected()) {
			// if (wifiScannerScanState.written()) {
			// 	Serial.println("Written" + String(wifiScannerScanState.value()));
			// 	switch (wifiScannerScanState.value())
			// 	{
			// 		case WIFI_CONFIG_STATE::IDLE:
			// 			Serial.println("IDLE");
			// 			delay(1000);
			// 			break;
			// 		case WIFI_CONFIG_STATE::SAVE:
			// 			Serial.println("SAVE");
			// 			delay(1000);
			// 			break;

			// 		default:
			// 			break;
			// 	}
			// }
		}
	}

	delay(1000);
}