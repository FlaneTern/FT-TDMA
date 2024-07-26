#include "PCH.h"
#include "Simulation.h"
#include "Database.h"

#define REROUTE_MCR 1

namespace WSN
{

	//static constexpr double s_EnergyRateWorking = 0.4;
	//static constexpr double s_EnergyRateDataTransfer = 1.0;
	//static constexpr double s_EnergyTransitionWorkingToTransfer = 20.0;
	//static constexpr double s_EnergyTransitionTransferToWorking = 20.0;

	static constexpr double s_EnergyTransitionWorkingToTransfer = 0.0;
	static constexpr double s_EnergyTransitionTransferToWorking = 0.0;

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

		std::cout << "\n\nPrintingSNLocations...";
		for (int i = 0; i < m_SensorNodes.size(); i++)
			std::cout << "{" << m_SensorNodes[i].m_Position.X << ", " << m_SensorNodes[i].m_Position.Y << "},\n";

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
				double xPos = dist.GenerateRandomNumber() * (s_RNG() % 2 ? -1 : 1);
				double yPos = dist.GenerateRandomNumber() * (s_RNG() % 2 ? -1 : 1);

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
					m_SensorNodes.back().m_CurrentParent = m_SensorNodes.back().m_Parent;
					SNCount++;
				}
			}
		}


	}

	Simulation::Simulation(SimulationParameters sp, std::vector<Position> SNLocations)
		: m_SimulationParameters(sp)
	{
#if not _DEBUG
		static uint64_t currentSimulationID = Database::GetDatabase()->GetLatestSimulationID();
#else
		static uint64_t currentSimulationID = 1;
#endif

		currentSimulationID++;
		m_SimulationID = currentSimulationID;

		for (int i = 0; i < SNLocations.size(); i++)
		{
			int level = 0;
			while (std::sqrt(SNLocations[i].X * SNLocations[i].X + SNLocations[i].Y * SNLocations[i].Y) > m_SimulationParameters.LevelRadius[level])
			{
				level++;
				if (level >= m_SimulationParameters.LevelRadius.size())
					throw std::runtime_error("SN is located outside of the highest level!");
			}
			m_SensorNodes.push_back(
				{
					{SNLocations[i].X, SNLocations[i].Y},
					(int64_t)-2,
					(uint64_t)level
				}
			);
			m_SensorNodes.back().m_CurrentParent = m_SensorNodes.back().m_Parent;
		}

		CreateSNRoutingTables();
		CalculateSNDeltaOpts();

		std::cout << "\n\nPrintingSNLocations...";
		for (int i = 0; i < m_SensorNodes.size(); i++)
			std::cout << "{" << m_SensorNodes[i].m_Position.X << ", " << m_SensorNodes[i].m_Position.Y << "},\n";
	}

	void Simulation::CreateSNRoutingTables()
	{
#if 0
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_Level == 0)
				m_SensorNodes[i].m_Parent = -1;
			else
				m_SensorNodes[i].m_Parent = m_SimulationParameters.LevelSNCount[m_SensorNodes[i].m_Level - 1] * (m_SensorNodes[i].m_Level - 1) + 1;
		}
