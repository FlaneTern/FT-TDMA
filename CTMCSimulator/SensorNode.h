#pragma once

namespace CTMCS
{
	enum class WorkingState
	{
		Collection = 0,
		Transfer,
		Recovery
	};

	std::string WorkingStateToString(const WorkingState& ws);

	struct SensorNode
	{
		int64_t Parent = -1;
		uint64_t Level = 0;
		double CurrentDataSize = 0.0;
	};
}
