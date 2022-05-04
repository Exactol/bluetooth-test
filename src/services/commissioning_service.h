#pragma once
#include "base_service.h"

struct DeviceInfo
{
	uint32_t deviceId;
	String deviceToken;
	String name;
};

class CommissioningService : public BaseService {
	public:
		void registerService() override;
		void registerAttributes() override;

		int initialize() override;
		bool isInitialized() override;

		int execute() override;

		// void onBLEConnected() override;
		// void onBLEDisconnected() override;

		CommissioningService(int deviceType);

	private:
		int deviceType;
		DeviceInfo deviceInfo;
		int saveDeviceInformation();
};

namespace COMMISSIONING_STATE {
	enum {
		IDLE = 1,

		SAVE = 2,
		SAVING = 4,
		SAVED = 8,

		COMPLETE = 16,

		ERROR = 1024
	};
}

namespace COMMISSIONING_DEVICE_TYPE {
	enum {
		SENSOR = 1,
		GATEWAY = 2,

		OTHER = 512,
	};
}
