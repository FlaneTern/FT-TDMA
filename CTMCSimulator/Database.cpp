#include "PCH.h"
#if 0
#include "Database.h"

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

namespace CTMCS
{
    // CONSTANTS
    static constexpr char c_Server[] = "tcp://127.0.0.1:3306\0";
    static constexpr char c_Username[] = "WSN\0";
    static constexpr char c_Password[] = "wsn123\0";
    static constexpr char c_DatabaseName[] = "CTMCS\0";


	// A LOT OF UNFREED STUFF such as driver, connection, and statements. Maybe free these?
    // Doesn't matter tbh

	Database* Database::s_DatabaseInstance = new Database();

	static sql::Driver* s_Driver;
	static sql::Connection* s_Connection;

    static constexpr uint32_t c_BatchRowCount = 50000 / 8;

    static std::string CreateInsertString(uint32_t argumentCount, uint32_t rowCount)
    {
        std::stringstream ss;
        ss << " values";

        for (int i = 0; i < rowCount; i++)
        {
            ss << "(?";
            for (int j = 1; j < argumentCount; j++)
                ss << ",?";
            ss << ")";
            if (i != rowCount - 1)
                ss << ',';
        }

        ss << ';';

        return ss.str();
    }


 	Database::Database()
	{
        try
        {
            s_Driver = get_driver_instance();
            s_Connection = s_Driver->connect(c_Server, c_Username, c_Password);
        }
        catch (sql::SQLException e)
        {
            throw std::runtime_error("Could not connect to server. Error message: " + std::string(e.what()));
        }

        s_Connection->setSchema(c_DatabaseName);

	}


    uint64_t Database::GetLatestSimulationID()
    {
        static sql::PreparedStatement* statement = s_Connection->prepareStatement("select max(SimulationID) from Simulation");
        sql::ResultSet* result = statement->executeQuery();

        result->next();
        return result->getUInt64(1);
    }
}

#endif