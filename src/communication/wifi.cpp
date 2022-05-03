#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoBLE.h>

#include "../common.h"
#include "wifi.h"
#include "ble.h"

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
	// gives driver time to startup
	// TODO: is there a better way to do this
	delay(100);
	Serial.println("WiFi initialized");
}