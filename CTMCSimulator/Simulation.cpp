#include "PCH.h"

#include "Simulation.h"

namespace CTMCS
{
	static std::mt19937_64 s_Random = std::mt19937_64(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	Simulation::Simulation(SimulationParameters simulationParameters)
	{
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
	}


	void Simulation::Run(SimulationRunParameters simulationRunParameters)
	{
		
		bool done = false;
		int runIterator = 0;

		double bigLambdaCurrent = simulationRunParameters.BigLambdaStart;
		std::vector<double> bigDeltaLevelCurrent = simulationRunParameters.BigDeltaLevelStart;

		auto SNs = m_SensorNodes;

		while (!done)
		{
			CTMCParameters CTMCParams;
			SimulationResults simulationResults;

			for (int i = 0; i < m_SimulationParameters.MaxLevel + 1; i++)
			{
				CTMCParams.Tau.push_back(1.0 / m_SimulationParameters.TransferTime);
				CTMCParams.Lambda.push_back(1.0 / simulationRunParameters.BigLambdaStart);
				CTMCParams.Delta.push_back(1.0 / bigDeltaLevelCurrent[i]);
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
						//else if (individualStatesTo[k] == Transfer && individualStatesTo[SNs[k].Parent] == Recovery)
						//	rateIsZero = true;
						else if (individualStatesFrom[k] == Collection && individualStatesTo[k] == Transfer)
							rate *= CTMCParams.Delta[SNs[k].Level];
						else if (individualStatesFrom[k] == Collection && individualStatesTo[k] == Recovery)
							rate *= CTMCParams.Lambda[SNs[k].Level];
						else if (individualStatesFrom[k] == Transfer && individualStatesTo[k] == Collection)
							rate *= CTMCParams.Tau[SNs[k].Level];
						else if (individualStatesFrom[k] == Recovery && individualStatesTo[k] == Collection)
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
					if (individualStatesFrom[i] == Collection && individualStatesTo[i] == Collection)
						SNs[i].CurrentDataSize += minimumTime;
					else if (individualStatesFrom[i] == Collection && individualStatesTo[i] == Transfer)
						SNs[i].CurrentDataSize += minimumTime;
					else if (individualStatesFrom[i] == Collection && individualStatesTo[i] == Recovery)
					{
						SNs[i].CurrentDataSize = 0.0;
						for (int j = 0; j < SNs.size(); j++)
						{
							if (i == j)
								continue;
							if (SNs[j].Parent == i && individualStatesFrom[j] == Transfer)
							{
								SNs[j].CurrentDataSize = 0; // DANGER : PARTIAL DATA TRANSFER FAILS
								// HANDLE TURNING CHILDREN TO Collection IF FAILED  !!!!!!!!!!!
								//if(individualStatesTo[j] == Transfer)
								//	nextState -= std::pow(3, j) * 2;
							}
						}


					}
					else if (individualStatesFrom[i] == Transfer && individualStatesTo[i] == Collection)
					{
						if (SNs[i].Parent != -1)
							SNs[SNs[i].Parent].CurrentDataSize += SNs[i].CurrentDataSize;
						else
							simulationResults.TotalDataSentToBS += SNs[i].CurrentDataSize;
						SNs[i].CurrentDataSize = 0.0;
					}
					else if (individualStatesFrom[i] == Transfer && individualStatesTo[i] == Transfer) {} // DANGER : PARTIAL DATA TRANSFER FAILS
					else if (individualStatesFrom[i] == Transfer && individualStatesTo[i] == Recovery) {} // not possible
					else if (individualStatesFrom[i] == Recovery && individualStatesTo[i] == Collection) {} // doesnt do anything
					else if (individualStatesFrom[i] == Recovery && individualStatesTo[i] == Transfer) {} // not possible
					else if (individualStatesFrom[i] == Recovery && individualStatesTo[i] == Recovery) {} // doesnt do anything



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

			std::cout << "BigLambda = " << bigLambdaCurrent << '\n';
			std::cout << "BigDelta = ( " << bigDeltaLevelCurrent[0];
			for (int i = 1; i < bigDeltaLevelCurrent.size(); i++)
				std::cout << ", " << bigDeltaLevelCurrent[i];
			std::cout << " )\n";
			//for (int i = 0; i < timeSpentInState.size(); i++)
			//	std::cout << "Time spent in state " << i << " = " << timeSpentInState[i] << '\n';
			std::cout << "Total Collection time = " << simulationResults.TotalCollectionTime << '\n';
			std::cout << "Total data sent to BS = " << simulationResults.TotalDataSentToBS << '\n';
			std::cout << "-------------------------------------------------------\n";


			// saving
			m_CTMCParameters.push_back(CTMCParams);
			m_TransitionRateMatrices.push_back(TransitionRateMatrix);
			m_StateTimes.push_back(timeSpentInState);
			m_SimulationResults.push_back(simulationResults);

			// TEMPORARILY CLEAR
			m_CTMCParameters.clear();
			m_TransitionRateMatrices.clear();
			m_StateTimes.clear();
			m_SimulationResults.clear();


			bigDeltaLevelCurrent[bigDeltaLevelCurrent.size() - 1] += simulationRunParameters.BigDeltaLevelStride[bigDeltaLevelCurrent.size() - 1];
			for (int i = bigDeltaLevelCurrent.size() - 1; i >= 0; i--)
			{
				if (bigDeltaLevelCurrent[i] > simulationRunParameters.BigDeltaLevelEnd[i])
				{
					if (i != 0)
					{
						bigDeltaLevelCurrent[i - 1] += simulationRunParameters.BigDeltaLevelStride[i - 1];
						bigDeltaLevelCurrent[i] = simulationRunParameters.BigDeltaLevelStart[i];
					}
					else
					{
						bigLambdaCurrent += simulationRunParameters.BigLambdaStride;
						if(bigLambdaCurrent <= simulationRunParameters.BigLambdaEnd)
							bigDeltaLevelCurrent = simulationRunParameters.BigDeltaLevelStart;
						else
							done = true;
					}
				}
			}



		}
	}
}