#pragma once
#include <vector>
#include "./services/base_service.h"

class BluetoothCommissioner {
	public:
		void startCommissioning();
		int completeCommissioning();

		void setupServices();
		void registerServices();

		int initialize();
		bool isInitialized();

		int execute();

		// TODO: unique pointers
		BluetoothCommissioner(std::vector<BaseService*> services) {
			this->services = services;
		};

		std::vector<BaseService*> const getServices() {
			return services;
		};
	private:
		std::vector<BaseService*> services;
};