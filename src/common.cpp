#include "common.h"
#include "communication/ble.h"
#include "bluetooth_commissioner.h"
#include "services/commissioning_service.h"
#include "services/misc_services.h"
#include "services/commissioning_service.h"
#include "services/wifi_service.h"

int deviceState = DEVICE_STATE::IDLE;

void setupDevice() {
	Serial.println("All services initialized, setting up device");
	deviceState = DEVICE_STATE::COMMISSIONED;
	digitalWrite(BLUE_LED, LOW);
	digitalWrite(GREEN_LED, HIGH);
}