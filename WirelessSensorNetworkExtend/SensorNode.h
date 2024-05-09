#pragma once

namespace WSN
{
	struct Position
	{
		double X;
		double Y;
	};

	// Sensor Node states
	enum class WorkingState
	{
		Collection,
		Transfer,
		Recovery
	};

	std::string WorkingStateToString(const WorkingState& ws);

	class SensorNode
	{
	public:
		Position m_Position;
	};
}