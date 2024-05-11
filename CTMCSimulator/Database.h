#pragma once
#include "Simulation.h"

namespace CTMCS
{
	class Database
	{
	public:
		Database(const Database&) = delete;

		inline static Database* GetDatabase() { return s_DatabaseInstance; }

		void Insert(uint64_t simulationID, const SimulationParameters& sp);
		void Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes);
		void Insert(uint64_t simulationID, uint64_t resultID, const CTMCParameters& ctmcParams);
		void Insert(uint64_t simulationID, uint64_t resultID, const SimulationResults& sr);
		void Insert(uint64_t simulationID, uint64_t resultID, const std::vector<double>& stateTime);
		void Insert(uint64_t simulationID, uint64_t resultID, const std::vector<std::vector<double>>& transitionRateMatrix);

		uint64_t GetLatestSimulationID();

	private:
		Database();

		static Database* s_DatabaseInstance;
	};
}

