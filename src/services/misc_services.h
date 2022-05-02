#pragma once
#include "flourish_service.h"

class BatteryService : public FlourishService {
	public:
		void registerService();
		void initialize();
};