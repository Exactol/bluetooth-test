#pragma once
#include <vector>
#include "./services/base_service.h"

class BluetoothCommissioner {
	public:
		void setupServices();
		void registerServices();

		int initialize();
		bool isInitialized();

		int execute();

		BluetoothCommissioner(std::vector<BaseService*> services);

	private:
		std::vector<BaseService*> services;
};