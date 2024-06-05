use WSN12;

select * from SimulationRRTDMA;

select SimulationFTTDMA.SimulationID, SimulationFTTDMA.ActualTotalDuration, SimulationRRTDMA.ActualTotalDuration from SimulationFTTDMA inner join SimulationRRTDMA where SimulationRRTDMA.SimulationID = SimulationFTTDMA.SimulationID;

select sum(SimulationFTTDMA.ActualTotalDuration), sum(SimulationRRTDMA.ActualTotalDuration) from SimulationFTTDMA inner join SimulationRRTDMA where SimulationRRTDMA.SimulationID = SimulationFTTDMA.SimulationID and SimulationFTTDMA.FailureMean = 3600;

select sum(SimulationFTTDMA.ActualTotalDuration) / sum(SimulationRRTDMA.ActualTotalDuration) from SimulationFTTDMA inner join SimulationRRTDMA where SimulationRRTDMA.SimulationID = SimulationFTTDMA.SimulationID and SimulationFTTDMA.FailureMean = 7200;

select sum(SimulationFTTDMA.ActualTotalDuration) / sum(SimulationRRTDMA.ActualTotalDuration), SimulationFTTDMA.FailureMean from SimulationFTTDMA inner join SimulationRRTDMA where SimulationRRTDMA.SimulationID = SimulationFTTDMA.SimulationID group by SimulationFTTDMA.FailureMean;

select sum(SimulationFTTDMA.ActualTotalDuration), sum(SimulationRRTDMA.ActualTotalDuration), sum(SimulationFTTDMA.ActualTotalDuration) / sum(SimulationRRTDMA.ActualTotalDuration), SimulationFTTDMA.FailureMean from SimulationFTTDMA inner join SimulationRRTDMA where SimulationRRTDMA.SimulationID = SimulationFTTDMA.SimulationID group by SimulationFTTDMA.FailureMean;

select 
(select SimulationID, sum(TotalDataSent) from SensorNodeFTTDMA group by SimulationID), 
(select SimulationID, sum(TotalDataSent) from SensorNodeFTTDMA group by SimulationID), 
sum(SimulationRRTDMA.ActualTotalDuration), 
SimulationFTTDMA.FailureMean 
from SimulationFTTDMA inner join SimulationRRTDMA where SimulationRRTDMA.SimulationID = SimulationFTTDMA.SimulationID group by SimulationFTTDMA.FailureMean;

select avg(tdsf) as TotalDataSent_FTTDMA, avg(tdsr) as TotalDataSent_RRTDMA, avg(tdsf) / avg(tdsr), SimulationFTTDMA.FailureMean from
(select SimulationID as sif, sum(TotalDataSent) as tdsf from SensorNodeFTTDMA group by SimulationID) temp1
inner join
(select SimulationID as sir, sum(TotalDataSent) as tdsr from SensorNodeRRTDMA group by SimulationID) temp2
inner join SimulationFTTDMA
where sif = sir and sif = SimulationFTTDMA.SimulationID and SimulationFTTDMA.FailureMean is not null
group by SimulationFTTDMA.FailureMean;

select avg(tdsf) / avg(SimulationFTTDMA.ActualTotalDuration) as PercentageDataSent_FTTDMA, avg(tdsr) / avg(SimulationFTTDMA.ActualTotalDuration) as PercentageDataSent_RRTDMA, (avg(tdsf) / sum(SimulationFTTDMA.ActualTotalDuration)) / (avg(tdsr) / sum(SimulationFTTDMA.ActualTotalDuration)) as PercentageDataSent_FTTDMAoverPercentageDataSent_RRTDMA, SimulationFTTDMA.FailureMean, SimulationFTTDMA.TotalDurationToBeTransferred from
(select SimulationID as sif, sum(TotalDataSent) as tdsf from SensorNodeFTTDMA group by SimulationID) temp1
inner join
(select SimulationID as sir, sum(TotalDataSent) as tdsr from SensorNodeRRTDMA group by SimulationID) temp2
inner join SimulationFTTDMA
inner join SimulationRRTDMA
where sif = sir and sif = SimulationFTTDMA.SimulationID and SimulationFTTDMA.SimulationID = SimulationRRTDMA.SimulationID and SimulationFTTDMA.FailureMean is not null
group by SimulationFTTDMA.FailureMean, SimulationFTTDMA.TotalDurationToBeTransferred;

select count(*), FailureMean from SensorNodeFTTDMA 
inner join SimulationFTTDMA on SensorNodeFTTDMA.SimulationID = SimulationFTTDMA.SimulationID
where SensorNodeFTTDMA.DeltaOpt < 10
group by FailureMean;

select avg(mc) from (select SimulationID, max(color) + 1 as mc from SensorNodeFTTDMA group by SimulationID) a;


--  -------------------


select avg(tdsf) / ((avg(color) + 1) ) as PercentageDataSent_FTTDMA, avg(tdsr) / (3600.0 * 24 * 90) as PercentageDataSent_RRTDMA, (avg(tdsf) / sum(SimulationFTTDMA.ActualTotalDuration)) / (avg(tdsr) / sum(SimulationFTTDMA.ActualTotalDuration)) as PercentageDataSent_FTTDMAoverPercentageDataSent_RRTDMA, SimulationFTTDMA.FailureMean, SimulationFTTDMA.TotalDurationToBeTransferred from
(select SimulationID as sif, sum(TotalDataSent) as tdsf from SensorNodeFTTDMA group by SimulationID) temp1
inner join
(select SimulationID as sir, sum(TotalDataSent) as tdsr from SensorNodeRRTDMA group by SimulationID) temp2
inner join SimulationFTTDMA
inner join SimulationRRTDMA
where sif = sir and sif = SimulationFTTDMA.SimulationID and SimulationFTTDMA.SimulationID = SimulationRRTDMA.SimulationID and SimulationFTTDMA.FailureMean is not null
group by SimulationFTTDMA.FailureMean, SimulationFTTDMA.TotalDurationToBeTransferred;



