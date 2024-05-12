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
    std::mutex* Database::s_InsertionMutex = new std::mutex;

	static sql::Driver* s_Driver;
	static sql::Connection* s_Connection;

    static constexpr uint32_t c_BatchRowCount = 50000 / 20;


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
            static sql::PreparedStatement* preparedStatement = s_Connection->prepareStatement(
                "Insert ignore into "
                "Simulation(SimulationID, TransferTime, RecoveryTime, MaxSimulationTime, SNCount, MaxLevel)"
                " values(?,?,?,?,?,?)");


            preparedStatement->setUInt64(1, simulationID);
            preparedStatement->setDouble(2, sp.TransferTime);
            preparedStatement->setDouble(3, sp.RecoveryTime);
            preparedStatement->setDouble(4, sp.MaxSimulationTime);
            preparedStatement->setUInt64(5, sp.SNCount);
            preparedStatement->setUInt64(6, sp.MaxLevel);


            preparedStatement->execute();
            preparedStatement->clearAttributes();
            preparedStatement->clearParameters();
            s_Connection->commit();

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
            static sql::PreparedStatement* preparedStatement = s_Connection->prepareStatement(
                "Insert ignore into "
                "SensorNode(SimulationID, SensorNodeID, Parent, Level_)" + 
                CreateInsertString(4, sensorNodes.size()));  // THIS IS BAD !!! CAN ONLY RUN ONE VALUE FOR SNCOUNT!



            for (int i = 0; i < sensorNodes.size(); i++)
            {
                preparedStatement->setUInt64(i * 4 + 1, simulationID);
                preparedStatement->setUInt64(i * 4 + 2, i);
                preparedStatement->setInt64(i * 4 + 3, sensorNodes[i].Parent);
                preparedStatement->setUInt64(i * 4 + 4, sensorNodes[i].Level);
            }


            preparedStatement->execute();
            preparedStatement->clearAttributes();
            preparedStatement->clearParameters();
            s_Connection->commit();

        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert SensorNode. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert SensorNode. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<CTMCParameters>& ctmcParams)
    {
        try
        {
            static sql::PreparedStatement* preparedStatement = s_Connection->prepareStatement(
                "Insert ignore into "
                "CTMCParameter(SimulationID, ResultID, Type_, Level_, Value_)" +
                CreateInsertString(5, c_BatchRowCount));


            bool done = false;
            int resultIDIterator = 0;
            int ctmcParamsLevelIterator = 0;
            for (int batchStartingRow = 0; !done; batchStartingRow += c_BatchRowCount)
            {
                std::cout << "Inserting ctmcParams " << simulationID << ", " << resultIDIterator << ", " << ctmcParamsLevelIterator << '\n';
                std::cout << "Out of " << ctmcParams.size() << '\n';

                for (int currentBatchRow = 0; !done && currentBatchRow + 3 < c_BatchRowCount; currentBatchRow+= 4)
                {
                    preparedStatement->setUInt64(currentBatchRow * 5 + 1, simulationID);
                    preparedStatement->setUInt64(currentBatchRow * 5 + 2, resultID[resultIDIterator]);
                    preparedStatement->setString(currentBatchRow * 5 + 3, "Tau");
                    preparedStatement->setUInt64(currentBatchRow * 5 + 4, ctmcParamsLevelIterator);
                    preparedStatement->setDouble(currentBatchRow * 5 + 5, ctmcParams[resultIDIterator].Tau[ctmcParamsLevelIterator]);

                    preparedStatement->setUInt64(currentBatchRow * 5 + 6, simulationID);
                    preparedStatement->setUInt64(currentBatchRow * 5 + 7, resultID[resultIDIterator]);
                    preparedStatement->setString(currentBatchRow * 5 + 8, "Lambda");
                    preparedStatement->setUInt64(currentBatchRow * 5 + 9, ctmcParamsLevelIterator);
                    preparedStatement->setDouble(currentBatchRow * 5 + 10, ctmcParams[resultIDIterator].Lambda[ctmcParamsLevelIterator]);

                    preparedStatement->setUInt64(currentBatchRow * 5 + 11, simulationID);
                    preparedStatement->setUInt64(currentBatchRow * 5 + 12, resultID[resultIDIterator]);
                    preparedStatement->setString(currentBatchRow * 5 + 13, "Delta");
                    preparedStatement->setUInt64(currentBatchRow * 5 + 14, ctmcParamsLevelIterator);
                    preparedStatement->setDouble(currentBatchRow * 5 + 15, ctmcParams[resultIDIterator].Delta[ctmcParamsLevelIterator]);

                    preparedStatement->setUInt64(currentBatchRow * 5 + 16, simulationID);
                    preparedStatement->setUInt64(currentBatchRow * 5 + 17, resultID[resultIDIterator]);
                    preparedStatement->setString(currentBatchRow * 5 + 18, "Mu");
                    preparedStatement->setUInt64(currentBatchRow * 5 + 19, ctmcParamsLevelIterator);
                    preparedStatement->setDouble(currentBatchRow * 5 + 20, ctmcParams[resultIDIterator].Mu[ctmcParamsLevelIterator]);

                    ctmcParamsLevelIterator++;

                    if (ctmcParamsLevelIterator == ctmcParams[resultIDIterator].Delta.size())
                    {
                        resultIDIterator++;
                        ctmcParamsLevelIterator = 0;
                    }

                    if (resultIDIterator == resultID.size())
                        done = true;

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            preparedStatement->setUInt64(i * 5 + 1, 0);
                            preparedStatement->setUInt64(i * 5 + 2, 0);
                            preparedStatement->setNull(i * 5 + 3, sql::DataType::ENUM);
                            preparedStatement->setNull(i * 5 + 4, sql::DataType::BIGINT);
                            preparedStatement->setNull(i * 5 + 5, sql::DataType::DOUBLE);

                        }
                    }
                }

                for (int i = c_BatchRowCount - (c_BatchRowCount % 4); i < c_BatchRowCount; i++)
                {
                    preparedStatement->setUInt64(i * 5 + 1, 0);
                    preparedStatement->setUInt64(i * 5 + 2, 0);
                    preparedStatement->setNull(i * 5 + 3, sql::DataType::ENUM);
                    preparedStatement->setNull(i * 5 + 4, sql::DataType::BIGINT);
                    preparedStatement->setNull(i * 5 + 5, sql::DataType::DOUBLE);
                }

                preparedStatement->execute();
                preparedStatement->clearAttributes();
                preparedStatement->clearParameters();
                s_Connection->commit();
            }

        }
        catch (sql::SQLException& e)
        {
            //std::cout << "ctmcParams.Delta.size() = " << ctmcParams.Delta.size() << '\n';
            std::cout << "SQL Error in Insert CTMCParameters. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert CTMCParameters. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<SimulationResults>& sr)
    {
        try
        {
            static sql::PreparedStatement* preparedStatement = s_Connection->prepareStatement(
                "Insert ignore into "
                "Result(SimulationID, ResultID, TotalCollectionTime, TotalDataSentToBS)"
                + CreateInsertString(4, c_BatchRowCount));


            bool done = false;
            int resultIDIterator = 0;
            for (int batchStartingRow = 0; !done; batchStartingRow += c_BatchRowCount)
            {
                std::cout << "Inserting SimulationResults " << simulationID << ", " << resultIDIterator << '\n';
                std::cout << "Out of " << sr.size() << '\n';

                for (int currentBatchRow = 0; !done && currentBatchRow < c_BatchRowCount; currentBatchRow++)
                {
                    preparedStatement->setUInt64(currentBatchRow * 4 + 1, simulationID);
                    preparedStatement->setUInt64(currentBatchRow * 4 + 2, resultID[resultIDIterator]);
                    preparedStatement->setDouble(currentBatchRow * 4 + 3, sr[resultIDIterator].TotalCollectionTime);
                    preparedStatement->setDouble(currentBatchRow * 4 + 4, sr[resultIDIterator].TotalDataSentToBS);

                    resultIDIterator++;
                    if (resultIDIterator == resultID.size())
                        done = true;

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            preparedStatement->setUInt64(i * 4 + 1, 0);
                            preparedStatement->setUInt64(i * 4 + 2, 0);
                            preparedStatement->setNull(i * 4 + 3, sql::DataType::DOUBLE);
                            preparedStatement->setNull(i * 4 + 4, sql::DataType::DOUBLE);

                        }
                    }
                }

                preparedStatement->execute();
                preparedStatement->clearAttributes();
                preparedStatement->clearParameters();
                s_Connection->commit();
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert SimulationResults. Error message: " + std::string(e.what());
            //std::cout << "DEBUGIT " << debugit << '\n';
            throw std::runtime_error("SQL Error in Insert SimulationResults. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<std::vector<double>>& stateTime)
    {
        try
        {
            static std::vector<BinarySemaphoreWrapper> bs(s_MaxThreadCount);
            static sql::PreparedStatement* preparedStatement = s_Connection->prepareStatement(
                "Insert ignore into "
                "StateTime(SimulationID, ResultID, State, Time_)" + 
                CreateInsertString(4, c_BatchRowCount)); // THIS IS BAD !!! CAN ONLY RUN ONE VALUE FOR SNCOUNT!


            bool done = false;
            int resultIDIterator = 0;
            int stateTimeIterator = 0;
            for (int batchStartingRow = 0; !done; batchStartingRow += c_BatchRowCount)
            {
                std::cout << "Inserting SimulationResults " << simulationID << ", " << resultIDIterator << ", " << stateTimeIterator << '\n';
                std::cout << "Out of " << stateTime.size() << '\n';

                for (int currentBatchRow = 0; !done && currentBatchRow < c_BatchRowCount; currentBatchRow++)
                {
                    preparedStatement->setUInt64(currentBatchRow * 4 + 1, simulationID);
                    preparedStatement->setUInt64(currentBatchRow * 4 + 2, resultID[resultIDIterator]);
                    preparedStatement->setUInt64(currentBatchRow * 4 + 3, stateTimeIterator);
                    preparedStatement->setDouble(currentBatchRow * 4 + 4, stateTime[resultIDIterator][stateTimeIterator]);

                    resultIDIterator++;
                    if (resultIDIterator == resultID.size())
                        done = true;

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            preparedStatement->setUInt64(i * 4 + 1, 0);
                            preparedStatement->setUInt64(i * 4 + 2, 0);
                            preparedStatement->setUInt64(i * 4 + 3, 0);
                            preparedStatement->setNull(i * 4 + 4, sql::DataType::DOUBLE);

                        }
                    }
                }

                preparedStatement->execute();
                preparedStatement->clearAttributes();
                preparedStatement->clearParameters();
                s_Connection->commit();
            }
        }
        catch (sql::SQLException& e)
        {
            std::cout << "SQL Error in Insert stateTime. Error message: " + std::string(e.what());
            throw std::runtime_error("SQL Error in Insert stateTime. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, const std::vector<uint64_t>& resultID, const std::vector<std::vector<std::vector<double>>>& transitionRateMatrix)
    {
        // no, just... no.
    }
}

