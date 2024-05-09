drop database if exists WSN;

create database WSN;
use WSN;

create table Simulation(
	SimulationID bigint unsigned not null,
    TotalDurationToBeTransferred double,
    TransferTime double,
    RecoveryTime double,
    primary key(SimulationID)
);

-- placeholder
insert into Simulation values(0, 0, 0, 0);

create table SimulationDistribution(
	SimulationID bigint unsigned not null,
    SimulationDistributionID bigint unsigned not null,
    DistributionType enum('Exponential', 'Gamma', 'Lognormal', 'Weibull', 'Normal', 'Uniform'),
	Mean double,
    Stddev double,
    Parameter1 double,
    Parameter2 double,
    
	primary key(SimulationID, SimulationDistributionID),
    foreign key(SimulationID) references Simulation(SimulationID)
);


create table FailureTimestamp(
	SimulationID bigint unsigned not null,
    SimulationDistributionID bigint unsigned not null,
    FailureNumber bigint not null,
    Timestamp_ double,
    
	primary key(SimulationID, SimulationDistributionID, FailureNumber),
    foreign key(SimulationID, SimulationDistributionID) references SimulationDistribution(SimulationID, SimulationDistributionID)
);

create table SimulationResult(
	SimulationID bigint unsigned not null,
    SimulationDistributionID bigint unsigned not null,
    SimulationAttemptID bigint not null,
    Delta double,
    CollectionTime double,
    WastedTime double,
	ActualTotalDuration double,
	FinalFailureIndex bigint,
    
	primary key(SimulationID, SimulationDistributionID, SimulationAttemptID),
	foreign key(SimulationID, SimulationDistributionID) references SimulationDistribution(SimulationID, SimulationDistributionID)
);


create table SimulationInterval(
	SimulationID bigint unsigned not null,
    SimulationDistributionID bigint unsigned not null,
    SimulationAttemptID bigint not null,
    IntervalNumber bigint not null,
    State enum('Collection', 'Transfer', 'Recovery'),
    StartTime double,
    EndTime double,

	primary key(SimulationID, SimulationDistributionID, SimulationAttemptID, IntervalNumber),
    foreign key(SimulationID, SimulationDistributionID, SimulationAttemptID) references SimulationResult(SimulationID, SimulationDistributionID, SimulationAttemptID)
);

select * from Simulation;
select * from SimulationDistribution;
select * from SimulationResult;
select * from SimulationInterval limit 10000;