#else
		// codenya rama =D

		using namespace std;

		const short spare = 5;

		class Node {
		public:
			float x, y;
			short tier, index, color = -1, degree = 0;
			vector <Node> route;
			Node(float arg1, float arg2, short arg3, short arg4) {
				x = arg1;
				y = arg2;
				tier = arg3;
				index = arg4;
			}
		};

		vector <Node> node;
		vector < pair <short, short> > edge;

		auto getDistance = [](Node node1, Node node2)
		{
			return sqrt((node1.x - node2.x) * (node1.x - node2.x) + (node1.y - node2.y) * (node1.y - node2.y));
		};

		auto findParent = [&]() {
			for (auto& i : node) {
				if (i.tier) {
					vector < pair <Node, float> > pos;
					for (auto j : node) {
						if (i.tier == j.tier + 1)
							pos.push_back(make_pair(j, getDistance(i, j)));
					}
					sort(pos.begin(), pos.end(), [&](pair <Node, float> ll, pair <Node, float> rr) {
						return ll.second < rr.second;
						});
					for (short j = 0; j < spare; ++j)
						if (pos[j].second <= m_SimulationParameters.TransmissionRange)
							i.route.push_back(pos[j].first);
				}
			}
		};

		auto findColor = [&]() { // i cant name shit
			for (auto& i : node)
				for (auto j : node)
					if (getDistance(i, j) <= m_SimulationParameters.InterferenceRange)
						++i.degree;
			sort(node.begin(), node.end(), [&](Node node1, Node node2) {
				return node1.degree > node2.degree;
				});
			bool exist = 1;
			for (short i = 0; exist; ++i) {
				exist = 0;
				for (auto& j : node) {
					if (j.color != -1)
						continue;
					bool con = 0;
					for (auto& k : node)
						if (k.color == i && getDistance(j, k) <= m_SimulationParameters.InterferenceRange) {
							con = 1;
							break;
						}
					if (!con) {
						j.color = i;
						exist = 1;
					}
				}
			}
		};


		for (short i = 0, l; i < m_SensorNodes.size(); ++i) {
			float j, k;
			node.push_back(Node(m_SensorNodes[i].m_Position.X, m_SensorNodes[i].m_Position.Y, m_SensorNodes[i].m_Level, i));
		}

		findParent();
		findColor();

		//for (auto i : node) {
		//	cout << i.index << ":";
		//	for (auto j : i.route)
		//		cout << ' ' << j.index;
		//	cout << endl;
		//}

		for (int i = 0; i < node.size(); i++)
		{
			if (node[i].tier == 0)
			{
				m_SensorNodes[node[i].index].m_Parent = -1;
				m_SensorNodes[node[i].index].m_CurrentParent = -1;
			}
			else
			{
				if (node[i].route.size() > 0)
				{
					m_SensorNodes[node[i].index].m_Parent = node[i].route[0].index;
					m_SensorNodes[node[i].index].m_CurrentParent = m_SensorNodes[node[i].index].m_Parent;
				}
			}
			m_SensorNodes[node[i].index].m_Color = node[i].color;
			m_SensorNodes[node[i].index].m_CurrentColor = node[i].color;
		}
