#include "PCH.h"

#include "Simulation.h"


//static constexpr int s_TotalDurationToBeTransferred = 3600 * 24 * 90;
//static constexpr int s_TotalDurationToBeTransferred = 3600;
//static constexpr double s_TotalDurationToBeTransferred = 3600.0 * 24 * 90 * 100;
static constexpr double s_TotalDurationToBeTransferred = 3600.0 * 24 * 90;
//static constexpr double s_TransferTime = 60;
//static constexpr double s_TransferTime = 60;
static constexpr double s_RecoveryTime = 3600.0 * 24 * 1;

int main()
{
	// SN Counts for each level: 4
	//std::vector<WSN::Position> SNlocations =
	//{
	//	{15.0763, -43.2123},
	//	{12.9206, -5.03132},
	//	{24.2634, -18.2994},
	//	{-11.4823, 5.67111},
	//};

	std::vector<bool> MCREnabled = { true, false };

	// SN Counts for each level: 10 30 60
	std::vector<WSN::Position> SNlocations =
	{
		{-8.63456, 17.141},
		{-22.2503, 39.6321},
		{-25.8326, -13.5056},
		{20.5619, 9.85628},
		{14.8701, 43.2496},
		{5.1847, 23.8003},
		{33.5924, -14.5008},
		{-44.9129, -6.60031},
		{-1.51591, -4.17984},
		{-26.5768, 18.0577},
		{61.134, 16.1298},
		{58.9599, -7.54594},
		{53.3253, -22.2011},
		{26.1252, 44.826},
		{-59.4128, -53.0725},
		{-22.7203, 57.5048},
		{-79.1055, -49.0556},
		{-80.6684, -21.5811},
		{32.2696, 79.6381},
		{-66.7531, 70.9463},
		{-56.1097, 40.677},
		{28.8232, -44.7084},
		{-30.3963, 88.1454},
		{92.3704, 35.5975},
		{-43.6608, 53.5923},
		{-84.6816, 41.3541},
		{30.6593, -58.1475},
		{61.8205, -76.0183},
		{84.5572, 42.0356},
		{40.7375, 86.1318},
		{-52.1107, -0.078751},
		{54.8927, -39.7065},
		{64.3083, -69.3955},
		{-9.07909, -57.028},
		{7.27485, -49.54},
		{45.603, -56.5305},
		{-63.0891, -59.1852},
		{-75.0429, 14.656},
		{-13.0272, 86.056},
		{-38.7187, 53.7712},
		{-129.719, 47.7082},
		{-90.9119, 81.2567},
		{68.3988, 132.425},
		{102.176, -9.0956},
		{-90.8114, -116.409},
		{74.8359, -82.5399},
		{-0.235691, 145.981},
		{-41.7657, -141.68},
		{-70.5528, -127.539},
		{64.5399, -98.3343},
		{-67.9072, 130.151},
		{133.427, -19.2933},
		{110.908, -99.7421},
		{87.8436, 71.6043},
		{-38.5048, 121.813},
		{75.3487, 87.023},
		{-33.4186, -98.564},
		{96.069, -56.6639},
		{-97.4841, 91.8506},
		{-112.492, -5.09398},
		{77.3737, -93.6578},
		{-90.499, -61.8842},
		{125.005, 11.5431},
		{-25.5765, 97.1504},
		{-93.2378, -59.5642},
		{129.953, 0.85201},
		{65.4183, -97.3637},
		{-82.1185, -94.7613},
		{-129.622, -6.95138},
		{104.917, 70.7538},
		{40.8298, -122.603},
		{106.644, -102.833},
		{112.938, 11.6617},
		{55.6528, 109.744},
		{-89.4675, 81.6822},
		{-45.0922, 113.507},
		{4.00692, -126.191},
		{-107.613, 66.7218},
		{135.215, -38.1519},
		{-112.376, 44.2682},
		{123.873, 63.5652},
		{149.195, 10.1516},
		{13.4471, -121.807},
		{-9.86665, 135.212},
		{45.0304, 89.3411},
		{-82.6016, 120.234},
		{-100.207, -38.9705},
		{46.9079, -102.18},
		{107.695, -40.7498},
		{-91.6069, 73.7351},
		{102.14, -76.6726},
		{-127.523, 43.5994},
		{102.347, 33.1066},
		{124.088, 80.1918},
		{131.288, -11.2051},
		{-104.701, -106.727},
		{-25.8828, 116.081},
		{-143.433, 34.4211},
		{75.5551, -115.642},
		{-114.833, -37.5648},
	};

	// SN Counts for each level: 5 15 30
	//std::vector<WSN::Position> SNlocations =
	//{
	//	{-10.0373, -36.9948},
	//	{ -43.399, -10.915 },
	//	{ 40.4403, 12.854 },
	//	{ 32.7396, -25.967 },
	//	{ 39.996, 24.477 },
	//	{ -14.054, 61.1594 },
	//	{ 17.7775, -97.9849 },
	//	{ -12.9659, -70.9714 },
	//	{ 90.741, 25.1639 },
	//	{ 60.5863, 73.0635 },
	//	{ 26.7284, -93.2101 },
	//	{ -55.2893, -14.5562 },
	//	{ 49.6141, -8.19067 },
	//	{ -71.889, -8.69219 },
	//	{ 7.96687, -53.214 },
	//	{ -65.615, -14.7924 },
	//	{ -63.1501, 13.6051 },
	//	{ 85.5553, 11.6911 },
	//	{ -58.5221, -62.0905 },
	//	{ 51.7534, -51.7153 },
	//	{ 122.29, 27.784 },
	//	{ 5.97498, -110.695 },
	//	{ 51.3961, -137.378 },
	//	{ 75.2227, -99.4351 },
	//	{ 11.545, 105.867 },
	//	{ -125.584, 23.167 },
	//	{ 75.0295, -128.223 },
	//	{ 53.1245, 98.3592 },
	//	{ -96.6363, -32.8554 },
	//	{ -138.589, -49.7979 },
	//	{ 121.375, 85.9555 },
	//	{ 35.4721, 119.267 },
	//	{ -36.0808, -123.568 },
	//	{ 79.239, 124.634 },
	//	{ -75.0994, -123.741 },
	//	{ 81.9304, 57.959 },
	//	{ 38.8517, -108.665 },
	//	{ 107.861, 59.1374 },
	//	{ 6.97895, 147.25 },
	//	{ 19.953, 130.249 },
	//	{ -113.9, -85.11 },
	//	{ 5.2618, -144.045 },
	//	{ 7.64954, -148.284 },
	//	{ -75.0806, -67.1635 },
	//	{ 124.396, 70.1818 },
	//	{ 53.0977, 127.602 },
	//	{ 132.137, -2.14448 },
	//	{ -115.371, -44.5531 },
	//	{ -81.5515, 122.99 },
	//	{ -13.7635, -99.6917 },
	//};

	std::vector<double> transmissionRanges =
	{
		//10,
		//25,
		//50,
		//100,
		//200,
		//250,
		100
	};


	std::vector<double> interferenceRanges =
	{
		//10,
		//25,
		//50,
		//100,
		//200,
		//250,
		50
	};

	std::vector<double> transferTimes =
	{
		//30.0 * 1,
		//30.0 * 2,
		//30.0 * 10,
		//30.0 * 50,
		30.0 * 2,
	};

	std::vector<double> means =
	{
		//3600.0 * 1,
		//3600.0 * 2,
		//3600.0 * 10,
		//3600.0 * 50,
		3600.0 * 24 * 10,
		//3600.0 * 1,
		//3600.0 * 8,
	};



	std::vector<double> energyRateTransfers =
	{
		//0.05,
		//0.1,
		//0.2,
		//0.5,
		//1.0,
		//2.0,
		//5.0,
		//10.0,
		//20.0,
		//8.0,
		0.4,
	};

	std::vector<double> energyRateWorkings =
	{
		//20.0,
		//10.0,
		//5.0,
		//2.0,
		//1.0,
		//0.5,
		//0.2,
		//0.1,
		//0.05,
		//1.0,
		0.05,
	};

	//std::vector<double> energyRateWorkings;
	//for (int i = 0; i < energyRatioTransferOverWorking.size(); i++)
	//{
	//	energyRateTransfers.push_back(1.0);
	//	energyRateWorkings.push_back(energyRateTransfers[i] / energyRatioTransferOverWorking[i]);
	//}
	

	for (int redo = 0; redo < 8; redo++)
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
									//{ 10 * 1, 30 * 1, 60 * 1 },
									{ 20 * 1, 200 * 1, 2000 * 1 },
									//{ 5 * 1, 15 * 1, 30 * 1 },
									//{ 4 },
								};
						
								for (auto levelSNCount : levelSNCounts)
								{
									for (int energyRateIterator = 0; energyRateIterator < energyRateTransfers.size(); energyRateIterator++)
									{
										for (auto& transmissionRange : transmissionRanges)
										{
											for (auto& interferenceRange : interferenceRanges)
											{
												for (auto mcrEnabled : MCREnabled)
												{
													WSN::Distribution failDist(failType, currentMean, currentStddev);
													WSN::SimulationParameters sp =
													{
														totalDurationToBeTransferred,
														transferTime,
														s_RecoveryTime,
														failDist,
														{ 50, 100, 150 },
														//{ 50 },
														levelSNCount,
														energyRateWorkings[energyRateIterator],
														energyRateTransfers[energyRateIterator],
														transmissionRange,
														interferenceRange,
														mcrEnabled
													};

													//WSN::Simulation* Si = new WSN::Simulation(sp, SNlocations);
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
				}
			}
		}

	}

	return 0;
}