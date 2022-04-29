#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>

#include "utility/wifi_drv.h"

const String model = "Flourish Device";
const String serialNumber = "abcde";
const String hardwareRevision = "0.1";
const String firmwareRevision = "0.1";

const int RED_LED = 2;
const int GREEN_LED = 3;
const int BLUE_LED = 4;

static const unsigned int BLE_MAX_CHARACTERISTIC_SIZE = 512;

enum class FLOURISH_EXCEPTION {
	NO_WIFI_SSID
};

namespace FLOURISH_DEVICE_STATE {
	enum {
		IDLE = 1,
		COMMISSIONING = 2,
		COMMISSIONED = 4,
		ERROR = 8
	};
}

namespace COMMISSIONING_STATE {
	enum {
		IDLE = 1,

		SAVE = 2,
		SAVING = 4,
		SAVED = 8,

		ERROR = 1024
	};
}

namespace WIFI_COMMISSIONING_STATE {
	enum {
		IDLE = 1,

		SCAN = 2,
		SCANNING = 4,
		SCANNED = 8,

		JOIN = 16,
		JOINING = 32,
		JOINED = 64,

		SAVE = 128,
		SAVING = 256,
		SAVED = 512,

		ERROR = 1024
	};
}

struct WiFiInfo
{
	char* ssid;
	char* password;
};

struct DeviceInfo
{
	uint32_t deviceId;
	String deviceToken;
	String name;
};

BLEService batteryService("180F");
BLEByteCharacteristic batteryPercentage("2A19", BLERead);

BLEService deviceInformationService("180A");
BLEStringCharacteristic deviceManufacturerName("2A29", BLERead, 20);
BLEStringCharacteristic deviceModelNumber("2A24", BLERead, 20);
BLEStringCharacteristic deviceSerialNumber("2A25", BLERead, 20);
BLEStringCharacteristic deviceHardwareRevision("2A27", BLERead, 20);
BLEStringCharacteristic deviceFirmwareRevision("2A26", BLERead, 20);

