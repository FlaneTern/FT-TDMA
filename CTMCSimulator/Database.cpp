#if 0
#include "CTMCSPCH.h"
#include "Database.h"

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>

namespace WSN
{
    // CONSTANTS
    static constexpr char c_Server[] = "tcp://127.0.0.1:3306\0";
    static constexpr char c_Username[] = "WSN\0";
    static constexpr char c_Password[] = "wsn123\0";
    static constexpr char c_DatabaseName[] = "WSN\0";


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

#if 0

    void Database::Insert(uint64_t simulationID, const SimulationParameters& simulationParameters)
    {
        
        try
        {
            static sql::PreparedStatement* statement = s_Connection->prepareStatement("Insert into \
                        Simulation(SimulationID, TotalDurationToBeTransferred, TransferTime, RecoveryTime) values(?, ?, ?, ?)");
            
            std::cout << "Inserting Simulation " << simulationID << '\n';

            statement->setUInt64(1, simulationID);
            statement->setDouble(2, simulationParameters.TotalDurationToBeTransferred);
            statement->setDouble(3, simulationParameters.TransferTime);
            statement->setDouble(4, simulationParameters.RecoveryTime);
            statement->execute();
        }
        catch (sql::SQLException& e)
        {
            throw std::runtime_error("SQL Error in Insert SDD. Error message: " + std::string(e.what()));
        }
    }

    void Database::Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes)
    {

    }

    void Database::Insert(uint64_t simulationID, const std::vector<Distribution>& dists, const std::vector<SimulationDistributionData>& sdd)
    {
        if (dists.size() != sdd.size())
            throw std::runtime_error("Dists size is not equal to sdd size in Database::Insert !");

        try
        {
            static sql::PreparedStatement* statement1 = s_Connection->prepareStatement("Insert into \
                        SimulationDistribution(SimulationID, SimulationDistributionID, DistributionType, Mean, Stddev, Parameter1, Parameter2) \
                        values(?, ?, ?, ?, ?, ?, ?)");

            static sql::PreparedStatement* statement2 = s_Connection->prepareStatement("Insert into \
                        FailureTimestamp(SimulationID, SimulationDistributionID, FailureNumber, Timestamp_) \
                        values(?, ?, ?, ?)");

            static sql::PreparedStatement* statement3 = s_Connection->prepareStatement("Insert into \
                        SimulationResult(SimulationID, SimulationDistributionID, SimulationAttemptID, Delta, CollectionTime, WastedTime, ActualTotalDuration, FinalFailureIndex) \
                        values(?, ?, ?, ?, ?, ?, ?, ?)");

            static sql::PreparedStatement* statement4 = s_Connection->prepareStatement("Insert into \
                        SimulationInterval(SimulationID, SimulationDistributionID, SimulationAttemptID, IntervalNumber, State, StartTime, EndTime) \
                        values(?, ?, ?, ?, ?, ?, ?)");

            for (int i = 0; i < dists.size(); i++)
            {
                std::cout << "Inserting SimulationDistribution " << simulationID << ", " << i <<  '\n';
                statement1->setUInt64(1, simulationID);
                statement1->setUInt64(2, i);
                statement1->setString(3, DistributionTypeToString(dists[i].m_DistributionType));
                statement1->setDouble(4, dists[i].m_Mean);
                statement1->setDouble(5, dists[i].m_Stddev);
                statement1->setDouble(6, dists[i].m_Parameter1);
                statement1->setDouble(7, dists[i].m_Parameter2);
                statement1->execute();

                for (int j = 0; j < sdd[i].FailureTimestamps.size() && j < sdd[i].SRD[j].FinalFailureIndex; j++)
                {   
                    std::cout << "Inserting FailureTimestamp " << simulationID << ", " << i << ", " << j << '\n';

                    statement2->setUInt64(1, simulationID);
                    statement2->setUInt64(2, i);
                    statement2->setUInt64(3, j);
                    statement2->setUInt64(4, sdd[i].FailureTimestamps[j]);
                    statement2->execute();
                }

                for (int j = 0; j < sdd[i].SRD.size(); j++)
                {
                    std::cout << "Inserting SimulationResult " << simulationID << ", " << i << ", " << j << '\n';

                    auto& srd = sdd[i].SRD[j];
                    statement3->setUInt64(1, simulationID);
                    statement3->setUInt64(2, i);
                    statement3->setUInt64(3, j);
                    statement3->setDouble(4, srd.Delta);
                    statement3->setDouble(5, srd.CollectionTime);
                    statement3->setDouble(6, srd.WastedTime);
                    statement3->setDouble(7, srd.ActualTotalDuration);
                    statement3->setUInt64(8, srd.FinalFailureIndex);
                    statement3->execute();

                    for (int k = 0; k < srd.SimulationIntervals.size(); k++)
                    {
                        std::cout << "Inserting SimulationInterval " << simulationID << ", " << i << ", " << j << ", " << k << '\n';
                        
                        statement4->setUInt64(1, simulationID);
                        statement4->setUInt64(2, i);
                        statement4->setUInt64(3, j);
                        statement4->setUInt64(4, k);
                        statement4->setString(5, WorkingStateToString(srd.SimulationIntervals[k].State));
                        statement4->setDouble(6, srd.SimulationIntervals[k].StartTime);
                        statement4->setDouble(7, srd.SimulationIntervals[k].EndTime);
                        statement4->execute();
                    }

                }
            }

        }
        catch (sql::SQLException& e)
        {
            throw std::runtime_error("SQL Error in Insert SDD. Error message: " + std::string(e.what()));
        }
    }
