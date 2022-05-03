#pragma once
#include "base_service.h"

class BatteryService : public BaseService {
	public:
		void registerService() override;
		void registerAttributes() override;
		BatteryService() = default;
};

class DeviceInformationService : public BaseService {
	public:
		void registerService() override;
		void registerAttributes() override;
		DeviceInformationService() = default;
};