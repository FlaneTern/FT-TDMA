#pragma once
#include "SensorNode.h"

namespace CTMCS
{
	class Database
	{
	public:
		Database(const Database&) = delete;

		inline static Database* GetDatabase() { return s_DatabaseInstance; }

		void Insert(uint64_t simulationID);
		void Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes);

		uint64_t GetLatestSimulationID();

	private:
		Database();

		static Database* s_DatabaseInstance;
	};
}

