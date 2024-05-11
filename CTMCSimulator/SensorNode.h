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
		int Parent = -1;
		int Level = -1;
		double CurrentDataSize = 0.0;
	};
}