BLEService wifiService("00000000-b50b-48b7-87e2-a6d52eb9cc9c");
BLEIntCharacteristic wifiState("00000001-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic wifiAPList("00000002-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
BLEStringCharacteristic wifiSsid("00000003-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEWrite | BLEIndicate, 32);
BLEStringCharacteristic wifiPassword("00000004-b50b-48b7-87e2-a6d52eb9cc9c", BLEWrite | BLEIndicate, 64);

BLEService commissioningService("00000000-1254-4046-81d7-676ba8909661");
BLEIntCharacteristic commissioningState("00000001-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
BLEUnsignedIntCharacteristic commissioningDeviceID("00000002-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic commissioningDeviceToken("00000003-1254-4046-81d7-676ba8909661", BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
BLEStringCharacteristic commissioningDeviceName("00000004-1254-4046-81d7-676ba8909661", BLERead | BLEWrite | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);

FlashStorage(wifiStorage, WiFiInfo);
FlashStorage(deviceStorage, DeviceInfo);

bool wifiMode = false;
int status = WL_IDLE_STATUS;

void startWifi() {
	wifiMode = true;

	// stop ble
	Serial.println("Stopping BLE");
	BLE.stopAdvertise();
	BLE.disconnect();
	BLE.end();

	Serial.println("Initializing WiFi");

	// start WiFi
	wiFiDrv.wifiDriverDeinit();
	wiFiDrv.wifiDriverInit();
	status = WL_IDLE_STATUS;
	// gives driver time to startup
	// TODO: is there a better way to do this
	delay(100);
	Serial.println("WiFi initialized");
}

void startBle() {
	wifiMode = false;

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

	BLE.addService(commissioningService);
	BLE.addService(wifiService);
	BLE.addService(batteryService);
	BLE.addService(deviceInformationService);
	BLE.advertise();
	Serial.println("BLE Initialized");
}

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

void setup()
{
	Serial.begin(9600);
	while (!Serial);

	// initialize pins
	pinMode(RED_LED, OUTPUT);
	pinMode(GREEN_LED, OUTPUT);
	pinMode(BLUE_LED, OUTPUT);

	// setup characteristics
	commissioningService.addCharacteristic(commissioningState);
	commissioningService.addCharacteristic(commissioningDeviceID);
	commissioningService.addCharacteristic(commissioningDeviceToken);
	commissioningService.addCharacteristic(commissioningDeviceName);
	commissioningDeviceName.setEventHandler(BLEWritten, [](BLEDevice device, BLECharacteristic characteristic) {
		// update localname if name is updated
		Serial.println("Name updated to " + commissioningDeviceName.value());
		BLE.setLocalName(commissioningDeviceName.value().c_str());
		BLE.advertise();
	});

	wifiService.addCharacteristic(wifiState);
	wifiService.addCharacteristic(wifiAPList);
	wifiService.addCharacteristic(wifiSsid);
	wifiService.addCharacteristic(wifiPassword);

	batteryService.addCharacteristic(batteryPercentage);

	deviceInformationService.addCharacteristic(deviceManufacturerName);
	deviceInformationService.addCharacteristic(deviceModelNumber);
	deviceInformationService.addCharacteristic(deviceSerialNumber);
	deviceInformationService.addCharacteristic(deviceHardwareRevision);
	deviceInformationService.addCharacteristic(deviceFirmwareRevision);

	commissioningState.writeValue(COMMISSIONING_STATE::IDLE);
	wifiState.writeValue(WIFI_COMMISSIONING_STATE::IDLE);

	deviceManufacturerName.writeValue("Flourish");
	deviceModelNumber.writeValue(model);
	deviceSerialNumber.writeValue(serialNumber);
	deviceHardwareRevision.writeValue(hardwareRevision);
	deviceFirmwareRevision.writeValue(firmwareRevision);

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

	startBle();

	Serial.println("Setup complete");
}

int scanNetworks() {
	Serial.println("Starting WiFi scan");
	wifiState.writeValue(WIFI_COMMISSIONING_STATE::SCANNING);

	startWifi();

	int8_t numSsid = WiFi.scanNetworks();
	Serial.println("Number of available networks: " + String(numSsid));

	if (numSsid == -1) {
		Serial.println("Couldn't get WiFi connection");
		// TODO: return error enum
		return -1;
	}

	// Serialize networks into string format
	// 2 					// network count
	// -85 Fios 	// rssi ssid
	// -88 Bar 		// rssi ssid
	Serial.println("Sending networks");
	String output_str;
	output_str.reserve(BLE_MAX_CHARACTERISTIC_SIZE);
	for (size_t i = 0; i < numSsid; i++)
	{
		String ssid(WiFi.SSID(i));
		String rssi(WiFi.RSSI(i));
		Serial.println("Network " + String(i) + ": " + ssid);
		Serial.println("Signal: " + rssi + " dBm");


		// string can't be larger than BLE max size (2 is size of space + newline)
		String newLine = "\n" + rssi + " " + ssid;
		if (output_str.length() + newLine.length() > BLE_MAX_CHARACTERISTIC_SIZE) {
			Serial.println("Too many networks, truncating");
			break;
		}

		output_str += newLine;
	}
	Serial.println("Output length: " + String(output_str.length()));

	// restart ble and send available AP list
	startBle();
	wifiAPList.writeValue(output_str);
	wifiState.writeValue(WIFI_COMMISSIONING_STATE::SCANNED);

	Serial.println("Scan complete");
	return 0;
}

int joinNetwork() {
	Serial.println("Joining Network: " + wifiSsid.value());
	wifiState.writeValue(WIFI_COMMISSIONING_STATE::JOINING);

	startWifi();

	Serial.println("Attempting to join");
	WiFi.setTimeout(10 * 1000);

	status = WiFi.begin(wifiSsid.value().c_str(), wifiPassword.value().c_str());

	int reasonCode = WiFi.reasonCode();
	if ( status != WL_CONNECTED ) {
		Serial.println("Status: " + String(status));
		Serial.println("reason: " + String(reasonCode)); // https://community.cisco.com/t5/wireless-mobility-documents/802-11-association-status-802-11-deauth-reason-codes/ta-p/3148055

		switch (status) {
			case WL_FAILURE:
				Serial.println("WiFi failure");
				break;
			case WL_NO_SSID_AVAIL:
				Serial.println("Failed to join, SSID not available");
				break;
			case WL_CONNECT_FAILED:
				Serial.println("Failed to join");
				break;
			case WL_DISCONNECTED:
				Serial.println("Failed to join, incorrect password");
				break;
		}

		// TODO: handle error
		return -1;
	}

	startBle();
	wifiState.writeValue(WIFI_COMMISSIONING_STATE::JOINED);

	return 0;
}

// TODO: TEST THIS
int saveDeviceInformation() {
	Serial.println("Saving information for device " + String(commissioningDeviceID.value()));

	if (commissioningDeviceID.value() == -1) {
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

int commissionDevice() {
	if (commissioningState.written()) {
		switch (commissioningState.value())
		{
			case COMMISSIONING_STATE::SAVE:
				saveDeviceInformation();
				break;

			case COMMISSIONING_STATE::ERROR:
				digitalWrite(RED_LED, HIGH);
				break;

			// idle
			default:
				break;
		}
	}
}

int commissionWifi() {
	if (wifiState.written()) {
		switch (wifiState.value())
		{
			case WIFI_COMMISSIONING_STATE::SCAN:
				scanNetworks();
				break;

			case WIFI_COMMISSIONING_STATE::SAVE:
				Serial.println("SAVE");
				delay(1000);
				break;

			case WIFI_COMMISSIONING_STATE::JOIN:
				joinNetwork();
				break;

			case COMMISSIONING_STATE::ERROR:
				digitalWrite(RED_LED, HIGH);
				break;

			// idle
			default:
				break;
		}
	}
}

void loop()
{
	BLEDevice central = BLE.central();

	digitalWrite(BLUE_LED, HIGH);

	if (central) {
		while (central.connected()) {
			commissionDevice();
			commissionWifi();
		}
	}

	delay(500);
	digitalWrite(BLUE_LED, LOW);
	delay(500);
}