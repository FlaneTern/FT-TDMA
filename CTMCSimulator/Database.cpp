#include "PCH.h"
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

    struct BinarySemaphoreWrapper
    {
        std::binary_semaphore Semaphore;

        BinarySemaphoreWrapper()
            : Semaphore(1) {}
    };


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


    void Database::Insert(uint64_t simulationID, const SimulationParameters& sp)
    {
        try
        {
            static std::vector<BinarySemaphoreWrapper> bs(s_MaxThreadCount);
            static std::vector<sql::PreparedStatement*> preparedStatements(s_MaxThreadCount, s_Connection->prepareStatement(
                "Insert ignore into "
                "Simulation(SimulationID, TransferTime, RecoveryTime, MaxSimulationTime, SNCount, MaxLevel)"
                " values(?,?,?,?,?,?)"));

            bool inserted = false;
            while (!inserted)
            {
                int iterator = -1;
                for (int i = 0; i < bs.size(); i++)
                {
                    if (bs[i].Semaphore.try_acquire())
                    {
                        iterator = i;
                        break;
                    }
                }

                if (iterator != -1)
                {
                    preparedStatements[iterator]->setUInt64(1, simulationID);
                    preparedStatements[iterator]->setDouble(2, sp.TransferTime);
                    preparedStatements[iterator]->setDouble(3, sp.RecoveryTime);
                    preparedStatements[iterator]->setDouble(4, sp.MaxSimulationTime);
                    preparedStatements[iterator]->setUInt64(5, sp.SNCount);
                    preparedStatements[iterator]->setUInt64(6, sp.MaxLevel);

                    preparedStatements[iterator]->execute();
                    preparedStatements[iterator]->clearAttributes();
                    preparedStatements[iterator]->clearParameters();
                    s_Connection->commit();

                    bs[iterator].Semaphore.release();
                    inserted = true;
                }
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert SimulationParameters. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert SimulationParameters. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes)
    {
        try
        {
            static std::vector<BinarySemaphoreWrapper> bs(s_MaxThreadCount);
            static std::vector<sql::PreparedStatement*> preparedStatements(s_MaxThreadCount, s_Connection->prepareStatement(
                "Insert ignore into "
                "SensorNode(SimulationID, SensorNodeID, Parent, Level_)" + 
                CreateInsertString(4, sensorNodes.size())));  // THIS IS BAD !!! CAN ONLY RUN ONE VALUE FOR SNCOUNT!

            bool inserted = false;
            while (!inserted)
            {
                int iterator = -1;
                for (int i = 0; i < bs.size(); i++)
                {
                    if (bs[i].Semaphore.try_acquire())
                    {
                        iterator = i;
                        break;
                    }
                }

                if (iterator != -1)
                {
                    for (int i = 0; i < sensorNodes.size(); i++)
                    {
                        preparedStatements[iterator]->setUInt64(i * 4 + 1, simulationID);
                        preparedStatements[iterator]->setUInt64(i * 4 + 2, i);
                        preparedStatements[iterator]->setInt64(i * 4 + 3, sensorNodes[i].Parent);
                        preparedStatements[iterator]->setUInt64(i * 4 + 4, sensorNodes[i].Level);
                    }

                    preparedStatements[iterator]->execute();
                    preparedStatements[iterator]->clearAttributes();
                    preparedStatements[iterator]->clearParameters();
                    s_Connection->commit();

                    bs[iterator].Semaphore.release();
                    inserted = true;
                }
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert SensorNode. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert SensorNode. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, uint64_t resultID, const CTMCParameters& ctmcParams)
    {
        try
        {
            static std::vector<BinarySemaphoreWrapper> bs(s_MaxThreadCount);
            static std::vector<sql::PreparedStatement*> preparedStatements(s_MaxThreadCount, s_Connection->prepareStatement(
                "Insert ignore into "
                "CTMCParameter(SimulationID, ResultID, Type_, Level_, Value_)" + 
                CreateInsertString(5, 4 * ctmcParams.Delta.size()))); // THIS IS BAD !!! CAN ONLY RUN ONE VALUE FOR MAXLEVEL!

            bool inserted = false;
            while (!inserted)
            {
                int iterator = -1;
                for (int i = 0; i < bs.size(); i++)
                {
                    if (bs[i].Semaphore.try_acquire())
                    {
                        iterator = i;
                        break;
                    }
                }

                if (iterator != -1)
                {
                    for (int i = 0; i < ctmcParams.Delta.size(); i++)
                    {
                        preparedStatements[iterator]->setUInt64(i * 20 + 1, simulationID);
                        preparedStatements[iterator]->setUInt64(i * 20 + 2, resultID);
                        preparedStatements[iterator]->setString(i * 20 + 3, "Tau");
                        preparedStatements[iterator]->setUInt64(i * 20 + 4, i);
                        preparedStatements[iterator]->setDouble(i * 20 + 5, ctmcParams.Tau[i]);

                        preparedStatements[iterator]->setUInt64(i * 20 + 6, simulationID);
                        preparedStatements[iterator]->setUInt64(i * 20 + 7, resultID);
                        preparedStatements[iterator]->setString(i * 20 + 8, "Lambda");
                        preparedStatements[iterator]->setUInt64(i * 20 + 9, i);
                        preparedStatements[iterator]->setDouble(i * 20 + 10, ctmcParams.Lambda[i]);

                        preparedStatements[iterator]->setUInt64(i * 20 + 11, simulationID);
                        preparedStatements[iterator]->setUInt64(i * 20 + 12, resultID);
                        preparedStatements[iterator]->setString(i * 20 + 13, "Delta");
                        preparedStatements[iterator]->setUInt64(i * 20 + 14, i);
                        preparedStatements[iterator]->setDouble(i * 20 + 15, ctmcParams.Delta[i]);

                        preparedStatements[iterator]->setUInt64(i * 20 + 16, simulationID);
                        preparedStatements[iterator]->setUInt64(i * 20 + 17, resultID);
                        preparedStatements[iterator]->setString(i * 20 + 18, "Mu");
                        preparedStatements[iterator]->setUInt64(i * 20 + 19, i);
                        preparedStatements[iterator]->setDouble(i * 20 + 20, ctmcParams.Mu[i]);
                    }

                    preparedStatements[iterator]->execute();
                    preparedStatements[iterator]->clearAttributes();
                    preparedStatements[iterator]->clearParameters();
                    s_Connection->commit();

                    bs[iterator].Semaphore.release();
                    inserted = true;
                }
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "ctmcParams.Delta.size() = " << ctmcParams.Delta.size() << '\n';
            std::cout << "SQL Error in Insert CTMCParameters. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert CTMCParameters. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, uint64_t resultID, const SimulationResults& sr)
    {
        try
        {
            static std::vector<BinarySemaphoreWrapper> bs(s_MaxThreadCount);
            static std::vector<sql::PreparedStatement*> preparedStatements(s_MaxThreadCount, s_Connection->prepareStatement(
                "Insert ignore into "
                "Result(SimulationID, ResultID, TotalCollectionTime, TotalDataSentToBS)"
                " values(?,?,?,?)"));

            bool inserted = false;
            while (!inserted)
            {
                int iterator = -1;
                for (int i = 0; i < bs.size(); i++)
                {
                    if (bs[i].Semaphore.try_acquire())
                    {
                        iterator = i;
                        break;
                    }
                }

                if (iterator != -1)
                {
                    preparedStatements[iterator]->setUInt64(1, simulationID);
                    preparedStatements[iterator]->setUInt64(2, resultID);
                    preparedStatements[iterator]->setDouble(3, sr.TotalCollectionTime);
                    preparedStatements[iterator]->setDouble(4, sr.TotalDataSentToBS);


                    preparedStatements[iterator]->execute();
                    preparedStatements[iterator]->clearAttributes();
                    preparedStatements[iterator]->clearParameters();
                    s_Connection->commit();

                    bs[iterator].Semaphore.release();
                    inserted = true;
                }
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert SimulationResults. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert SimulationResults. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, uint64_t resultID, const std::vector<double>& stateTime)
    {
        try
        {
            static std::vector<BinarySemaphoreWrapper> bs(s_MaxThreadCount);
            static std::vector<sql::PreparedStatement*> preparedStatements(s_MaxThreadCount, s_Connection->prepareStatement(
                "Insert ignore into "
                "StateTime(SimulationID, ResultID, State, Time_)" + 
                CreateInsertString(4, stateTime.size()))); // THIS IS BAD !!! CAN ONLY RUN ONE VALUE FOR SNCOUNT!


            bool inserted = false;
            while (!inserted)
            {
                int iterator = -1;
                for (int i = 0; i < bs.size(); i++)
                {
                    if (bs[i].Semaphore.try_acquire())
                    {
                        iterator = i;
                        break;
                    }
                }

                if (iterator != -1)
                {
                    for (int i = 0; i < stateTime.size(); i++)
                    {
                        preparedStatements[iterator]->setUInt64(i * 4 + 1, simulationID);
                        preparedStatements[iterator]->setUInt64(i * 4 + 2, resultID);
                        preparedStatements[iterator]->setUInt64(i * 4 + 3, i);
                        preparedStatements[iterator]->setDouble(i * 4 + 4, stateTime[i]);
                    }

                    preparedStatements[iterator]->execute();
                    preparedStatements[iterator]->clearAttributes();
                    preparedStatements[iterator]->clearParameters();
                    s_Connection->commit();

                    bs[iterator].Semaphore.release();
                    inserted = true;
                }
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert stateTime. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert stateTime. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, uint64_t resultID, const std::vector<std::vector<double>>& transitionRateMatrix)
    {
        // no, just... no.
    }
}