#endif


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
			if (m_SensorNodes[i].m_CurrentParent == -1)
				continue;

			if (m_SensorNodes[i].m_CurrentParent == -2)
				throw std::runtime_error("An SN does not have any parents !");

			children[m_SensorNodes[i].m_CurrentParent].push_back(i);
		}

		auto CalculatePW = [&](std::vector<double> deltasIn)
		{
			std::vector<double> CWs(m_SensorNodes.size());

			for (int i = m_SimulationParameters.LevelRadius.size() - 1; i >= 0; i--)
			{
				for (int j = 0; j < m_SensorNodes.size(); j++)
				{
					if (m_SensorNodes[j].m_Level != i)
						continue;

					std::vector<double> childrenCWs;
					for (int k = 0; k < children[j].size(); k++)
						//childrenPWs.push_back(PWs[children[j][k]]);
						childrenCWs.push_back(CWs[children[j][k]]);

					CWs[j] = PWSteadyStateFunc(deltasIn[j], m_SimulationParameters.TransferTime,
						m_SimulationParameters.FailureDistribution.m_Mean, m_SimulationParameters.RecoveryTime, childrenCWs);

					for (int k = 0; k < children[j].size(); k++)
						CWs[j] += CWs[children[j][k]];
				}
			}

			double BSTotal = 0.0;
			for (int i = 0; i < CWs.size(); i++)
			{
				if (m_SensorNodes[i].m_CurrentParent == -1)
					//BSTotal += PWs[i];
					BSTotal += CWs[i];
			}

			return BSTotal;
		};

		static constexpr double randomRangeHigh = 100000.0;
		static constexpr double randomRangeLow = 1.0;


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
					//if (particlesTemp[particle][dimension] > 100000.0)
					//	particlesTemp[particle][dimension] = 100000.0;
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

			//std::cout << "Iteration : " << iteration << '\n';
			//std::cout << "Delta = ( ";
			//for (int i = 0; i < swarmBestCoords.size(); i++)
			//	std::cout << swarmBestCoords[i] << ", ";
			//std::cout << " )\nPW = " << swarmBestValue << "\n--------------------------------------------------------------------------------------\n";
		}

		for (int i = 0; i < m_SensorNodes.size(); i++)
			m_SensorNodes[i].m_DeltaOpt = swarmBestCoords[i];
		m_SimulationResults.CWSNEfficiency = swarmBestValue / m_SensorNodes.size();

		std::cout << "Delta = ( ";
		for (int i = 0; i < swarmBestCoords.size(); i++)
			std::cout << swarmBestCoords[i] << ", ";
		std::cout << " )\nPW = " << swarmBestValue / m_SensorNodes.size() << "\n--------------------------------------------------------------------------------------\n";

		std::cout << "Done!\n";
	}

	void Simulation::Run()
	{
		double seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		InnerRun(SimulationType::FT_TDMA, seed);
		Reset();
		InnerRun(SimulationType::RR_TDMA, seed);
	}

	void Simulation::InnerRun(SimulationType simulationType, double seed)
	{
		std::mt19937_64 innerRNG(seed);

		static constexpr double failGenerationDurationMultiplier = 0.1;

		SimulationResults& sr = m_SimulationResults;
			
		struct WorkingStateTimestamp
		{
			uint64_t SNID;
			WorkingState State;
			double Timestamp;
		};

		std::vector<WorkingStateTimestamp> previousEvents;
		auto pqCompare = [](WorkingStateTimestamp left, WorkingStateTimestamp right) 
		{ 
			if (left.Timestamp > right.Timestamp)
				return true;
			else if (left.Timestamp < right.Timestamp)
				return false;

			return left.SNID < right.SNID;
		};

		std::priority_queue<WorkingStateTimestamp, std::vector<WorkingStateTimestamp>, decltype(pqCompare)> eventQueue(pqCompare);
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			eventQueue.push({ (uint64_t)i, WorkingState::Collection, 0.0 });
			previousEvents.push_back({ (uint64_t)i, WorkingState::Collection, 0.0 });
		}
		
		// Generating failure timestamps
		std::vector<std::vector<double>> SNsFailureTimestamps(m_SensorNodes.size());
		std::vector<int> SNsFailureTimestampsIterator(m_SensorNodes.size(), 0);
#if 1
		for(int i = 0; i < m_SensorNodes.size(); i++)
		{
			double currentTime = 0;
			double timeToNextFailure;
			while (currentTime < failGenerationDurationMultiplier * m_SimulationParameters.TotalDurationToBeTransferred)
			{
				timeToNextFailure = m_SimulationParameters.FailureDistribution.GenerateRandomNumber(innerRNG);
				currentTime += timeToNextFailure;
				// LOOK AT THIS
				sr.Failures.push_back({ (uint64_t)i, currentTime });
				SNsFailureTimestamps[i].push_back(currentTime);
				//eventQueue.push({ SNID, WorkingState::Recovery, currentTime });
			}
		}
