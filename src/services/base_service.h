#pragma once

class BaseService {
	public:
		virtual void registerService() = 0;
		virtual void registerAttributes() = 0;

		virtual int execute() {};

		virtual int initialize() {};
		virtual bool isInitialized() { return true; };

		virtual void onBLEConnected() {};
		virtual void onBLEDisconnected() {};
};