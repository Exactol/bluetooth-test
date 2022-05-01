#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>

#include "../common.h"
#include "wifi.h"
#include "ble.h"

FlashStorage(wifiStorage, WiFiInfo);

BLEService wifiService("00000000-b50b-48b7-87e2-a6d52eb9cc9c");
BLEIntCharacteristic wifiState("00000001-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEWrite | BLEIndicate);
BLEStringCharacteristic wifiAPList("00000002-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEIndicate, BLE_MAX_CHARACTERISTIC_SIZE);
BLEStringCharacteristic wifiSsid("00000003-b50b-48b7-87e2-a6d52eb9cc9c", BLERead | BLEWrite | BLEIndicate, 32);
BLEStringCharacteristic wifiPassword("00000004-b50b-48b7-87e2-a6d52eb9cc9c", BLEWrite | BLEIndicate, 64);

int wifiStatus = WL_IDLE_STATUS;

void startWifi() {
	// stop ble
	Serial.println("Stopping BLE");
	BLE.stopAdvertise();
	BLE.disconnect();
	BLE.end();

	Serial.println("Initializing WiFi");

	// start WiFi
	wiFiDrv.wifiDriverDeinit();
	wiFiDrv.wifiDriverInit();
	wifiStatus = WL_IDLE_STATUS;
	// gives driver time to startup
	// TODO: is there a better way to do this
	delay(100);
	Serial.println("WiFi initialized");
}

void initializeWifiServices() {
	wifiService.addCharacteristic(wifiState);
	wifiService.addCharacteristic(wifiAPList);
	wifiService.addCharacteristic(wifiSsid);
	wifiService.addCharacteristic(wifiPassword);

	wifiState.writeValue(WIFI_COMMISSIONING_STATE::IDLE);
}

void registerWifiServices() {
	BLE.addService(wifiService);
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

	wifiStatus = WiFi.begin(wifiSsid.value().c_str(), wifiPassword.value().c_str());

	int reasonCode = WiFi.reasonCode();
	if ( wifiStatus != WL_CONNECTED ) {
		Serial.println("Status: " + String(wifiStatus));
		Serial.println("reason: " + String(reasonCode)); // https://community.cisco.com/t5/wireless-mobility-documents/802-11-association-status-802-11-deauth-reason-codes/ta-p/3148055

		switch (wifiStatus) {
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

			case WIFI_COMMISSIONING_STATE::ERROR:
				digitalWrite(RED_LED, HIGH);
				break;

			// idle
			default:
				break;
		}
	}
}
