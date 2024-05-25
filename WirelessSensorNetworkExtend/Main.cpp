#include "PCH.h"

#include "Simulation.h"


//static constexpr int s_TotalDurationToBeTransferred = 3600 * 24 * 90;
//static constexpr int s_TotalDurationToBeTransferred = 3600;
//static constexpr double s_TotalDurationToBeTransferred = 3600.0 * 24 * 90 * 100;
static constexpr double s_TotalDurationToBeTransferred = 3600.0 * 24 * 90;
static constexpr double s_TransferTime = 60;
//static constexpr double s_TransferTime = 60;
static constexpr double s_RecoveryTime = 30;

int main()
{

	std::vector<double> transferTimes =
	{
		30.0 * 1,
		30.0 * 2,
		30.0 * 10,
		30.0 * 50,
	};

	std::vector<double> means =
	{
		3600.0 * 1,
		3600.0 * 2,
		3600.0 * 10,
		3600.0 * 50,
	};

	for (int redo = 0; redo < 1; redo++)
	{
		//for (double transferTime = 30; transferTime <= 30 * 1001; transferTime *= 10)
		//for (double transferTime = 30; transferTime <= 30 * 50 * 50 * 51; transferTime *= 50)
		for (auto& transferTime : transferTimes)
		{

			//for (double totalDurationToBeTransferred = 3600 * 24; totalDurationToBeTransferred <= 3600 * 24 * 7; totalDurationToBeTransferred += 3600 * 24)
			for (double totalDurationToBeTransferred = 3600.0 * 24 * 90; totalDurationToBeTransferred <= 3600.0 * 24 * 90; totalDurationToBeTransferred += 3600.0 * 24 * 90)
			{
				for (double multiplier = 1; multiplier <= 1; multiplier *= 8)
				//for (double multiplier = 1; multiplier <= 8; multiplier *= 8)
				//for (double multiplier = 1; multiplier <= 1001; multiplier *= 10)
				//for (double multiplier = 1; multiplier <= 125001; multiplier *= 50)
				{
					//for (double mean = 3600; mean <= 3600; mean += 3600)
					//for (double mean = 100000; mean <= 100000; mean += 100000)
					//for (double mean = 3600; mean <= 7200; mean += 900)
					//for (double mean = 3600; mean <= 3600 * 8 + 100; mean *= 2)
					//for (double mean = 900; mean <= 2700; mean += 900)
					for (auto& mean : means)
					{
						//for (double stddev = 3600; stddev <= 3600; stddev += 3600)
						//for (double stddev = 3600; stddev <= 7200; stddev += 900)
						//for (double stddev = 3600; stddev <= 3600 * 8 + 100; stddev *= 2)
						//for (double stddev = 900; stddev <= 900; stddev += 900)
						//for (double stddev = 900; stddev <= 7200; stddev += 900)
						{
							//std::cout << "Starting :\t Redo : " << redo << ",\t Standard Deviation : " << stddev * multiplier << ",\t Mean : " << 3600 * multiplier << '\n';
							double currentMean = mean * multiplier;
							//double currentStddev = stddev * multiplier;
							double currentStddev = currentMean;

							std::vector<WSN::DistributionType> failTypes =
							{
								WSN::DistributionType::Exponential,
								//WSN::DistributionType::Gamma,
								//WSN::DistributionType::Lognormal,
								//WSN::DistributionType::Weibull,
							};

							for (auto failType : failTypes)
							{
								if (failType == WSN::DistributionType::Exponential && currentMean != currentStddev)
									continue;

								std::vector<std::vector<uint64_t>> levelSNCounts =
								{
									//{ 60 * 1, 30 * 1, 10 * 1 },
									//{ 60 * 1, 10 * 1, 30 * 1 },
									//{ 30 * 1, 60 * 1, 10 * 1 },
									//{ 30 * 1, 10 * 1, 60 * 1 },
									//{ 10 * 1, 60 * 1, 30 * 1 },
									{ 10 * 1, 30 * 1, 60 * 1 },
								};
						
								for (auto levelSNCount : levelSNCounts)
								{
									WSN::Distribution failDist(failType, currentMean, currentStddev);
									WSN::SimulationParameters sp =
									{
										totalDurationToBeTransferred,
										transferTime,
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
		}

	}

	return 0;
}