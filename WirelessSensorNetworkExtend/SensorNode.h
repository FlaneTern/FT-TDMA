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
		int64_t m_Parent;
		uint64_t m_Level;

		double m_DeltaOpt;

		double m_CurrentData = 0;

		double m_CollectionTime = 0;
		double m_WastedTime = 0;
	};
}