#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <limits>


enum State
{
	Working = 0,
	DataTransfer,
	Recovery
};

// Rates for level [i]	
static std::vector<double> TAU;
static std::vector<double> LAMBDA;
static std::vector<double> DELTA;
static std::vector<double> MU;

static constexpr double ReparationTime = 30.0;
static constexpr double TransferTime = 60.0;

static constexpr double MaxSimulationTime = 1000000.0;

static std::mt19937_64 s_Random = std::mt19937_64(std::chrono::high_resolution_clock::now().time_since_epoch().count());

struct SensorNode
{
	int Parent = -1;
	int Level = -1;
	double CurrentDataSize = 0.0;
};

int main()
{
	std::vector<int> BigDeltaLevelStart = { 100, 200, 400, 400 };
	std::vector<int> BigDeltaLevelEnd = { 500, 600, 800, 800 };
	std::vector<int> BigDeltaLevelStride = { 100, 100, 100, 100 };
	std::vector<int> BigDeltaLevelCurrent = BigDeltaLevelStart;

	int SNCount = 7;
	int maxLevel = 3;

	std::vector<SensorNode> SNs(SNCount);
	SNs[0].Level = 0;


	for (int i = 1; i < SNCount; i++)
	{
		int randomParent = s_Random() % i;
		while ((randomParent != i && SNs[randomParent].Level >= maxLevel) || randomParent == i)
			randomParent = s_Random() % i;

		if (randomParent == SNCount)
			SNs[i].Level = 0;
		else
		{
			SNs[i].Level = SNs[randomParent].Level + 1;
			SNs[i].Parent = randomParent;
		}
	}

	for (int i = 0; i < SNs.size(); i++)
		std::cout << "SN " << i << ", Parent : " << SNs[i].Parent << ", Level : " << SNs[i].Level << '\n';

	std::vector<double> SNCurrentDataSize(SNCount);


	std::vector<std::vector<int>> individualStates;
	for (int i = 0; i < std::pow(3, SNCount); i++)
	{
		std::vector<int> individualStatesCurrent(SNCount);
		int temp = i;
		int iterator = 0;
		do
		{
			individualStatesCurrent[iterator] = temp % 3;
			temp /= 3;
			iterator++;
		} while (temp > 0);

		individualStates.push_back(individualStatesCurrent);
	}


	for (int bigLambda = 3600; bigLambda <= 3600; bigLambda++)
	{
		bool done = false;
		while (!done)
		{
			// yes i know this is dumb
			TAU.clear();
			LAMBDA.clear();
			DELTA.clear();
			MU.clear();
			for (int i = 0; i < maxLevel + 1; i++)
			{
				TAU.push_back(1.0 / TransferTime);
				LAMBDA.push_back(1.0 / bigLambda);
				DELTA.push_back(1.0 / BigDeltaLevelCurrent[i]);
				MU.push_back(1.0 / (ReparationTime + 1.0 / (2 * DELTA.back())));
			}
			for (int i = 0; i < SNs.size(); i++)
				SNs[i].CurrentDataSize = 0.0;

			std::vector<std::vector<bool>> TransitionExists(std::pow(3, SNCount), std::vector<bool>(std::pow(3, SNCount), false));
			std::vector<std::vector<double>> TransitionRateMatrix(std::pow(3, SNCount), std::vector<double>(std::pow(3, SNCount)));
			std::vector<std::vector<std::exponential_distribution<double>>> ExponentialDists(std::pow(3, SNCount), std::vector<std::exponential_distribution<double>>(std::pow(3, SNCount)));

			for (int i = 0; i < std::pow(3, SNCount); i++)
			{
				for (int j = 0; j < std::pow(3, SNCount); j++)
				{
					if (i == j)
						continue;

					const std::vector<int>& individualStatesFrom = individualStates[i];
					const std::vector<int>& individualStatesTo = individualStates[j];

					bool rateIsZero = false;

					double rate = 1.0;

					for (int k = 0; k < individualStatesFrom.size() && !rateIsZero; k++)
					{
						if (individualStatesFrom[k] == individualStatesTo[k])
							continue;
						//else if (individualStatesTo[k] == DataTransfer && individualStatesTo[SNs[k].Parent] == Recovery)
						//	rateIsZero = true;
						else if (individualStatesFrom[k] == Working && individualStatesTo[k] == DataTransfer)
							rate *= DELTA[SNs[k].Level];
						else if (individualStatesFrom[k] == Working && individualStatesTo[k] == Recovery)
							rate *= LAMBDA[SNs[k].Level];
						else if (individualStatesFrom[k] == DataTransfer && individualStatesTo[k] == Working)
							rate *= TAU[SNs[k].Level];
						else if (individualStatesFrom[k] == Recovery && individualStatesTo[k] == Working)
							rate *= MU[SNs[k].Level];
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
					if(TransitionExists[i][j])
						ExponentialDists[i][j] = std::exponential_distribution<double>(TransitionRateMatrix[i][j]);
				}
			}

			std::vector<double> timeSpentInState(std::pow(3, SNCount));
			std::vector<double> randomTime(std::pow(3, SNCount));

			double totalDataSentToBS = 0.0;

			int currentState = 0;

			for (double currentTime = 0.0; currentTime < MaxSimulationTime; )
			{
				//std::cout << "CurrentState = " << currentState << '\n';
				for (int i = 0; i < TransitionRateMatrix.size(); i++)
				{
					if(TransitionExists[currentState][i])
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

				const std::vector<int>& individualStatesFrom = individualStates[currentState];
				const std::vector<int>& individualStatesTo = individualStates[nextState];
				for (int i = 0; i < SNs.size(); i++)
				{

					// TO DO !!!
					if (individualStatesFrom[i] == Working && individualStatesTo[i] == Working)
						SNs[i].CurrentDataSize += minimumTime;
					else if(individualStatesFrom[i] == Working && individualStatesTo[i] == DataTransfer)
						SNs[i].CurrentDataSize += minimumTime;
					else if (individualStatesFrom[i] == Working && individualStatesTo[i] == Recovery)
					{
						SNs[i].CurrentDataSize = 0.0;
						for (int j = 0; j < SNs.size(); j++)
						{
							if (i == j)
								continue;
							if (SNs[j].Parent == i && individualStatesFrom[j] == DataTransfer)
							{
								SNs[j].CurrentDataSize = 0; // DANGER : PARTIAL DATA TRANSFER FAILS
								// HANDLE TURNING CHILDREN TO WORKING IF FAILED  !!!!!!!!!!!
								//if(individualStatesTo[j] == DataTransfer)
								//	nextState -= std::pow(3, j) * 2;
							}
						}


					}
					else if(individualStatesFrom[i] == DataTransfer && individualStatesTo[i] == Working) 
					{
						if (SNs[i].Parent != -1)
							SNs[SNs[i].Parent].CurrentDataSize += SNs[i].CurrentDataSize;
						else
							totalDataSentToBS += SNs[i].CurrentDataSize;
						SNs[i].CurrentDataSize = 0.0;
					} 
					else if(individualStatesFrom[i] == DataTransfer && individualStatesTo[i] == DataTransfer) {} // DANGER : PARTIAL DATA TRANSFER FAILS
					else if(individualStatesFrom[i] == DataTransfer && individualStatesTo[i] == Recovery) {} // not possible
					else if(individualStatesFrom[i] == Recovery && individualStatesTo[i] == Working) {} // doesnt do anything
					else if(individualStatesFrom[i] == Recovery && individualStatesTo[i] == DataTransfer) {} // not possible
					else if(individualStatesFrom[i] == Recovery && individualStatesTo[i] == Recovery) {} // doesnt do anything
					
						
						
				}

				timeSpentInState[currentState] += minimumTime;
				currentState = nextState;
				currentTime += minimumTime;
				//std::cout << "current time  = " << currentTime << '\n';

			}

			double totalWorkingTime = 0.0;

			for (int i = 0; i < timeSpentInState.size(); i++)
			{	
				std::vector<int> individualStatesCurrent = individualStates[i];
				int workingCount = 0;
				for (int j = 0; j < individualStatesCurrent.size(); j++)
				{
					if (individualStatesCurrent[j] == 0)
						workingCount++;
				}
				totalWorkingTime += workingCount * timeSpentInState[i];
			}

			//for (int i = 0; i < TransitionRateMatrix.size(); i++)
			//{
			//	for (int j = 0; j < TransitionRateMatrix[0].size(); j++)
			//	{
			//		std::cout << TransitionRateMatrix[i][j] << ' ';
			//	}
			//	std::cout << '\n';
			//}

			std::cout << "BigDelta = ( " << BigDeltaLevelCurrent[0];
			for (int i = 1; i < BigDeltaLevelCurrent.size(); i++)
				std::cout << ", " << BigDeltaLevelCurrent[i];
			std::cout << " )\n";
			//for (int i = 0; i < timeSpentInState.size(); i++)
			//	std::cout << "Time spent in state " << i << " = " << timeSpentInState[i] << '\n';
			std::cout << "Total working time = " << totalWorkingTime << '\n';
			std::cout << "Total data sent to BS = " << totalDataSentToBS << '\n';
			std::cout << "-------------------------------------------------------\n";

			BigDeltaLevelCurrent[BigDeltaLevelCurrent.size() - 1] += BigDeltaLevelStride[BigDeltaLevelCurrent.size() - 1];
			for (int i = BigDeltaLevelCurrent.size() - 1; i >= 0; i--)
			{
				if (BigDeltaLevelCurrent[i] > BigDeltaLevelEnd[i])
				{
					if (i != 0)
					{
						BigDeltaLevelCurrent[i - 1] += BigDeltaLevelStride[i - 1];
						BigDeltaLevelCurrent[i] = BigDeltaLevelStart[i];
					}
					else
						done = true;
				}
			}
		}

	}
}