#include "PCH.h"
#include "Simulation.h"



int main()
{
	CTMCS::SimulationParameters siParams;
	siParams.SNCount = 5;
	siParams.MaxLevel = 4;
	siParams.MaxSimulationTime = 1000;
	siParams.RecoveryTime = 30;
	siParams.TransferTime = 60;

	CTMCS::SimulationRunParameters siRunParams;
	siRunParams.BigDeltaLevelStart = { 100, 200, 400, 400 };
	siRunParams.BigDeltaLevelEnd = { 500, 600, 800, 800 };
	siRunParams.BigDeltaLevelStride = { 100, 100, 100, 100 };
	siRunParams.BigLambdaStart = 3600;
	siRunParams.BigLambdaEnd = 7200;
	siRunParams.BigLambdaStride = 3600;

	CTMCS::Simulation si(siParams);

	si.Run(siRunParams);
}