select SimulationID as sif, sum(TotalDataSent) as tdsf from SensorNodeFTTDMA group by SimulationID;


select SimulationFTTDMA.SimulationID, SimulationFTTDMA.FailureMean, SimulationFTTDMA.TransferTime, (temp.maxcolor * TransferTime) / ((temp.maxcolor * TransferTime) + TransferTime), temp2.PercentageDataSent_FTTDMA, temp2.PercentageDataSent_RRTDMA, temp2.PercentageDataSent_FTTDMA / temp2.PercentageDataSent_RRTDMA
from SimulationFTTDMA
inner join (select max(color) + 1 as maxcolor, SimulationID from SensorNodeFTTDMA group by SimulationID) temp
on temp.SimulationID = SimulationFTTDMA.SimulationID
inner join 
(
select sif, (tdsf) / (SimulationFTTDMA.TotalDurationToBeTransferred * 100) as PercentageDataSent_FTTDMA, avg(tdsr) / (SimulationRRTDMA.TotalDurationToBeTransferred * 100) as PercentageDataSent_RRTDMA, (avg(tdsf) / sum(SimulationFTTDMA.ActualTotalDuration)) / (avg(tdsr) / sum(SimulationFTTDMA.ActualTotalDuration)) as PercentageDataSent_FTTDMAoverPercentageDataSent_RRTDMA, SimulationFTTDMA.FailureMean, SimulationFTTDMA.TotalDurationToBeTransferred from
(select SimulationID as sif, sum(TotalDataSent) as tdsf from SensorNodeFTTDMA group by SimulationID) temp3
inner join
(select SimulationID as sir, sum(TotalDataSent) as tdsr from SensorNodeRRTDMA group by SimulationID) temp4
inner join SimulationFTTDMA
inner join SimulationRRTDMA
where sif = sir and sif = SimulationFTTDMA.SimulationID and SimulationFTTDMA.SimulationID = SimulationRRTDMA.SimulationID and SimulationFTTDMA.FailureMean is not null
group by SimulationFTTDMA.SimulationID
) temp2
on SimulationFTTDMA.SimulationID = temp2.sif;

select max(color) + 1 as maxcolor, SimulationID from SensorNodeFTTDMA group by SimulationID;

-- data efficiency
select SimulationFTTDMA.SimulationID, SimulationFTTDMA.FailureMean, SimulationFTTDMA.TransferTime, temp2.PercentageDataSent_FTTDMA, temp2.PercentageDataSent_RRTDMA, temp2.PercentageDataSent_FTTDMA / temp2.PercentageDataSent_RRTDMA
from SimulationFTTDMA
inner join 
(
select sif, (tdsf) / SimulationFTTDMA.TotalDurationToBeTransferred as PercentageDataSent_FTTDMA, avg(tdsr) / SimulationRRTDMA.TotalDurationToBeTransferred as PercentageDataSent_RRTDMA, (avg(tdsf) / sum(SimulationFTTDMA.ActualTotalDuration)) / (avg(tdsr) / sum(SimulationFTTDMA.ActualTotalDuration)) as PercentageDataSent_FTTDMAoverPercentageDataSent_RRTDMA, SimulationFTTDMA.FailureMean, SimulationFTTDMA.TotalDurationToBeTransferred from
(select SimulationID as sif, sum(TotalDataSent) as tdsf from SensorNodeFTTDMA group by SimulationID) temp3
inner join
(select SimulationID as sir, sum(TotalDataSent) as tdsr from SensorNodeRRTDMA group by SimulationID) temp4
inner join SimulationFTTDMA
inner join SimulationRRTDMA
where sif = sir and sif = SimulationFTTDMA.SimulationID and SimulationFTTDMA.SimulationID = SimulationRRTDMA.SimulationID and SimulationFTTDMA.FailureMean is not null
group by SimulationFTTDMA.SimulationID
) temp2
on SimulationFTTDMA.SimulationID = temp2.sif;


select SimulationFTTDMA.SimulationID, SimulationFTTDMA.EnergyRateWorking, SimulationFTTDMA.EnergyRateTransfer, sum(SensorNodeFTTDMA.EnergyConsumed), sum(SensorNodeRRTDMA.EnergyConsumed), SimulationFTTDMA.EnergyRateTransfer / SimulationFTTDMA.EnergyRateWorking, sum(SensorNodeFTTDMA.EnergyConsumed) / sum(SensorNodeFTTDMA.TotalDatasent), sum(SensorNodeRRTDMA.EnergyConsumed) / sum(SensorNodeRRTDMA.TotalDatasent)
from SimulationFTTDMA
inner join SensorNodeFTTDMA
inner join SensorNodeRRTDMA
on SimulationFTTDMA.SimulationID = SensorNodeFTTDMA.SimulationID and SimulationFTTDMA.SimulationID = SensorNodeRRTDMA.SimulationID
group by SimulationFTTDMA.SimulationID;