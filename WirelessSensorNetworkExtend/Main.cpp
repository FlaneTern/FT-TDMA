#include "PCH.h"

#include "Simulation.h"


//static constexpr int s_TotalDurationToBeTransferred = 3600 * 24 * 90;
//static constexpr int s_TotalDurationToBeTransferred = 3600;
static constexpr double s_TotalDurationToBeTransferred = 3600.0 * 24 * 90 * 100;
static constexpr double s_TransferTime = 60;
static constexpr double s_RecoveryTime = 30;

int main()
{
	for (int redo = 0; redo < 1; redo++)
	{

		//for (double multiplier = 1; multiplier <= 8; multiplier *= 8)
		for (double multiplier = 1; multiplier <= 1; multiplier *= 8)
		{
			//for (double mean = 3600; mean <= 3600; mean += 3600)
			for (double mean = 3600; mean <= 7200; mean += 900)
			{
				//for (double stddev = 3600; stddev <= 3600; stddev += 3600)
				for (double stddev = 3600; stddev <= 7200; stddev += 900)
				//for (double stddev = 900; stddev <= 900; stddev += 900)
				//for (double stddev = 900; stddev <= 7200; stddev += 900)
				{
					//std::cout << "Starting :\t Redo : " << redo << ",\t Standard Deviation : " << stddev * multiplier << ",\t Mean : " << 3600 * multiplier << '\n';
					double currentMean = mean * multiplier;
					double currentStddev = stddev * multiplier;

					std::vector<WSN::DistributionType> failTypes =
					{
						//WSN::DistributionType::Exponential,
						//WSN::DistributionType::Gamma,
						//WSN::DistributionType::Lognormal,
						WSN::DistributionType::Weibull,
					};

					for (auto failType : failTypes)
					{
						if (failType == WSN::DistributionType::Exponential && currentMean != currentStddev)
							continue;

						std::vector<std::vector<uint64_t>> levelSNCounts =
						{
							{ 60 * 1, 30 * 1, 10 * 1 },
							//{ 60 * 1, 10 * 1, 30 * 1 },
							//{ 30 * 1, 60 * 1, 10 * 1 },
							//{ 30 * 1, 10 * 1, 60 * 1 },
							//{ 10 * 1, 60 * 1, 30 * 1 },
							//{ 10 * 1, 30 * 1, 60 * 1 },
						};
						
						for (auto levelSNCount : levelSNCounts)
						{
							WSN::Distribution failDist(failType, currentMean, currentStddev);
							WSN::SimulationParameters sp =
							{
								s_TotalDurationToBeTransferred,
								s_TransferTime,
								s_RecoveryTime,
								failDist,
								{ 50, 100, 150 },
								levelSNCount
							};

							WSN::Simulation* Si = new WSN::Simulation(sp);

							Si->Run();
							delete Si;
						}

					}
				}
			}
		}

	}

	return 0;
}