drop database if exists WSN3;

create database WSN3;
use WSN3;

create table Simulation(
	SimulationID bigint unsigned not null,
    TotalDurationToBeTransferred double,
    TransferTime double,
    RecoveryTime double, 
    FailureDistributionType enum('Exponential', 'Gamma', 'Lognormal', 'Weibull', 'Normal', 'Uniform'),
    FailureMean double,
    FailureStddev double,
    FailureParameter1 double,
    FailureParameter2 double,
    ActualTotalDuration double,
    FinalFailureIndex bigint unsigned,
    CWSNEfficiency double,
    primary key(SimulationID)
);

create table SensorNode(
	SimulationID bigint unsigned not null,
    SensorNodeID bigint unsigned not null,
    PosX double,
    PosY double,
    Parent bigint,
    Level_ bigint unsigned,
    DeltaOpt double,
    CollectionTime double,
    WastedTime double,
    EnergyConsumed double,
    SentPacketTotalDelay double,
    SentPacketCount bigint unsigned,
    Color bigint unsigned,
    primary key(SimulationID, SensorNodeID),
    foreign key(SimulationID) references Simulation(SimulationID)
);

-- placeholder
insert into Simulation values(0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

select * from Simulation;
select * from SensorNode; -- where SimulationID > 54;