#endif

		int colorCount = -1;
		for (int i = 0; i < m_SensorNodes.size(); i++)
			colorCount = std::max(colorCount, (int)m_SensorNodes[i].m_Color);
		colorCount++;
		std::cout << "colorCount = " << colorCount << '\n';

		double transferredTotalDuration = 0;
		double currentTime = 0.0;
		int failureCount = 0;

		int superSlotIterator = 0;

		bool condition = true;

		while (condition)
		{
			auto currentEvent = eventQueue.top();
			currentTime = currentEvent.Timestamp;
			auto& currentState = currentEvent.State;
			auto& currentSN = currentEvent.SNID;
			eventQueue.pop();

			//std::cout << "Current Time = " << currentTime << '\n';

			superSlotIterator = currentTime / (m_SimulationParameters.TransferTime * colorCount);

			//std::cout << "here = " << currentSN << '\n';
			//if(eventQueue.size() > 99)
			//	std::cout << "eventQueue.size() " << eventQueue.size() << '\n';


			// deciding the next state to put in eventQueue
			{
				double nextTime = currentTime;
				WorkingState nextState;
				if (currentState == WorkingState::Collection)
				{
					if (simulationType == SimulationType::FT_TDMA)
					{
						//double optimalTime = currentTime + m_SensorNodes[currentSN].m_DeltaOpt;
						//double clammedOptimalTime = (int)((currentTime) / (m_SensorNodes[currentSN].m_DeltaOpt + m_SimulationParameters.TransferTime)) * (m_SensorNodes[currentSN].m_DeltaOpt + m_SimulationParameters.TransferTime) + m_SensorNodes[currentSN].m_DeltaOpt;
						

#if 0
						// now newtime is equal to timeslotstart
						nextTime = (int)((currentTime + m_SensorNodes[currentSN].m_DeltaOpt) / (m_SimulationParameters.TransferTime * colorCount)) * m_SimulationParameters.TransferTime * colorCount + m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime;

						while(nextTime < currentTime)
							nextTime += m_SimulationParameters.TransferTime * colorCount;

						//while (std::abs(nextTime - (currentTime + m_SensorNodes[currentSN].m_DeltaOpt)) > 0.5 * m_SimulationParameters.TransferTime * colorCount &&
						//	std::abs(nextTime + m_SimulationParameters.TransferTime * colorCount - (currentTime + m_SensorNodes[currentSN].m_DeltaOpt)) < std::abs(nextTime - (currentTime + m_SensorNodes[currentSN].m_DeltaOpt)))
						while (std::abs(nextTime + m_SimulationParameters.TransferTime * colorCount - (currentTime + m_SensorNodes[currentSN].m_DeltaOpt)) < std::abs(nextTime - (currentTime + m_SensorNodes[currentSN].m_DeltaOpt)))
							nextTime += m_SimulationParameters.TransferTime * colorCount;
#elif 0
						
						
						//// int of the optimaliterator
						//double optimalTime = (int)((currentTime) / (m_SensorNodes[currentSN].m_DeltaOpt + m_SimulationParameters.TransferTime)) * (m_SensorNodes[currentSN].m_DeltaOpt + m_SimulationParameters.TransferTime) + m_SensorNodes[currentSN].m_DeltaOpt;
						//// int of the super time slot
						//nextTime = (int)((optimalTime) / (m_SimulationParameters.TransferTime * colorCount)) * m_SimulationParameters.TransferTime * colorCount + m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime;
						//while(nextTime < currentTime)
						//	nextTime += m_SimulationParameters.TransferTime * colorCount;
						//while(std::abs(nextTime - optimalTime) > std::abs(nextTime + m_SimulationParameters.TransferTime * colorCount - optimalTime))
						//	nextTime += m_SimulationParameters.TransferTime * colorCount;
#elif 0
						double optimalTime = currentTime + m_SensorNodes[currentSN].m_DeltaOpt;
						nextTime = m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime;
						if (nextTime < currentTime)
							nextTime += (int)((currentTime - nextTime) / (m_SimulationParameters.TransferTime * colorCount)) * m_SimulationParameters.TransferTime * colorCount;
						while (nextTime >= currentTime)
							nextTime -= m_SimulationParameters.TransferTime * colorCount;
						nextTime += m_SimulationParameters.TransferTime * colorCount;
						while(std::abs(nextTime - optimalTime) > std::abs(nextTime + m_SimulationParameters.TransferTime * colorCount - optimalTime))
							nextTime += m_SimulationParameters.TransferTime * colorCount;
#elif 1
						double optimalTime = currentTime + m_SensorNodes[currentSN].m_DeltaOpt;
						nextTime = m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime;
						if (nextTime < currentTime)
							nextTime += (int)((currentTime - nextTime) / (m_SimulationParameters.TransferTime * colorCount)) * m_SimulationParameters.TransferTime * colorCount;
						while (nextTime >= currentTime)
							nextTime -= m_SimulationParameters.TransferTime * colorCount;
						nextTime += m_SimulationParameters.TransferTime * colorCount;
						while (nextTime < optimalTime)
							nextTime += m_SimulationParameters.TransferTime * colorCount;
#endif

					}
					else if(simulationType == SimulationType::RR_TDMA)
					{
#if 0
						//nextTime = (int)((currentTime + m_SensorNodes[currentSN].m_DeltaOpt) / (m_SimulationParameters.TransferTime * colorCount)) * m_SimulationParameters.TransferTime * colorCount + m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime;

						//while (nextTime < currentTime)
						//	nextTime += m_SimulationParameters.TransferTime * colorCount;
#elif 1
						nextTime = m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime;
						if (nextTime < currentTime)
							nextTime += (int)((currentTime - nextTime) / (m_SimulationParameters.TransferTime * colorCount)) * m_SimulationParameters.TransferTime * colorCount;
						while (nextTime >= currentTime)
							nextTime -= m_SimulationParameters.TransferTime * colorCount;
						nextTime += m_SimulationParameters.TransferTime * colorCount;

#endif
						//if(nextTime < currentTime)
						//	std::cout << "NEXTTIME = " << nextTime << '\n';
					}
					nextState = WorkingState::Transfer;

					//std::cout << "a = " << nextTime << '\n';
					//std::cout << "b = " << m_SensorNodes[currentSN].m_CurrentColor * m_SimulationParameters.TransferTime << '\n';
					//std::cout << "c = " << (int)((currentTime + m_SensorNodes[currentSN].m_DeltaOpt) / (m_SimulationParameters.TransferTime * colorCount)) << '\n';
					//std::cout << "d = " << m_SensorNodes[currentSN].m_CurrentColor << '\n';
					//std::cout << "e = " << colorCount << '\n';
					//std::cout << "Next Slot = " << nextTime << '\n';
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
				}
			}

			// still no rerouting
			if (previousEvents[currentSN].State == WorkingState::Collection)
			{
				if (currentState == WorkingState::Collection) // handles the initialization
				{
					m_SensorNodes[currentSN].m_Packets.push_back({ currentSN, currentTime });
					m_SensorNodes[currentSN].m_CurrentPacketIterator = m_SensorNodes[currentSN].m_Packets.size() - 1;
				}
				else if (currentState == WorkingState::Transfer)
				{
					m_SensorNodes[currentSN].m_CollectionTime += currentTime - previousEvents[currentSN].Timestamp;
					m_SensorNodes[currentSN].m_CurrentData += currentTime - previousEvents[currentSN].Timestamp;
					m_SensorNodes[currentSN].m_EnergyConsumed += (currentTime - previousEvents[currentSN].Timestamp) * m_SimulationParameters.EnergyRateWorking + s_EnergyTransitionWorkingToTransfer;
					//m_SensorNodes[currentSN].m_Packets[m_SensorNodes[currentSN].m_CurrentPacketIterator].Size += currentTime - previousEvents[currentSN].Timestamp; // look at this
					m_SensorNodes[currentSN].m_Packets[m_SensorNodes[currentSN].m_CurrentPacketIterator].Size += currentTime - m_SensorNodes[currentSN].m_Packets[m_SensorNodes[currentSN].m_CurrentPacketIterator].InitialTimestamp; // look at this
				}
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
					m_SensorNodes[currentSN].m_EnergyConsumed += (currentTime - previousEvents[currentSN].Timestamp) * m_SimulationParameters.EnergyRateWorking;
					m_SensorNodes[currentSN].m_Packets.clear();
					m_SensorNodes[currentSN].m_CurrentPacketIterator = - 1;

					if (m_SimulationParameters.MCREnabled)
						RerouteMCRFailure(currentSN);
				}
			}
			else if (previousEvents[currentSN].State == WorkingState::Transfer)
			{
				if (currentState == WorkingState::Collection)
				{
					if(m_SensorNodes[currentSN].m_CurrentParent != -1)
					{
						if (previousEvents[m_SensorNodes[currentSN].m_CurrentParent].State != WorkingState::Recovery)
						{
							m_SensorNodes[m_SensorNodes[currentSN].m_CurrentParent].m_CurrentData += m_SensorNodes[currentSN].m_CurrentData;
							for (int i = 0; i < m_SensorNodes[currentSN].m_Packets.size(); i++)
								m_SensorNodes[m_SensorNodes[currentSN].m_CurrentParent].m_Packets.push_back(m_SensorNodes[currentSN].m_Packets[i]);
						}
					}
					else
					{
						transferredTotalDuration += m_SensorNodes[currentSN].m_CurrentData;
						for (int i = 0; i < m_SensorNodes[currentSN].m_Packets.size(); i++)
						{
							m_SensorNodes[m_SensorNodes[currentSN].m_Packets[i].InitialSNID].m_SentPacketTotalDelay += currentTime - m_SensorNodes[currentSN].m_Packets[i].InitialTimestamp;
							m_SensorNodes[m_SensorNodes[currentSN].m_Packets[i].InitialSNID].m_SentPacketCount++;

							m_SensorNodes[m_SensorNodes[currentSN].m_Packets[i].InitialSNID].m_TotalDataSent += m_SensorNodes[currentSN].m_Packets[i].Size;
						}

					}

					if (m_SensorNodes[currentSN].m_CurrentParent == -1 || previousEvents[m_SensorNodes[currentSN].m_CurrentParent].State != WorkingState::Recovery)
					{
						m_SensorNodes[currentSN].m_Packets.clear();
						m_SensorNodes[currentSN].m_CurrentData = 0;
					}

					m_SensorNodes[currentSN].m_WastedTime += m_SimulationParameters.TransferTime;
					m_SensorNodes[currentSN].m_EnergyConsumed += m_SimulationParameters.TransferTime * m_SimulationParameters.EnergyRateTransfer + s_EnergyTransitionTransferToWorking;
					m_SensorNodes[currentSN].m_Packets.push_back({ currentSN, currentTime });
					m_SensorNodes[currentSN].m_CurrentPacketIterator = m_SensorNodes[currentSN].m_Packets.size() - 1;
				}
				else if (currentState == WorkingState::Transfer) {} // not possible
				else if (currentState == WorkingState::Recovery) // WARNING : PARTIAL TRANSFER FAILS	
				{
					m_SensorNodes[currentSN].m_CurrentData = 0;
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
					m_SensorNodes[currentSN].m_EnergyConsumed += (currentTime - previousEvents[currentSN].Timestamp) * m_SimulationParameters.EnergyRateTransfer;
					m_SensorNodes[currentSN].m_Packets.clear();
					m_SensorNodes[currentSN].m_CurrentPacketIterator = - 1;

					if (m_SimulationParameters.MCREnabled)
						RerouteMCRFailure(currentSN);
				}
			}
			else if (previousEvents[currentSN].State == WorkingState::Recovery)
			{

				if (currentState == WorkingState::Collection)
				{
					m_SensorNodes[currentSN].m_WastedTime += m_SimulationParameters.RecoveryTime;
					m_SensorNodes[currentSN].m_Packets.push_back({ currentSN, currentTime });
					m_SensorNodes[currentSN].m_CurrentPacketIterator = m_SensorNodes[currentSN].m_Packets.size() - 1;
					
					if(m_SimulationParameters.MCREnabled)
						RerouteMCRRecover(currentSN);
				}
				else if (currentState == WorkingState::Transfer) {} // not possible
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
					m_SensorNodes[currentSN].m_Packets.clear();
					m_SensorNodes[currentSN].m_CurrentPacketIterator =  - 1;
					if (m_SimulationParameters.MCREnabled)
						RerouteMCRFailure(currentSN);
				}
			}

			// throw std::runtime_error("Exceeded the last failure point!");;
			previousEvents[currentSN] = currentEvent;
			
			//if (m_SensorNodes[currentSN].m_Packets.size() > 1000)
			//	std::cout << "m_SensorNodes[currentSN].m_Packets.size() = " << m_SensorNodes[currentSN].m_Packets.size() << '\n';


			//condition = transferredTotalDuration < m_SimulationParameters.TotalDurationToBeTransferred;

			//condition = false;
			//for (int i = 0; i < m_SensorNodes.size(); i++)
			//{
			//	if (m_SensorNodes[i].m_TotalDataSent <= m_SimulationParameters.TotalDurationToBeTransferred)
			//	{
			//		condition = true;
			//		break;
			//	}
			//}
			condition = currentTime < m_SimulationParameters.TotalDurationToBeTransferred;
		}

		sr.ActualTotalDuration = currentTime;
		sr.FinalFailureIndex = failureCount;

		//for (int i = 0; i < m_SensorNodes.size(); i++)
		//{
		//	std::cout << "SN " << i << "\tDelta = " << m_SensorNodes[i].m_DeltaOpt << '\t';
		//	std::cout << "Collection Time = " << m_SensorNodes[i].m_CollectionTime << '\t';
		//	std::cout << "Wasted Time = " << m_SensorNodes[i].m_WastedTime << '\t';
		//	std::cout << "EnergyConsumed = " << m_SensorNodes[i].m_EnergyConsumed << '\n';
		//}
		std::cout << "Actual Total Duration = " << sr.ActualTotalDuration << '\n';

