#include "PCH.h"
#include "Simulation.h"
#include "Database.h"


namespace WSN
{

	//std::vector<SimulationSummaryData> Simulation::s_Summary;

	Simulation::Simulation(SimulationParameters sp)
		: m_SimulationParameters(sp)
	{
#if not _DEBUG
		static uint64_t currentSimulationID = Database::GetDatabase()->GetLatestSimulationID();
#else
		static uint64_t currentSimulationID = 1;
#endif

		currentSimulationID++;
		m_SimulationID = currentSimulationID;

		GenerateSNs();
		CreateSNRoutingTables();
		CalculateSNDeltaOpts();

	}

	void Simulation::GenerateSNs()
	{
		if (m_SimulationParameters.LevelRadius.size() != m_SimulationParameters.LevelSNCount.size())
			throw std::runtime_error("m_SimulationParameters.LevelRadius.size() != m_SimulationParameters.LevelSNCount.size() in Simulation::GenerateSNs");

		for (int i = 0; i < m_SimulationParameters.LevelRadius.size(); i++)
		{
			// mean = (A + B) / 2
			// stddev = sqrt((b-a) * (b-a) / 12)
			Distribution dist(DistributionType::Uniform, m_SimulationParameters.LevelRadius[i] / 2.0,
				std::sqrt(m_SimulationParameters.LevelRadius[i] * m_SimulationParameters.LevelRadius[i] / 12.0));


			for (int SNCount = 0; SNCount < m_SimulationParameters.LevelSNCount[i]; )
			{
				double xPos = dist.GenerateRandomNumber();
				double yPos = dist.GenerateRandomNumber();

				if (xPos * xPos + yPos * yPos <= m_SimulationParameters.LevelRadius[i] * m_SimulationParameters.LevelRadius[i]
					&& (i == 0 || xPos * xPos + yPos * yPos > m_SimulationParameters.LevelRadius[i - 1] * m_SimulationParameters.LevelRadius[i - 1]))
				{
					m_SensorNodes.push_back(
						{
							{xPos, yPos},
							(int64_t) -2,
							(uint64_t)i
						}
					);
					SNCount++;
				}
			}
		}
	}

