#include "PCH.h"
#include "Simulation.h"



int main()
{
#if 1
	{
		CTMCS::SimulationParameters siParams;
		siParams.SNCount = 5;
		siParams.MaxLevel = 0;
		siParams.MaxSimulationTime = 10000000;
		siParams.RecoveryTime = 30;
		siParams.TransferTime = 60;

		CTMCS::SimulationRunParameters siRunParams;
		siRunParams.BigDeltaLevelStart = { 200 };
		siRunParams.BigDeltaLevelEnd = { 800 };
		siRunParams.BigDeltaLevelStride = { 1 };
		siRunParams.BigLambdaStart = 3600;
		siRunParams.BigLambdaEnd = 3600;
		siRunParams.BigLambdaStride = 3600;

		if (siParams.MaxLevel + 1 != siRunParams.BigDeltaLevelEnd.size())
			throw std::runtime_error("DeltaLevel and MaxLevel Mismatch !");

		CTMCS::Simulation si(siParams);

		std::cout << "here\n";
		si.Run(siRunParams);
	}

	{
		CTMCS::SimulationParameters siParams;
		siParams.SNCount = 5;
		siParams.MaxLevel = 1;
		siParams.MaxSimulationTime = 10000000;
		siParams.RecoveryTime = 30;
		siParams.TransferTime = 60;

		CTMCS::SimulationRunParameters siRunParams;
		siRunParams.BigDeltaLevelStart = { 1, 200 };
		siRunParams.BigDeltaLevelEnd = { 800, 1000 };
		siRunParams.BigDeltaLevelStride = { 10, 10 };
		siRunParams.BigLambdaStart = 3600;
		siRunParams.BigLambdaEnd = 3600;
		siRunParams.BigLambdaStride = 3600;

		if (siParams.MaxLevel + 1 != siRunParams.BigDeltaLevelEnd.size())
			throw std::runtime_error("DeltaLevel and MaxLevel Mismatch !");

		CTMCS::Simulation si(siParams);

		si.Run(siRunParams);
	}

	{
		CTMCS::SimulationParameters siParams;
		siParams.SNCount = 5;
		siParams.MaxLevel = 2;
		siParams.MaxSimulationTime = 10000000;
		siParams.RecoveryTime = 30;
		siParams.TransferTime = 60;

		CTMCS::SimulationRunParameters siRunParams;
		siRunParams.BigDeltaLevelStart = { 20, 200, 200 };
		siRunParams.BigDeltaLevelEnd = { 800, 1000, 1000 };
		siRunParams.BigDeltaLevelStride = { 20, 20, 20 };
		siRunParams.BigLambdaStart = 3600;
		siRunParams.BigLambdaEnd = 3600;
		siRunParams.BigLambdaStride = 3600;

		if (siParams.MaxLevel + 1 != siRunParams.BigDeltaLevelEnd.size())
			throw std::runtime_error("DeltaLevel and MaxLevel Mismatch !");

		CTMCS::Simulation si(siParams);

		si.Run(siRunParams);
	}

#endif

#if 0
	{
		CTMCS::SimulationParameters siParams;
		siParams.SNCount = 5;
		siParams.MaxLevel = 2;
		siParams.MaxSimulationTime = 1000;
		siParams.RecoveryTime = 30;
		siParams.TransferTime = 60;

		CTMCS::SimulationRunParameters siRunParams;
		siRunParams.BigDeltaLevelStart = { 20, 200, 200 };
		siRunParams.BigDeltaLevelEnd = { 800, 1000, 1000 };
		siRunParams.BigDeltaLevelStride = { 100, 100, 100 };
		siRunParams.BigLambdaStart = 3600;
		siRunParams.BigLambdaEnd = 3600;
		siRunParams.BigLambdaStride = 3600;

		if (siParams.MaxLevel + 1 != siRunParams.BigDeltaLevelEnd.size())
			throw std::runtime_error("DeltaLevel and MaxLevel Mismatch !");

		CTMCS::Simulation si(siParams);

		si.Run(siRunParams);
	}
#endif
#if 0
	{
		CTMCS::SimulationParameters siParams;
		siParams.SNCount = 5;
		siParams.MaxLevel = 1;
		siParams.MaxSimulationTime = 1000;
		siParams.RecoveryTime = 30;
		siParams.TransferTime = 60;

		CTMCS::SimulationRunParameters siRunParams;
		siRunParams.BigDeltaLevelStart = { 20, 200 };
		siRunParams.BigDeltaLevelEnd = { 800, 1000 };
		siRunParams.BigDeltaLevelStride = { 100, 100 };
		siRunParams.BigLambdaStart = 3600;
		siRunParams.BigLambdaEnd = 3600;
		siRunParams.BigLambdaStride = 3600;

		if (siParams.MaxLevel + 1 != siRunParams.BigDeltaLevelEnd.size())
			throw std::runtime_error("DeltaLevel and MaxLevel Mismatch !");

		CTMCS::Simulation si(siParams);

		si.Run(siRunParams);
	}
#endif
}