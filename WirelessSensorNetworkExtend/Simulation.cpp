#include "PCH.h"
#include "Simulation.h"
#include "Database.h"


namespace WSN
{

	//std::vector<SimulationSummaryData> Simulation::s_Summary;

	Simulation::Simulation(SimulationParameters sp)
		: m_SimulationParameters(sp)
	{

		static uint64_t currentSimulationID = Database::GetDatabase()->GetLatestSimulationID();

		currentSimulationID++;
		m_SimulationID = currentSimulationID;

		// Generating sensor nodes
		// TODO : CHANGE 100 TO A VARIABLE!
		for (int i = 0; i < 100; i++)
		{
			Position pos;
			pos.X = m_SimulationParameters.m_SensorNodeLocationDistribution.X.GenerateRandomNumber();
			pos.Y = m_SimulationParameters.m_SensorNodeLocationDistribution.Y.GenerateRandomNumber();
			m_SensorNodes.push_back({ pos });
		}
	}


	void Simulation::Run()
	{
		static constexpr double failGenerationDurationMultiplier = 100.0;

		for (int i = 0; i < m_FailureDistributions.size(); i++)
		{
			SimulationDistributionData sdd;
			
			// Generating failure timestamps
			{
				double currentTime = 0;
				double timeToNextFailure;
				while (currentTime < failGenerationDurationMultiplier * m_SimulationParameters.TotalDurationToBeTransferred)
				{
					timeToNextFailure = m_FailureDistributions[i].GenerateRandomNumber();
					currentTime += timeToNextFailure;
					sdd.FailureTimestamps.push_back(currentTime);
				}
			}


			// TODO : CHANGE DELTA TO A VARIABLE
			// Running the states
			for (double delta = 50.0; delta < 55.0; delta += 1.0)
			{
				SimulationResultData srd;
				srd.Delta = delta;

				double currentTime = 0;
				uint64_t failureIterator = 0;
				double nextFailureTime = sdd.FailureTimestamps[failureIterator];

				bool failed = false;

				double transferredTotalDuration = 0;

				WorkingState currentState = WorkingState::Collection;

				while (transferredTotalDuration < m_SimulationParameters.TotalDurationToBeTransferred)
				{
					if (failed)
					{
						failureIterator++;
						if (failureIterator < sdd.FailureTimestamps.size())
							nextFailureTime = sdd.FailureTimestamps[failureIterator];
						else
							throw std::runtime_error("Exceeded the last failure point!");;

						failed = false;
					}


					if (currentState == WorkingState::Collection)
					{
						double nextTime = currentTime + delta;
						if (nextTime > nextFailureTime)
						{
							failed = true;
							srd.SimulationIntervals.push_back({ WorkingState::Collection, currentTime, nextFailureTime });
							currentState = WorkingState::Recovery;

							srd.WastedTime += nextFailureTime - currentTime;

							currentTime = nextFailureTime;
						}
						else
						{
							srd.SimulationIntervals.push_back({ WorkingState::Collection, currentTime, nextTime });
							currentState = WorkingState::Transfer;

							srd.CollectionTime += delta;

							currentTime = nextTime;
						}
					}
					else if (currentState == WorkingState::Transfer)
					{
						double nextTime = currentTime + m_SimulationParameters.TransferTime;
						if (nextTime > nextFailureTime)
						{
							failed = true;
							srd.SimulationIntervals.push_back({ WorkingState::Transfer, currentTime, nextFailureTime });
							currentState = WorkingState::Recovery;

							srd.WastedTime += delta + nextFailureTime - currentTime;
							srd.CollectionTime -= delta;

							currentTime = nextFailureTime;

							// ALERT !!!!!!!!!!!!!!
							// DO PARTIALLY SENT DATAS CONTRIBUTE TO TRANSFERREDTOTALDURATION??
							
							//double proportionSent = (double)(nextFailureTime - currentTime) / m_SimulationParameters.TransferTime;
							//transferredTotalDuration += delta * proportionSent;
						}
						else
						{
							srd.SimulationIntervals.push_back({ WorkingState::Transfer, currentTime, nextTime });
							currentState = WorkingState::Collection;

							srd.WastedTime += m_SimulationParameters.TransferTime;

							currentTime = nextTime;
							transferredTotalDuration += delta;
						}
					}
					else
					{
						double nextTime = currentTime + m_SimulationParameters.RecoveryTime;
						if (nextTime > nextFailureTime)
						{
							failed = true;
							srd.SimulationIntervals.push_back({ WorkingState::Recovery, currentTime, nextFailureTime });
							currentState = WorkingState::Recovery;

							srd.WastedTime += nextFailureTime - currentTime;

							currentTime = nextFailureTime;
						}
						else
						{
							srd.SimulationIntervals.push_back({ WorkingState::Recovery, currentTime, nextTime });
							currentState = WorkingState::Collection;

							srd.WastedTime += m_SimulationParameters.RecoveryTime;

							currentTime = nextTime;
						}
					}

				}
				srd.ActualTotalDuration = currentTime;
				srd.FinalFailureIndex = failureIterator - 1;

				sdd.SRD.push_back(srd);
			}

			m_SimulationDistributionDatas.push_back(sdd);
		}

		Database::GetDatabase()->Insert(m_SimulationID, m_SimulationParameters);
		Database::GetDatabase()->Insert(m_SimulationID, m_FailureDistributions, m_SimulationDistributionDatas);
	}



}