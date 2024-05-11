#pragma once
#include "SensorNode.h"	

namespace CTMCS
{
	static constexpr int s_MaxThreadCount = 1;

	struct CTMCParameters
	{
		std::vector<double> Tau;
		std::vector<double> Lambda;
		std::vector<double> Delta;
		std::vector<double> Mu;
	};

	struct SimulationRunParameters
	{
		std::vector<double> BigDeltaLevelStart;
		std::vector<double> BigDeltaLevelEnd;
		std::vector<double> BigDeltaLevelStride;

		double BigLambdaStart;
		double BigLambdaEnd;
		double BigLambdaStride;
	};

	struct IterationSRP
	{
		IterationSRP(const SimulationRunParameters& srp);
		IterationSRP(const IterationSRP& previous, const SimulationRunParameters& srp);
		bool Done;
		std::vector<double> BigDeltaLevelCurrent;
		double BigLambdaCurrent;
	};

	struct SimulationParameters
	{
		double TransferTime = -1;
		double RecoveryTime = -1;
		double MaxSimulationTime = -1;

		uint64_t SNCount = 0;
		uint64_t MaxLevel = 0;
	};

	struct SimulationResults
	{
		double TotalCollectionTime = 0;
		double TotalDataSentToBS = 0;
	};


	class Simulation
	{
	public:
		Simulation(SimulationParameters simulationParameters);

		void Run(SimulationRunParameters simulationRunParameters);

	private:

		uint64_t m_SimulationID;

		// defined in constructor
		SimulationParameters m_SimulationParameters;
		std::vector<SensorNode> m_SensorNodes;
		std::vector<std::vector<int>> m_IndividualStates;

		//defined per run iteration
		std::vector<std::vector<std::vector<double>>> m_TransitionRateMatrices;
		std::vector<CTMCParameters> m_CTMCParameters;
		std::vector<std::vector<double>> m_StateTimes;
		std::vector<SimulationResults> m_SimulationResults;

		void InnerRun(uint64_t resultID, IterationSRP isrp, std::counting_semaphore<s_MaxThreadCount>* cs);

	};

}