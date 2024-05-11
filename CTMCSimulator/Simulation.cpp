#include "PCH.h"

#include "Simulation.h"

#include "Database.h"

namespace CTMCS
{
	static std::mt19937_64 s_Random = std::mt19937_64(std::chrono::high_resolution_clock::now().time_since_epoch().count());


	IterationSRP::IterationSRP(const SimulationRunParameters& srp)
		: Done(false), BigDeltaLevelCurrent(srp.BigDeltaLevelStart), BigLambdaCurrent(srp.BigLambdaStart)
	{
		if (BigLambdaCurrent > srp.BigLambdaEnd)
			throw std::runtime_error("BigLambdaStart " + std::to_string(BigLambdaCurrent) + " is bigger than BigLambdaEnd " + std::to_string(srp.BigLambdaEnd) + " !");
		
		for(int i = 0; i < BigDeltaLevelCurrent.size(); i++)
			if(BigDeltaLevelCurrent[i] > srp.BigDeltaLevelEnd[i])
				throw std::runtime_error("BigDeltaLevelCurrent " + std::to_string(i) + " " + std::to_string(BigLambdaCurrent) + " is bigger than BigLambdaEnd " + std::to_string(srp.BigLambdaEnd) + " !");
	}

	IterationSRP::IterationSRP(const IterationSRP& previous, const SimulationRunParameters& srp)
		: Done(false), BigDeltaLevelCurrent(previous.BigDeltaLevelCurrent), BigLambdaCurrent(previous.BigLambdaCurrent)
	{
		BigDeltaLevelCurrent[BigDeltaLevelCurrent.size() - 1] += srp.BigDeltaLevelStride[BigDeltaLevelCurrent.size() - 1];
		for (int i = BigDeltaLevelCurrent.size() - 1; i >= 0; i--)
		{
			if (BigDeltaLevelCurrent[i] > srp.BigDeltaLevelEnd[i])
			{
				if (i != 0)
				{
					BigDeltaLevelCurrent[i - 1] += srp.BigDeltaLevelStride[i - 1];
					BigDeltaLevelCurrent[i] = srp.BigDeltaLevelStart[i];
				}
				else
				{
					BigLambdaCurrent += srp.BigLambdaStride;
					if (BigLambdaCurrent <= srp.BigLambdaEnd)
						BigDeltaLevelCurrent = srp.BigDeltaLevelStart;
					else
						Done = true;
				}
			}
		}
	}

	Simulation::Simulation(SimulationParameters simulationParameters)
	{
		static uint64_t currentSimulationID = Database::GetDatabase()->GetLatestSimulationID();

		currentSimulationID++;
		m_SimulationID = currentSimulationID;

		m_SimulationParameters = simulationParameters;


		m_SensorNodes = std::vector<SensorNode>(m_SimulationParameters.SNCount);
		m_SensorNodes[0].Level = 0;


		for (int i = 1; i < m_SimulationParameters.SNCount; i++)
		{
			int randomParent = s_Random() % i;
			while ((randomParent != i && m_SensorNodes[randomParent].Level >= m_SimulationParameters.MaxLevel) || randomParent == i)
				randomParent = s_Random() % i;

			if (randomParent == m_SimulationParameters.SNCount)
				m_SensorNodes[i].Level = 0;
			else
			{
				m_SensorNodes[i].Level = m_SensorNodes[randomParent].Level + 1;
				m_SensorNodes[i].Parent = randomParent;
			}
		}


		for (int i = 0; i < m_SensorNodes.size(); i++)
			std::cout << "SN " << i << ", Parent : " << m_SensorNodes[i].Parent << ", Level : " << m_SensorNodes[i].Level << '\n';

		std::vector<double> SNCurrentDataSize(m_SimulationParameters.SNCount);


		for (int i = 0; i < std::pow(3, m_SimulationParameters.SNCount); i++)
		{
			std::vector<int> individualStatesCurrent(m_SimulationParameters.SNCount);
			int temp = i;
			int iterator = 0;
			do
			{
				individualStatesCurrent[iterator] = temp % 3;
				temp /= 3;
				iterator++;
			} while (temp > 0);

			m_IndividualStates.push_back(individualStatesCurrent);
		}


		Database::GetDatabase()->Insert(m_SimulationID, m_SensorNodes);
		Database::GetDatabase()->Insert(m_SimulationID, m_SimulationParameters);
	}

