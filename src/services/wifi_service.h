#pragma once
#include <ArduinoBLE.h>
#include "base_service.h"
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

struct Network
{
	String ssid;
	String password;
};

class WiFiService : public BaseService {
	public:
		void registerService() override;
		void registerAttributes() override;

		int execute() override;

		int initialize() override;
		bool isInitialized() override;

		// void onBLEConnected() override;
		// void onBLEDisconnected() override;

		WiFiService() = default;

	private:
	 	Network network;
		int scanNetworks();
		int joinNetwork();
		int saveNetwork();
};