#else
void Database::Insert(uint64_t simulationID, const SimulationParameters& simulationParameters)
{
    std::cout << "Inserting Simulation " << simulationID << '\n';
    try
    {
        static sql::PreparedStatement* statement = s_Connection->prepareStatement(
            "Insert into "
            "Simulation(SimulationID, TotalDurationToBeTransferred, TransferTime, RecoveryTime) values(?, ?, ?, ?)");

        std::cout << "Inserting Simulation " << simulationID << '\n';

        statement->setUInt64(1, simulationID);
        statement->setDouble(2, simulationParameters.TotalDurationToBeTransferred);
        statement->setDouble(3, simulationParameters.TransferTime);
        statement->setDouble(4, simulationParameters.RecoveryTime);
        statement->execute();
        s_Connection->commit();
    }
    catch (sql::SQLException& e)
    {
        throw std::runtime_error("SQL Error in Insert SDD. Error message: " + std::string(e.what()));
    }
}

void Database::Insert(uint64_t simulationID, const std::vector<SensorNode>& sensorNodes)
{

}

void Database::Insert(uint64_t simulationID, const std::vector<Distribution>& dists, const std::vector<SimulationDistributionData>& sdd)
{
    if (dists.size() != sdd.size())
        throw std::runtime_error("Dists size is not equal to sdd size in Database::Insert !");

    try
    {
        static sql::PreparedStatement* statement1 = s_Connection->prepareStatement(
            "Insert ignore into "
            "SimulationDistribution(SimulationID, SimulationDistributionID, DistributionType, Mean, Stddev, Parameter1, Parameter2)" + 
            CreateInsertString(7, c_BatchRowCount));

        static sql::PreparedStatement* statement2 = s_Connection->prepareStatement(
            "Insert ignore into "
            "FailureTimestamp(SimulationID, SimulationDistributionID, FailureNumber, Timestamp_)" +
            CreateInsertString(4, c_BatchRowCount));

        static sql::PreparedStatement* statement3 = s_Connection->prepareStatement(
            "Insert ignore into "
            "SimulationResult(SimulationID, SimulationDistributionID, SimulationAttemptID, Delta, CollectionTime, WastedTime, ActualTotalDuration, FinalFailureIndex)" +
            CreateInsertString(8, c_BatchRowCount));

        static sql::PreparedStatement* statement4 = s_Connection->prepareStatement(
            "Insert ignore into "
            "SimulationInterval(SimulationID, SimulationDistributionID, SimulationAttemptID, IntervalNumber, State, StartTime, EndTime)" +
            CreateInsertString(7, c_BatchRowCount));

#if 0
        for (int i = 0; i < dists.size(); i += c_BatchRowCount)
        {
            std::cout << "Inserting SimulationDistribution " << simulationID << ", " << i << '\n';
            for (int batchRow = 0; batchRow < c_BatchRowCount && batchRow + i < dists.size(); batchRow++)
            {
                statement1->setUInt64(batchRow * 7 + 1, simulationID);
                statement1->setUInt64(batchRow * 7 + 2, batchRow + i);
                statement1->setString(batchRow * 7 + 3, DistributionTypeToString(dists[batchRow + i].m_DistributionType));
                statement1->setDouble(batchRow * 7 + 4, dists[batchRow + i].m_Mean);
                statement1->setDouble(batchRow * 7 + 5, dists[batchRow + i].m_Stddev);
                statement1->setDouble(batchRow * 7 + 6, dists[batchRow + i].m_Parameter1);
                statement1->setDouble(batchRow * 7 + 7, dists[batchRow + i].m_Parameter2);
            }

            statement1->execute();
            statement1->clearAttributes();
            statement1->clearParameters();
        }
#endif

        {
            bool done = false;
            int DistIterator = 0;
            for (int batchStartingRow = 0; !done; batchStartingRow += c_BatchRowCount)
            {
                std::cout << "Inserting SimulationDistribution " << simulationID << ", " << batchStartingRow << '\n';
                std::cout << "Out of " << dists.size() << '\n';
                for (int currentBatchRow = 0; !done && currentBatchRow < c_BatchRowCount; currentBatchRow++)
                {
                    statement1->setUInt64(currentBatchRow * 7 + 1, simulationID);
                    statement1->setUInt64(currentBatchRow * 7 + 2, currentBatchRow + batchStartingRow);
                    statement1->setString(currentBatchRow * 7 + 3, DistributionTypeToString(dists[currentBatchRow + batchStartingRow].m_DistributionType));
                    statement1->setDouble(currentBatchRow * 7 + 4, dists[currentBatchRow + batchStartingRow].m_Mean);
                    statement1->setDouble(currentBatchRow * 7 + 5, dists[currentBatchRow + batchStartingRow].m_Stddev);
                    statement1->setDouble(currentBatchRow * 7 + 6, dists[currentBatchRow + batchStartingRow].m_Parameter1);
                    statement1->setDouble(currentBatchRow * 7 + 7, dists[currentBatchRow + batchStartingRow].m_Parameter2);

                    DistIterator++;
                    if (DistIterator == dists.size())
                        done = true;

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            statement1->setUInt64(i * 7 + 1, 0);
                            statement1->setUInt64(i * 7 + 2, 0);
                            statement1->setNull(i * 7 + 3, sql::DataType::ENUM);
                            statement1->setNull(i * 7 + 4, sql::DataType::DOUBLE);
                            statement1->setNull(i * 7 + 5, sql::DataType::DOUBLE);
                            statement1->setNull(i * 7 + 6, sql::DataType::DOUBLE);
                            statement1->setNull(i * 7 + 7, sql::DataType::DOUBLE);

                        }
                    }
                }

                statement1->execute();
                statement1->clearAttributes();
                statement1->clearParameters();
                s_Connection->commit();
            }
        }

        std::cout << "Inserting SimulationDistribution Done!\n";

        {
            bool done = false;
            int SDDIterator = 0;
            int FailureTimestampsIterator = 0;
            for (int batchStartingRow = 0; !done; batchStartingRow += c_BatchRowCount)
            {
                std::cout << "Inserting SimulationDistribution " << simulationID << ", " << batchStartingRow << '\n';

                for (int currentBatchRow = 0; !done && currentBatchRow < c_BatchRowCount; currentBatchRow++)
                {
                    statement2->setUInt64(currentBatchRow * 4 + 1, simulationID);
                    statement2->setUInt64(currentBatchRow * 4 + 2, SDDIterator);
                    statement2->setUInt64(currentBatchRow * 4 + 3, FailureTimestampsIterator);
                    statement2->setDouble(currentBatchRow * 4 + 4, sdd[SDDIterator].FailureTimestamps[FailureTimestampsIterator]);

                    FailureTimestampsIterator++;
                    if (FailureTimestampsIterator == sdd[SDDIterator].FailureTimestamps.size())
                    {
                        SDDIterator++;
                        FailureTimestampsIterator = 0;
                    }

                    if (SDDIterator == sdd.size())
                        done = true;

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            statement2->setUInt64(i * 4 + 1, 0);
                            statement2->setUInt64(i * 4 + 2, 0);
                            statement2->setNull(i * 4 + 3, sql::DataType::BIGINT);
                            statement2->setNull(i * 4 + 4, sql::DataType::DOUBLE);

                        }
                    }
                }

                statement2->execute();
                statement2->clearAttributes();
                statement2->clearParameters();
                s_Connection->commit();
            }
        }

        
        {
            bool done = false;
            int SDDIterator = 0;
            int SRDIterator = 0;

            for (int batchStartingRow = 0; !done; batchStartingRow += c_BatchRowCount)
            {
                std::cout << "Inserting SimulationResult " << simulationID << ", " << SDDIterator << ", " << SRDIterator << '\n';
                for (int currentBatchRow = 0; !done && currentBatchRow < c_BatchRowCount; currentBatchRow++)
                {

                    auto& srd = sdd[SDDIterator].SRD[SRDIterator];
                    statement3->setUInt64(currentBatchRow * 8 + 1, simulationID);
                    statement3->setUInt64(currentBatchRow * 8 + 2, SDDIterator);
                    statement3->setUInt64(currentBatchRow * 8 + 3, SRDIterator);
                    statement3->setDouble(currentBatchRow * 8 + 4, srd.Delta);
                    statement3->setDouble(currentBatchRow * 8 + 5, srd.CollectionTime);
                    statement3->setDouble(currentBatchRow * 8 + 6, srd.WastedTime);
                    statement3->setDouble(currentBatchRow * 8 + 7, srd.ActualTotalDuration);
                    statement3->setUInt64(currentBatchRow * 8 + 8, srd.FinalFailureIndex);

                    SRDIterator++;
                    if (SRDIterator == sdd[SDDIterator].SRD.size())
                    {
                        SDDIterator++;
                        SRDIterator = 0;
                    }

                    if (SDDIterator == sdd.size())
                        done = true;

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            statement3->setUInt64(i * 8 + 1, 0);
                            statement3->setUInt64(i * 8 + 2, 0);
                            statement3->setUInt64(i * 8 + 3, 0);
                            statement3->setNull(i * 8 + 4, sql::DataType::DOUBLE);
                            statement3->setNull(i * 8 + 5, sql::DataType::DOUBLE);
                            statement3->setNull(i * 8 + 6, sql::DataType::DOUBLE);
                            statement3->setNull(i * 8 + 7, sql::DataType::DOUBLE);
                            statement3->setNull(i * 8 + 8, sql::DataType::BIGINT);

                        }
                    }
                }

                statement3->execute();
                statement3->clearAttributes();
                statement3->clearParameters();
                s_Connection->commit();
            }
        }

        
        {
            bool done = false;
            int SDDIterator = 0;
            int SRDIterator = 0;
            int SIIterator = 0;

            for (int batchStartingRow = 0; !done; batchStartingRow++)
            {

                std::cout << "Inserting SimulationInterval " << simulationID << ", " << SDDIterator << ", " << SRDIterator << ", " << SIIterator << '\n';
                
                for (int currentBatchRow = 0; !done && currentBatchRow < c_BatchRowCount; currentBatchRow++)
                {
                    auto& srd = sdd[SDDIterator].SRD[SRDIterator];


                    statement4->setUInt64(currentBatchRow * 7 + 1, simulationID);
                    statement4->setUInt64(currentBatchRow * 7 + 2, SDDIterator);
                    statement4->setUInt64(currentBatchRow * 7 + 3, SRDIterator);
                    statement4->setUInt64(currentBatchRow * 7 + 4, SIIterator);
                    statement4->setString(currentBatchRow * 7 + 5, WorkingStateToString(srd.SimulationIntervals[SIIterator].State));
                    statement4->setDouble(currentBatchRow * 7 + 6, srd.SimulationIntervals[SIIterator].StartTime);
                    statement4->setDouble(currentBatchRow * 7 + 7, srd.SimulationIntervals[SIIterator].EndTime);
                    SIIterator++;
                    if (SIIterator == srd.SimulationIntervals.size())
                    {
                        SRDIterator++;
                        SIIterator = 0;

                        if (SRDIterator == sdd[SDDIterator].SRD.size())
                        {
                            SDDIterator++;
                            SRDIterator = 0;
                            if (SDDIterator == sdd.size())
                                done = true;
                        }
                    }

                    if (done)
                    {
                        for (int i = currentBatchRow + 1; i < c_BatchRowCount; i++)
                        {
                            statement4->setUInt64(i * 7 + 1, 0);
                            statement4->setUInt64(i * 7 + 2, 0);
                            statement4->setUInt64(i * 7 + 3, 0);
                            statement4->setUInt64(i * 7 + 4, 0);
                            statement4->setNull(i * 7 + 5, sql::DataType::ENUM);
                            statement4->setNull(i * 7 + 6, sql::DataType::DOUBLE);
                            statement4->setNull(i * 7 + 7, sql::DataType::DOUBLE);

                        }
                    }

                }
                statement4->execute();
                statement4->clearAttributes();
                statement4->clearParameters();
                s_Connection->commit();
            }

        }

    }
    catch (sql::SQLException& e)
    {
        std::cout << "SQL Error in Insert SDD. Error message: " + std::string(e.what());
        throw std::runtime_error("SQL Error in Insert SDD. Error message: " + std::string(e.what()));
    }
}

#endif


    uint64_t Database::GetLatestSimulationID()
    {
        static sql::PreparedStatement* statement = s_Connection->prepareStatement("select max(SimulationID) from Simulation");
        sql::ResultSet* result = statement->executeQuery();

        result->next();
        return result->getUInt64(1);
    }
}
#endif