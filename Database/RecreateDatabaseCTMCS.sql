drop database if exists CTMCS;

create database CTMCS;
use CTMCS;

create table Simulation(
	SimulationID bigint unsigned not null,
    TransferTime double,
    RecoveryTime double,
    MaxSimulationTime double,
	SNCount bigint unsigned,
    MaxLevel bigint unsigned,
    primary key(SimulationID)
);

create table SensorNode(
	SimulationID bigint unsigned not null,
    SensorNodeID bigint unsigned not null,
    Parent bigint,
    Level_ bigint unsigned,
    primary key(SimulationID, SensorNodeID),
    foreign key(SimulationID) references Simulation(SimulationID)
);

create table Result(
	SimulationID bigint unsigned not null,
    ResultID bigint unsigned not null,
    TotalCollectionTime double,
	TotalDataSentToBS double,
    primary key(SimulationID, ResultID),
    foreign key(SimulationID) references Simulation(SimulationID)
);

create table CTMCParameter(
	SimulationID bigint unsigned not null,
    ResultID bigint unsigned not null,
    Type_ enum('Tau', 'Lambda', 'Delta', 'Mu') not null,
	Level_ bigint unsigned not null,
    Value_ double,
    primary key(SimulationID, ResultID, Type_, Level_),
    foreign key(SimulationID, ResultID) references Result(SimulationID, ResultID)
);

create table StateTime(
	SimulationID bigint unsigned not null,
    ResultID bigint unsigned not null,
    State bigint unsigned not null,
	Time_ double,
    primary key(SimulationID, ResultID, State),
    foreign key(SimulationID, ResultID) references Result(SimulationID, ResultID)
);

create table TransitionRateMatrix(
	SimulationID bigint unsigned not null,
    ResultID bigint unsigned not null,
    StateFrom bigint unsigned not null,
    StateTo bigint unsigned not null,
    Rate double,
    primary key(SimulationID, ResultID, StateFrom, StateTo),
    foreign key(SimulationID, ResultID) references Result(SimulationID, ResultID)
);

-- placeholder
insert into Simulation values(0, 0, 0, 0, 0, 0);

select * from Simulation;
select * from SensorNode;
select * from Result;
select * from CTMCParameter;
select * from StateTime;
select * from TransitionRateMatrix;

select result.ResultID, CTMCParameter.Value_ from result inner join CTMCParameter on result.SimulationID = CTMCParameter.SimulationID
                       and result.ResultID = CTMCParameter.ResultID and CTMCParameter.Type_ = 'Delta' and CTMCParameter.Level_ = 0 and result.SimulationID = 1 order by result.ResultID