	void Simulation::Run(SimulationRunParameters simulationRunParameters)
	{
		std::counting_semaphore<s_MaxThreadCount> cs(s_MaxThreadCount);


		IterationSRP isrp(simulationRunParameters);

		uint64_t resultID = 0;
		while (!isrp.Done)
		{
			cs.acquire();
			std::thread th(&Simulation::InnerRun, this, resultID, isrp, &cs);
			th.detach();
			isrp = IterationSRP(isrp, simulationRunParameters);
			resultID++;
		}

		for (int i = 0; i < s_MaxThreadCount; i++)
			cs.acquire();
	}


	void Simulation::InnerRun(uint64_t resultID, IterationSRP isrp, std::counting_semaphore<s_MaxThreadCount>* cs)
	{
		auto SNs = m_SensorNodes;

		CTMCParameters CTMCParams;
		SimulationResults simulationResults;

		for (int i = 0; i < m_SimulationParameters.MaxLevel + 1; i++)
		{
			CTMCParams.Tau.push_back(1.0 / m_SimulationParameters.TransferTime);
			CTMCParams.Lambda.push_back(1.0 / isrp.BigLambdaCurrent);
			CTMCParams.Delta.push_back(1.0 / isrp.BigDeltaLevelCurrent[i]);
			CTMCParams.Mu.push_back(1.0 / (m_SimulationParameters.RecoveryTime + 1.0 / (2 * CTMCParams.Delta.back())));
		}
		for (int i = 0; i < SNs.size(); i++)
			SNs[i].CurrentDataSize = 0.0;

			

		std::vector<std::vector<bool>> TransitionExists(std::pow(3, m_SimulationParameters.SNCount), std::vector<bool>(std::pow(3, m_SimulationParameters.SNCount), false));
		std::vector<std::vector<double>> TransitionRateMatrix(std::pow(3, m_SimulationParameters.SNCount), std::vector<double>(std::pow(3, m_SimulationParameters.SNCount)));
		std::vector<std::vector<std::exponential_distribution<double>>> ExponentialDists(std::pow(3, m_SimulationParameters.SNCount), std::vector<std::exponential_distribution<double>>(std::pow(3, m_SimulationParameters.SNCount)));

		for (int i = 0; i < std::pow(3, m_SimulationParameters.SNCount); i++)
		{
			for (int j = 0; j < std::pow(3, m_SimulationParameters.SNCount); j++)
			{
				if (i == j)
					continue;

				const std::vector<int>& individualStatesFrom = m_IndividualStates[i];
				const std::vector<int>& individualStatesTo = m_IndividualStates[j];

				bool rateIsZero = false;

				double rate = 1.0;

				for (int k = 0; k < individualStatesFrom.size() && !rateIsZero; k++)
				{
					if (individualStatesFrom[k] == individualStatesTo[k])
						continue;
					//else if (individualStatesTo[k] == (int)WorkingState::Transfer && individualStatesTo[SNs[k].Parent] == (int)WorkingState::Recovery)
					//	rateIsZero = true;
					else if (individualStatesFrom[k] == (int)WorkingState::Collection && individualStatesTo[k] == (int)WorkingState::Transfer)
						rate *= CTMCParams.Delta[SNs[k].Level];
					else if (individualStatesFrom[k] == (int)WorkingState::Collection && individualStatesTo[k] == (int)WorkingState::Recovery)
						rate *= CTMCParams.Lambda[SNs[k].Level];
					else if (individualStatesFrom[k] == (int)WorkingState::Transfer && individualStatesTo[k] == (int)WorkingState::Collection)
						rate *= CTMCParams.Tau[SNs[k].Level];
					else if (individualStatesFrom[k] == (int)WorkingState::Recovery && individualStatesTo[k] == (int)WorkingState::Collection)
						rate *= CTMCParams.Mu[SNs[k].Level];
					else
						rateIsZero = true;
				}

				if (!rateIsZero)
				{
					TransitionRateMatrix[i][j] = rate;
					TransitionExists[i][j] = true;
				}
				else
					TransitionRateMatrix[i][j] = 0.0;
			}
		}

		for (int i = 0; i < TransitionRateMatrix.size(); i++)
		{
			double rateSum = 0.0;
			for (int j = 0; j < TransitionRateMatrix[0].size(); j++)
				rateSum += TransitionRateMatrix[i][j];
			TransitionRateMatrix[i][i] = -1 * rateSum;
		}

		for (int i = 0; i < TransitionRateMatrix.size(); i++)
		{
			for (int j = 0; j < TransitionRateMatrix[0].size(); j++)
			{
				if (TransitionExists[i][j])
					ExponentialDists[i][j] = std::exponential_distribution<double>(TransitionRateMatrix[i][j]);
			}
		}

		std::vector<double> timeSpentInState(std::pow(3, m_SimulationParameters.SNCount));
		std::vector<double> randomTime(std::pow(3, m_SimulationParameters.SNCount));

		int currentState = 0;

		for (double currentTime = 0.0; currentTime < m_SimulationParameters.MaxSimulationTime; )
		{
			//std::cout << "CurrentState = " << currentState << '\n';
			for (int i = 0; i < TransitionRateMatrix.size(); i++)
			{
				if (TransitionExists[currentState][i])
					randomTime[i] = ExponentialDists[currentState][i](s_Random);
				else
					randomTime[i] = std::numeric_limits<double>::infinity();
			}

			int nextState = -1;
			double minimumTime = std::numeric_limits<double>::infinity();

			for (int i = 0; i < TransitionRateMatrix.size(); i++)
			{
				if (minimumTime > randomTime[i])
				{
					nextState = i;
					minimumTime = randomTime[i];
				}
			}

			const std::vector<int>& individualStatesFrom = m_IndividualStates[currentState];
			const std::vector<int>& individualStatesTo = m_IndividualStates[nextState];
			for (int i = 0; i < SNs.size(); i++)
			{

				// TO DO !!!
				if (individualStatesFrom[i] == (int)WorkingState::Collection && individualStatesTo[i] == (int)WorkingState::Collection)
					SNs[i].CurrentDataSize += minimumTime;
				else if (individualStatesFrom[i] == (int)WorkingState::Collection && individualStatesTo[i] == (int)WorkingState::Transfer)
					SNs[i].CurrentDataSize += minimumTime;
				else if (individualStatesFrom[i] == (int)WorkingState::Collection && individualStatesTo[i] == (int)WorkingState::Recovery)
				{
					SNs[i].CurrentDataSize = 0.0;
					for (int j = 0; j < SNs.size(); j++)
					{
						if (i == j)
							continue;
						if (SNs[j].Parent == i && individualStatesFrom[j] == (int)WorkingState::Transfer)
						{
							SNs[j].CurrentDataSize = 0; // DANGER : PARTIAL DATA (int)WorkingState::Transfer FAILS
							// HANDLE TURNING CHILDREN TO (int)WorkingState::Collection IF FAILED  !!!!!!!!!!!
							//if(individualStatesTo[j] == (int)WorkingState::Transfer)
							//	nextState -= std::pow(3, j) * 2;
						}
					}


				}
				else if (individualStatesFrom[i] == (int)WorkingState::Transfer && individualStatesTo[i] == (int)WorkingState::Collection)
				{
					if (SNs[i].Parent != -1)
						SNs[SNs[i].Parent].CurrentDataSize += SNs[i].CurrentDataSize;
					else
						simulationResults.TotalDataSentToBS += SNs[i].CurrentDataSize;
					SNs[i].CurrentDataSize = 0.0;
				}
				else if (individualStatesFrom[i] == (int)WorkingState::Transfer && individualStatesTo[i] == (int)WorkingState::Transfer) {} // DANGER : PARTIAL DATA (int)WorkingState::Transfer FAILS
				else if (individualStatesFrom[i] == (int)WorkingState::Transfer && individualStatesTo[i] == (int)WorkingState::Recovery) {} // not possible
				else if (individualStatesFrom[i] == (int)WorkingState::Recovery && individualStatesTo[i] == (int)WorkingState::Collection) {} // doesnt do anything
				else if (individualStatesFrom[i] == (int)WorkingState::Recovery && individualStatesTo[i] == (int)WorkingState::Transfer) {} // not possible
				else if (individualStatesFrom[i] == (int)WorkingState::Recovery && individualStatesTo[i] == (int)WorkingState::Recovery) {} // doesnt do anything



			}

			timeSpentInState[currentState] += minimumTime;
			currentState = nextState;
			currentTime += minimumTime;
			//std::cout << "current time  = " << currentTime << '\n';

		}


		for (int i = 0; i < timeSpentInState.size(); i++)
		{
			std::vector<int> individualStatesCurrent = m_IndividualStates[i];
			int CollectionCount = 0;
			for (int j = 0; j < individualStatesCurrent.size(); j++)
			{
				if (individualStatesCurrent[j] == 0)
					CollectionCount++;
			}
			simulationResults.TotalCollectionTime += CollectionCount * timeSpentInState[i];
		}

		//for (int i = 0; i < TransitionRateMatrix.size(); i++)
		//{
		//	for (int j = 0; j < TransitionRateMatrix[0].size(); j++)
		//	{
		//		std::cout << TransitionRateMatrix[i][j] << ' ';
		//	}
		//	std::cout << '\n';
		//}

		std::cout << "BigLambda = " << isrp.BigLambdaCurrent << '\n';
		std::cout << "BigDelta = ( " << isrp.BigDeltaLevelCurrent[0];
		for (int i = 1; i < isrp.BigDeltaLevelCurrent.size(); i++)
			std::cout << ", " << isrp.BigDeltaLevelCurrent[i];
		std::cout << " )\n";
		//for (int i = 0; i < timeSpentInState.size(); i++)
		//	std::cout << "Time spent in state " << i << " = " << timeSpentInState[i] << '\n';
		std::cout << "Total Collection time = " << simulationResults.TotalCollectionTime << '\n';
		std::cout << "Total data sent to BS = " << simulationResults.TotalDataSentToBS << '\n';
		std::cout << "-------------------------------------------------------\n";


		// saving
		Database::GetDatabase()->Insert(m_SimulationID, resultID, CTMCParams);
		Database::GetDatabase()->Insert(m_SimulationID, resultID, simulationResults);
		Database::GetDatabase()->Insert(m_SimulationID, resultID, timeSpentInState);
		Database::GetDatabase()->Insert(m_SimulationID, resultID, TransitionRateMatrix);

		// i dont know what to do with this
#if 0
		m_CTMCParameters.push_back(CTMCParams);
		m_TransitionRateMatrices.push_back(TransitionRateMatrix);
		m_StateTimes.push_back(timeSpentInState);
		m_SimulationResults.push_back(simulationResults);

		// TEMPORARILY CLEAR
		m_CTMCParameters.clear();
		m_TransitionRateMatrices.clear();
		m_StateTimes.clear();
		m_SimulationResults.clear();

#endif

		cs->release();
	}
}