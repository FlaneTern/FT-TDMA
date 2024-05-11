#pragma once

namespace CTMCS
{
	enum State
	{
		Collection = 0,
		Transfer,
		Recovery
	};

	struct SensorNode
	{
		int64_t Parent = -1;
		uint64_t Level = 0;
		double CurrentDataSize = 0.0;
	};
}
