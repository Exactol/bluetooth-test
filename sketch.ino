#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <vector>
#include <sstream>
#include
#include "utility/wifi_drv.h"

const String model = "Flourish Device";
const String serialNumber = "abcde";
const String hardwareRevision = "0.1";
const String firmwareRevision = "0.1";

const int RED_LED = 2;
const int GREEN_LED = 3;
const int BLUE_LED = 4;

struct Network {
	int32_t rssi;
	const char* ssid; // ssids have a max length of 32
};

static const unsigned int BLE_MAX_CHARACTERISTIC_SIZE = 512;

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

enum class FLOURISH_EXCEPTION {
	NO_WIFI_SSID
};

// namespace FLOURISH_DEVICE_STATE {
// 	enum {
// 		IDLE = 1,
// 		COMMISSIONING = 2,
// 		COMMISSIONED = 4,
// 		ERROR = 8
// 	};
// }

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
BLEStringCharacteristic wifiScannerAPList("00000002-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);

BLEService wifiConfiguratorService("00000000-dabd-4a32-8e63-7631272ab6e3");
BLEByteCharacteristic wifiConfigState("00000001-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic wifiConfigSsid("00000002-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate, 32);
BLEStringCharacteristic wifiConfigPassword("00000003-dabd-4a32-8e63-7631272ab6e3", BLERead | BLEWrite | BLEIndicate, 64);

// FlashStorage(flashStorage, DeviceCommissioning::PersistentInfo);

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

	// end wifi
	// Serial.println("Stopping WiFi");
	// WiFi.end();

	Serial.println("Initializing BLE");

	// initialize BLE
	if (!BLE.begin()) {
		Serial.println("Failed to start BLE");
		while (1);
	}

	// TODO: user configurable name?
	BLE.setLocalName("Flourish Device");
	BLE.setDeviceName("Flourish Device");

	BLE.setAdvertisedService(wifiScannerService);

	BLE.setAppearance(0x0540); // set appearance to Generic Sensor (from BLE appearance values)

	BLE.addService(wifiScannerService);
	BLE.addService(wifiConfiguratorService);
	BLE.addService(batteryService);
	BLE.addService(deviceInformationService);
	BLE.advertise();
	Serial.println("BLE Initialized");
}

void onBLEConnected(BLEDevice central) {
	Serial.print("Connected event, central: ");
	Serial.println(central.address());

	wifiScannerScanState.writeValue(WIFI_CONFIG_STATE::IDLE);
	wifiConfigState.writeValue(WIFI_SCANNER_STATE::IDLE);

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
	wifiScannerService.addCharacteristic(wifiScannerScanState);
	wifiScannerService.addCharacteristic(wifiScannerAPList);

	wifiConfiguratorService.addCharacteristic(wifiConfigState);
	wifiConfiguratorService.addCharacteristic(wifiConfigSsid);
	wifiConfiguratorService.addCharacteristic(wifiConfigPassword);

	batteryService.addCharacteristic(batteryPercentage);

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

	// setup event handlers
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

	startBle();

	Serial.println("Setup complete");
}

std::vector<Network> getNetworks() {
	Serial.println("Scanning networks");
	int8_t numSsid = WiFi.scanNetworks();

	if (numSsid == -1) {
		Serial.println("Couldn't get WiFi connection");
		// TODO: raise error
	}

	Serial.println("Number of available networks: " + String(numSsid));

	std::vector<Network> networks(numSsid);
	for (size_t i = 0; i < numSsid; i++)
	{
		networks[i] = {
			WiFi.RSSI(i),
			WiFi.SSID(i)
		};

		Serial.println("Network " + String(i) + ": " + String( networks[i].ssid ));
		Serial.println("Signal: " + String(networks[i].rssi) + " dBm");
	}

	return networks;
}

void scanner() {
	if (wifiScannerScanState.written()) {
		Serial.println("Written" + String(wifiScannerScanState.value()));
		switch (wifiScannerScanState.value())
		{
			case WIFI_SCANNER_STATE::IDLE:
				Serial.println("IDLE");
				break;
			case WIFI_SCANNER_STATE::SCAN: {
				Serial.println("Starting WiFi scan");
				wifiScannerScanState.writeValue(WIFI_SCANNER_STATE::SCANNING);

				startWifi();

				std::vector<Network> networks = getNetworks();

				// restart ble and send values
				startBle();

				Serial.println("Found " + String(networks.size()) + " networks");

				// Serialize networks into string format
				// 2 					// network count
				// -85 Fios 	// rssi ssid
				// -88 Bar 		// rssi ssid
				std::ostringstream output_stream;
				output_stream << networks.size();
				for (size_t i = 0; i < networks.size(); i++)
				{
					String ssid(networks[i].ssid);
					String rssi(networks[i].rssi);

					// string can't be larger than BLE max size (2 is size of space + newline)
					if ((size_t) output_stream.tellp() + ssid.length() + rssi.length() + 2 > BLE_MAX_CHARACTERISTIC_SIZE) {
						Serial.println("Too many networks, truncating");
						break;
					}

					output_stream << "\n" << rssi.c_str() << " " << ssid.c_str();
				}

				wifiScannerAPList.writeValue(String(output_stream.str().c_str()));
				wifiScannerScanState.writeValue(WIFI_SCANNER_STATE::SCANNED);

				Serial.println("Scan complete");
				break;
			}
			default:
				break;
		}
	}
}

int configurator() {
	if (wifiConfigState.written()) {
		Serial.println("WiFi Config Written " + String(wifiConfigState.value()));
		switch (wifiConfigState.value())
		{
			case WIFI_CONFIG_STATE::IDLE:
				Serial.println("IDLE");
				break;
			case WIFI_CONFIG_STATE::SAVE:
				Serial.println("SAVE");

				delay(1000);
				break;

			case WIFI_CONFIG_STATE::JOIN: {
				Serial.println("JOIN");
				wifiConfigState.writeValue(WIFI_CONFIG_STATE::JOINING);
				// if (wifiConfigSsid.value() == NULL) {
				// 	// TODO: raise error
				// 	int a = 3;
				// }
				Serial.println("Joining ");
				Serial.println(wifiConfigSsid.value());

				startWifi();

				Serial.println("Attempting to join");
				WiFi.setTimeout(10 * 1000);
				status = WiFi.begin(wifiConfigSsid.value().c_str(), wifiConfigPassword.value().c_str());
				int reasonCode = WiFi.reasonCode();
				Serial.println("Status: " + String(status));
				Serial.println("reason: " + String(reasonCode)); // https://community.cisco.com/t5/wireless-mobility-documents/802-11-association-status-802-11-deauth-reason-codes/ta-p/3148055

				if ( status != WL_CONNECTED ) {
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

				wifiConfigState.writeValue(WIFI_CONFIG_STATE::JOINED);
				break;
			}
			default:
				break;
		}
	}

	return 0;
}

void loop()
{
	BLEDevice central = BLE.central();

	digitalWrite(BLUE_LED, HIGH);

	if (central) {
		while (central.connected()) {
			scanner();
			configurator();
		}
	}

	delay(500);
	digitalWrite(BLUE_LED, LOW);
	delay(500);
}