#if not _DEBUG
		Database::GetDatabase()->Insert(m_SimulationID, m_SimulationParameters, m_SimulationResults, simulationType);
		Database::GetDatabase()->Insert(m_SimulationID, m_SensorNodes, simulationType);
#endif
	}

	void Simulation::Reset()
	{
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			m_SensorNodes[i].m_CollectionTime = 0;
			m_SensorNodes[i].m_CurrentData = 0;
			m_SensorNodes[i].m_WastedTime = 0;
			m_SensorNodes[i].m_EnergyConsumed = 0;
			m_SensorNodes[i].m_SentPacketTotalDelay = 0;
			m_SensorNodes[i].m_SentPacketCount = 0;
			m_SensorNodes[i].m_Packets.clear();
			m_SensorNodes[i].m_TotalDataSent = 0;
		}

		m_SimulationResults = SimulationResults();
		m_SimulationResults.ActualTotalDuration = 0;
		m_SimulationResults.FinalFailureIndex = 0;
		m_SimulationResults.Failures.clear();
	}


	void Simulation::RerouteMCRFailure(uint64_t ParentSNID)
	{
		static int callCount = 0;
		//std::cout << "RerouteMCRFailure callCount = " << callCount++ << '\n';

		struct TempSN
		{
			uint64_t SNID;
			SensorNode SN;
			int Connectivity;
			double Distance;
		};
		auto& SNParent = m_SensorNodes[ParentSNID];
		//int64_t SNGrandparentID = SNParent.m_CurrentParent;
		int64_t SNGrandparentID = SNParent.m_Parent;
		Position grandparentPos = (SNGrandparentID == -1 ? Position{ 0.0, 0.0 } : m_SensorNodes[SNGrandparentID].m_Position);


		std::vector<TempSN> children;
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_CurrentParent != ParentSNID)
				continue;
			int connectivity = 0;
			for (int j = 0; j < m_SensorNodes.size(); j++)
			{
				if (m_SensorNodes[j].m_CurrentParent != ParentSNID)
					continue;
				
				if (std::sqrt((m_SensorNodes[i].m_Position.X - m_SensorNodes[j].m_Position.X) * (m_SensorNodes[i].m_Position.X - m_SensorNodes[j].m_Position.X) +
					(m_SensorNodes[i].m_Position.Y - m_SensorNodes[j].m_Position.Y) * (m_SensorNodes[i].m_Position.Y - m_SensorNodes[j].m_Position.Y)) <= m_SimulationParameters.TransmissionRange)
					connectivity++;
			}

			double distance = std::sqrt((m_SensorNodes[i].m_Position.X - grandparentPos.X) * (m_SensorNodes[i].m_Position.X - grandparentPos.X) +
				(m_SensorNodes[i].m_Position.Y - grandparentPos.Y) * (m_SensorNodes[i].m_Position.Y - grandparentPos.Y));


			SensorNode temp;
			temp.m_Color = m_SensorNodes[i].m_Color;
			temp.m_CurrentColor = m_SensorNodes[i].m_CurrentColor;
			temp.m_Parent = m_SensorNodes[i].m_Parent;
			temp.m_CurrentParent = m_SensorNodes[i].m_CurrentParent;
			temp.m_Level = m_SensorNodes[i].m_Level;
			temp.m_Position = m_SensorNodes[i].m_Position;
			children.push_back({ (uint64_t)i, temp, connectivity, distance });
		}

		// verify this
		std::sort(children.begin(), children.end(), [](const TempSN& a, const TempSN& b)
			{
				if (a.Connectivity > b.Connectivity)
					return true;

				if (a.Connectivity < b.Connectivity)
					return false;

				if (a.Distance < b.Distance)
					return true;

				return false;
			});

		//for (int i = 0; i < children.size(); i++)
		//{
		//	std::cout << "i = " << i << "children[i].SNID = " << children[i].SNID << "Connectivity = " << children[i].Connectivity <<
		//		"Distance = " << children[i].Distance << '\n';
		//}

		auto newColor = SNParent.m_CurrentColor;

		int MCR_BP_ID = -2;

		for (int i = 0; i < children.size(); i++)
		{
			bool safe = true;

			if (children[i].Distance > m_SimulationParameters.TransmissionRange)
				safe = false;

			for (int j = 0; j < m_SensorNodes.size() && safe; j++)
			{
				if (std::sqrt((children[i].SN.m_Position.X - m_SensorNodes[j].m_Position.X) * (children[i].SN.m_Position.X - m_SensorNodes[j].m_Position.X) +
					(children[i].SN.m_Position.Y - m_SensorNodes[j].m_Position.Y) * (children[i].SN.m_Position.Y - m_SensorNodes[j].m_Position.Y)) <= m_SimulationParameters.InterferenceRange &&
					m_SensorNodes[j].m_CurrentColor == newColor)
					safe = false;
			}


			if (safe)
			{
				MCR_BP_ID = children[i].SNID;
				break;
			}
		}

		if (MCR_BP_ID != -2)
			std::cout << "!!!!!!!!!!!!!!!SUCCESS, SNID = " << MCR_BP_ID << "!!!!!!!!!!!!!!!\n";

		// setting currentparent

		if (MCR_BP_ID != -2)
		{
			for (int i = 0; i < children.size(); i++)
				m_SensorNodes[children[i].SNID].m_CurrentParent = MCR_BP_ID;
			if(MCR_BP_ID != ParentSNID)
				m_SensorNodes[MCR_BP_ID].m_CurrentColor = SNParent.m_CurrentColor;
			m_SensorNodes[MCR_BP_ID].m_CurrentParent = SNGrandparentID;
		}


		// recalculate deltaopt
		//CalculateSNDeltaOpts();

	}

	void Simulation::RerouteMCRRecover(uint64_t ParentSNID)
	{
		static int callCount = 0;
		//std::cout << "RerouteMCRRecover callCount = " << callCount++ << '\n';

		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_Parent == ParentSNID)
			{
				m_SensorNodes[i].m_CurrentParent = ParentSNID;
				m_SensorNodes[i].m_CurrentColor = m_SensorNodes[i].m_Color;
			}
		}

		//CalculateSNDeltaOpts();
	}

}