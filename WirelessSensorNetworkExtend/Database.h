#pragma once
#include "Simulation.h"

namespace WSN
{
	class Database
	{
	public:
		Database(const Database&) = delete;

		inline static Database* GetDatabase() { return s_DatabaseInstance; }

		/// <summary>
		/// Saves the simulation hyperparameters
		/// </summary>
		/// <param name="simulationID">Simulation ID</param>
		/// <param name="simulationParameters">Simulation Hyperparameters</param>
		void Insert(uint64_t simulationID, const SimulationParameters& simulationParameters);

		/// <summary>
		/// Saves the initial sensor nodes in a simulation
		/// </summary>
		/// <param name="simulationID">Simulation ID</param>
		/// <param name="sensorNodes">Sensor Nodes within the simulation</param>
		void Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes);

		/// <summary>
		/// Saves the distribution parameters used to generate failure timestamps and the results of brute forcing delta
		/// </summary>
		/// <param name="simulationID">Simulation ID</param>
		/// <param name="dists"></param>
		/// <param name="sdd"></param>
		void Insert(uint64_t simulationID, const std::vector<Distribution>& dists, const std::vector<SimulationDistributionData>& sdd);


		uint64_t GetLatestSimulationID();

	private:
		Database();

		static Database* s_DatabaseInstance;
	};
}