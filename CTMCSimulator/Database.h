#pragma once
#include "Simulation.h"

namespace CTMCS
{
	class Database
	{
	public:
		Database(const Database&) = delete;

		inline static Database* GetDatabase() { return s_DatabaseInstance; }
		inline static std::mutex* GetInsertionMutex() { return s_InsertionMutex; }

		void Insert(uint64_t simulationID, const SimulationParameters& sp);
		void Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes);
		void Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<CTMCParameters>& ctmcParams);
		void Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<SimulationResults>& sr);
		void Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<std::vector<double>>& stateTime);
		void Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<std::vector<std::vector<double>>>& transitionRateMatrix);

		uint64_t GetLatestSimulationID();

	private:
		Database();

		static Database* s_DatabaseInstance;

		static std::mutex* s_InsertionMutex;
	};
}