	void Simulation::CreateSNRoutingTables()
	{
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_Level == 0)
				m_SensorNodes[i].m_Parent = -1;
			else
				m_SensorNodes[i].m_Parent = m_SimulationParameters.LevelSNCount[m_SensorNodes[i].m_Level - 1] * (m_SensorNodes[i].m_Level - 1) + 1;
		}
	}

	// big parameters, i.e. durations instead of rates
	double PWSteadyStateFunc(double selfDelta, double selfTau, double selfLambda, double selfReparationTime, std::vector<double> childrenPWs)
	{
		double mu = 1.0;
		for (int i = 0; i < childrenPWs.size(); i++)
			mu += childrenPWs[i];
		mu *= selfReparationTime + selfDelta / 2.0;

		return 1.0 / (1.0 + (selfTau / selfDelta) + (mu / selfLambda));
	}


	void Simulation::CalculateSNDeltaOpts()
	{
		static constexpr int particleCount = 50;
		static constexpr double inertiaWeight = 0.5;
		static constexpr double cognitiveCoefficient = 1.5;
		static constexpr double socialCoefficient = 1.5;
		static constexpr int swarmBestChangeFinishThreshold = 200;


		std::vector<std::vector<uint64_t>> children(m_SensorNodes.size());

		std::vector<std::vector<double>> particles;

		std::vector<std::vector<double>> particlesVelocity;

		std::vector<std::vector<double>> particlesBestCoords;

		std::vector<double> particlesBestValue;

		std::vector<double> swarmBestCoords;
		double swarmBestValue;

		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_Parent == -1)
				continue;

			children[m_SensorNodes[i].m_Parent].push_back(i);
		}

		auto CalculatePW = [&](std::vector<double> deltasIn)
		{
			std::vector<double> PWs(m_SensorNodes.size());
			std::vector<double> PWWs(m_SensorNodes.size());

			for (int i = m_SimulationParameters.LevelRadius.size() - 1; i >= 0; i--)
			{
				for (int j = 0; j < m_SensorNodes.size(); j++)
				{
					if (m_SensorNodes[j].m_Level != i)
						continue;

					std::vector<double> childrenPWs;
					for (int k = 0; k < children[j].size(); k++)
						//childrenPWs.push_back(PWs[children[j][k]]);
						childrenPWs.push_back(PWWs[children[j][k]]);

					PWs[j] = PWSteadyStateFunc(deltasIn[j], m_SimulationParameters.TransferTime,
						m_SimulationParameters.FailureDistribution.m_Mean, m_SimulationParameters.RecoveryTime, childrenPWs);

					PWWs[j] = PWs[j];
					for (int k = 0; k < children[j].size(); k++)
						PWWs[j] += PWWs[children[j][k]];
				}
			}

			double BSTotal = 0.0;
			for (int i = 0; i < PWWs.size(); i++)
			{
				//if (m_SensorNodes[i].m_Parent == -1)
					//BSTotal += PWs[i];
					BSTotal += PWWs[i];
			}

			return BSTotal;
		};

		static constexpr double randomRangeHigh = 1.0;
		static constexpr double randomRangeLow = 100000;


		// mean = (A + B) / 2
		// stddev = sqrt((b-a) * (b-a) / 12)
		Distribution dist(DistributionType::Uniform, (randomRangeHigh + randomRangeLow) / 2.0,
			std::sqrt((randomRangeHigh - randomRangeLow) * (randomRangeHigh - randomRangeLow) / 12.0));

		Distribution dist01(DistributionType::Uniform, 1 / 2.0,
			std::sqrt(1 / 12.0));


		// particle count
		for (int i = 0; i < particleCount; i++)
		{
			std::vector<double> deltas;
			deltas.reserve(m_SensorNodes.size());
			for (int j = 0; j < m_SensorNodes.size(); j++)
				deltas.push_back(dist.GenerateRandomNumber());
			particles.push_back(deltas);
			particlesBestCoords.push_back(deltas);
			particlesBestValue.push_back(CalculatePW(deltas));
			
			std::vector<double> velos;
			velos.reserve(m_SensorNodes.size());
			for (int j = 0; j < m_SensorNodes.size(); j++)
			{
				double velo = dist.GenerateRandomNumber();
				if (s_RNG() % 2)
					velo *= -1;
				velos.push_back(velo);
			}
			particlesVelocity.push_back(velos);
		}


		{
			int bestIndex = -1;
			double bestValue = -1;
			for (int i = 0; i < particlesBestCoords.size(); i++)
			{
				if (particlesBestValue[i] > bestValue)
				{
					bestIndex = i;
					bestValue = particlesBestValue[i];
				}
			}

			swarmBestCoords = particlesBestCoords[bestIndex];
			swarmBestValue = bestValue;
		}

		int iterationsSinceLastSwarmBestChange = 0;
		int iteration = 0;

		while (iterationsSinceLastSwarmBestChange < swarmBestChangeFinishThreshold)
		{
			auto particlesTemp = particles;

			for (int particle = 0; particle < particleCount; particle++)
			{
				for (int dimension = 0; dimension < m_SensorNodes.size(); dimension++)
				{
					particlesVelocity[particle][dimension] =
						inertiaWeight * particlesVelocity[particle][dimension] +
						cognitiveCoefficient * dist01.GenerateRandomNumber() * (particlesBestCoords[particle][dimension] - particles[particle][dimension]) +
						socialCoefficient * dist01.GenerateRandomNumber() * (swarmBestCoords[dimension] - particles[particle][dimension]);
				}

				for (int dimension = 0; dimension < m_SensorNodes.size(); dimension++)
				{
					particlesTemp[particle][dimension] += particlesVelocity[particle][dimension];
					if (particlesTemp[particle][dimension] < 0.0)
						particlesTemp[particle][dimension] = 0.0;
				}

 				double temp = CalculatePW(particlesTemp[particle]);
				if (temp > particlesBestValue[particle])
				{
					particlesBestCoords[particle] = particlesTemp[particle];
					particlesBestValue[particle] = temp;

					if (temp > swarmBestValue)
					{
						swarmBestCoords = particlesTemp[particle];
						swarmBestValue = temp;
						iterationsSinceLastSwarmBestChange = -1;
					}

				}
			}

			particles = particlesTemp;

			iterationsSinceLastSwarmBestChange++;
			iteration++;

			std::cout << "Iteration : " << iteration << '\n';
			std::cout << "Delta = ( ";
			for (int i = 0; i < swarmBestCoords.size(); i++)
				std::cout << swarmBestCoords[i] << ", ";
			std::cout << " )\nPW = " << swarmBestValue << "\n--------------------------------------------------------------------------------------\n";
		}

		std::cout << "Done!";
	}

	void Simulation::Run()
	{
		static constexpr double failGenerationDurationMultiplier = 100.0;

		SimulationResults& sr = m_SimulationResults;
			
		// Generating failure timestamps
		std::vector<std::vector<double>> SNsFailureTimestamps(m_SensorNodes.size());
		std::vector<int> SNsFailureTimestampsIterator(m_SensorNodes.size(), 0);
		{
			double currentTime = 0;
			double timeToNextFailure;
			while (currentTime < failGenerationDurationMultiplier * m_SimulationParameters.TotalDurationToBeTransferred)
			{
				timeToNextFailure = m_SimulationParameters.FailureDistribution.GenerateRandomNumber();
				currentTime += timeToNextFailure;
				// LOOK AT THIS
				uint64_t SNID = s_RNG() % m_SensorNodes.size();
				sr.Failures.push_back({ SNID, currentTime});
				SNsFailureTimestamps[SNID].push_back(currentTime);
			}
		}


		struct WorkingStateTimestamp
		{
			uint64_t SNID;
			WorkingState State;
			double Timestamp;
		};

		std::vector<WorkingStateTimestamp> previousEvents;
		auto pqCompare = [](WorkingStateTimestamp left, WorkingStateTimestamp right) 
		{ 
			if (left.Timestamp < right.Timestamp)
				return true;
			else if(left.Timestamp > right.Timestamp)
				return false;

			return left.SNID > right.SNID;
		};

		std::priority_queue<WorkingStateTimestamp, std::vector<WorkingStateTimestamp>, decltype(pqCompare)> eventQueue(pqCompare);
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			eventQueue.push({ (uint64_t)i, WorkingState::Collection, 0.0 });
			previousEvents.push_back({ (uint64_t)i, WorkingState::Collection, 0.0 });
		}
		
		double transferredTotalDuration = 0;
		double currentTime = 0.0;
		int failureCount = 0;

		while (transferredTotalDuration < m_SimulationParameters.TotalDurationToBeTransferred)
		{

			auto currentEvent = eventQueue.top();
			currentTime = currentEvent.Timestamp;
			auto& currentState = currentEvent.State;
			auto& currentSN = currentEvent.SNID;
			eventQueue.pop();

			// deciding the next state to put in eventQueue
			{
				double nextTime = currentTime;
				WorkingState nextState;
				if (currentState == WorkingState::Collection)
				{
					nextTime += m_SensorNodes[currentSN].m_DeltaOpt;
					nextState = WorkingState::Transfer;
				}
				else if (currentState == WorkingState::Transfer)
				{
					nextTime += m_SimulationParameters.TransferTime;
					nextState = WorkingState::Collection;
				}
				else if (currentState == WorkingState::Recovery)
				{
					nextTime += m_SimulationParameters.RecoveryTime;
					nextState = WorkingState::Collection;
				}

				if (SNsFailureTimestampsIterator[currentSN] < SNsFailureTimestamps[currentSN].size() && 
					nextTime >= SNsFailureTimestamps[currentSN][SNsFailureTimestampsIterator[currentSN]])
				{
					eventQueue.push({ currentSN, WorkingState::Recovery, SNsFailureTimestamps[currentSN][SNsFailureTimestampsIterator[currentSN]] });
					SNsFailureTimestampsIterator[currentSN]++;
				}
				else
				{
					eventQueue.push({ currentSN, nextState, nextTime });
					SNsFailureTimestampsIterator[currentSN]++;
				}

			}

			// still no rerouting
			if (previousEvents[currentSN].State == WorkingState::Collection)
			{
				if (currentState == WorkingState::Collection) // handles the initialization
					continue; 
				else if (currentState == WorkingState::Transfer)
				{
					m_SensorNodes[currentSN].m_CollectionTime += m_SensorNodes[currentSN].m_DeltaOpt;
					m_SensorNodes[currentSN].m_CurrentData += m_SensorNodes[currentSN].m_DeltaOpt;
				}
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
				}
			}
			else if (previousEvents[currentSN].State == WorkingState::Transfer)
			{
				if (currentState == WorkingState::Collection)
				{
					if(m_SensorNodes[currentSN].m_Parent != -1)
					{
						if (previousEvents[m_SensorNodes[currentSN].m_Parent].State != WorkingState::Recovery)
							m_SensorNodes[m_SensorNodes[currentSN].m_Parent].m_CurrentData += m_SensorNodes[currentSN].m_CurrentData;
					}
					else
					{
						transferredTotalDuration += m_SensorNodes[currentSN].m_CurrentData;
					}

					m_SensorNodes[currentSN].m_WastedTime += m_SimulationParameters.TransferTime;
					m_SensorNodes[currentSN].m_CurrentData = 0;
				}
				else if (currentState == WorkingState::Transfer) {} // not possible
				else if (currentState == WorkingState::Recovery) // WARNING : PARTIAL TRANSFER FAILS	
				{
					m_SensorNodes[currentSN].m_CurrentData = 0;
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
				}
			}
			else if (previousEvents[currentSN].State == WorkingState::Recovery)
			{
				if (currentState == WorkingState::Collection) // does nothing
					m_SensorNodes[currentSN].m_WastedTime += m_SimulationParameters.RecoveryTime;
				else if (currentState == WorkingState::Transfer) {} // not possible
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
				}
			}

			// throw std::runtime_error("Exceeded the last failure point!");;


			previousEvents[currentSN] = currentEvent;

		}

		sr.ActualTotalDuration = currentTime;
		sr.FinalFailureIndex = failureCount;


#if not _DEBUG
		Database::GetDatabase()->Insert(m_SimulationID, m_SimulationParameters);
		Database::GetDatabase()->Insert(m_SimulationID, m_SensorNodes);
		Database::GetDatabase()->Insert(m_SimulationID, m_SimulationResults);
#endif
	}



}