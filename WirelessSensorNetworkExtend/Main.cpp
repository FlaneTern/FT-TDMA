#include "WSNPCH.h"

#include "Simulation.h"


static constexpr int s_TotalDurationToBeTransferred = 3600 * 24 * 90;
static constexpr int s_TransferTime = 60;
static constexpr int s_RecoveryTime = 30;

int main()
{
	for (int redo = 0; redo < 3; redo++)
	{
		WSN::Distribution distX(WSN::DistributionType::Normal, 5, 2);
		WSN::Distribution distY(WSN::DistributionType::Normal, 5, 2);
		WSN::SimulationParameters sp =
		{
			s_TotalDurationToBeTransferred,
			s_TransferTime,
			s_RecoveryTime,
			{
				distX,
				distY
			}
		};

		WSN::Simulation* Si = new WSN::Simulation(sp);

		//for (double multiplier = 1; multiplier <= 8; multiplier *= 8)
		for (double multiplier = 1; multiplier <= 1; multiplier *= 8)
		{
			for (double stddev = 900; stddev <= 900; stddev += 900)
			//for (double stddev = 900; stddev <= 7200; stddev += 900)
			{
				//std::cout << "Starting :\t Redo : " << redo << ",\t Standard Deviation : " << stddev * multiplier << ",\t Mean : " << 3600 * multiplier << '\n';
				double currentMean = 3600 * multiplier;
				double currentStddev = stddev * multiplier;

				Si->AddFailureDistribution(WSN::Distribution(WSN::DistributionType::Gamma, currentMean, currentStddev));
				Si->AddFailureDistribution(WSN::Distribution(WSN::DistributionType::Lognormal, currentMean, currentStddev));
				Si->AddFailureDistribution(WSN::Distribution(WSN::DistributionType::Weibull, currentMean, currentStddev));

			}
		}

		Si->Run();
		delete Si;
	}

	return 0;
}