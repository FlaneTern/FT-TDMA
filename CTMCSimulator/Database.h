#pragma once

namespace WSN
{
	class Database
	{
	public:
		Database(const Database&) = delete;

		inline static Database* GetDatabase() { return s_DatabaseInstance; }



		uint64_t GetLatestSimulationID();

	private:
		Database();

		static Database* s_DatabaseInstance;
	};
}

