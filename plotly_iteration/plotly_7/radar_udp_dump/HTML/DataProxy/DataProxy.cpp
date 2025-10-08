#include "DataProxy.h"
//#include "../DataCollector/BMWLowDataCollector.h"
//#include "../DataCollector/HondaDataCollector.h"
//#include "../DataCollector/Gen5PlatformDataCollector.h"
#include "../DataCollector/STLA_FLR4P.h"
DataProxy::DataProxy()
{
}
DataProxy& DataProxy::getInstance()
{
	
		static DataProxy instance;
		return instance;
	
}

void DataProxy::CollectData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
	if (stream_num == 12)
	{
		cout << "\n DataProxy::CollectData platform " << platform << " custId " << custId << " stream_num " << stream_num;
	}
	//cout << "\n DataProxy::CollectData platform "<< platform<<" custId "<< custId<<"stream_num "<< stream_num<<" Radar_Posn "<<int(Radar_Posn);
	//cout << "\n CollectData radarpos " << int(Radar_Posn) << " cusrid " << custId;

	if (IsRECU==1)
	{
		customerid = BMW_SAT;
		FLR4P& flr4p = FLR4P::getInstance();
		//CollectTrackerData_BMW_MID

	    flr4p.CollectTrackerData_BMW_MID(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
		set_radarposition.insert(int(Radar_Posn));
	}

	if (fileCount==0)
	{
		//cout << "\nF1 DataProxy::CollectData";
	}

	if (fileCount == 1)
	{
		//cout << "\nF2 DataProxy::CollectData";
	}

	if (custId == BMW_LOW)
	{
		//BMWLowDataCollector& bmwlow_datacollector = BMWLowDataCollector::getInstance();
		//bmwlow_datacollector.CollectData(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId);
		customerid = custId;
	}
	
	if (custId == HONDA_GEN5)
	{
		//HondaDataCollector& honda_datacollector = HondaDataCollector::getInstance();
		//honda_datacollector.CollectData(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
		customerid = custId;

	}
	//STLA

	//GPO platform
	//if (custId == PLATFORM_GEN5)
	//{
	//	
	//	Gen5PlatformDataCollector& gpo_datacollector = Gen5PlatformDataCollector::getInstance();
	//	if (stream_num == C2_CORE_MASTER_STREAM)// || stream_num == e_DET_LOGGING_STREAM)
	//	{
	//		gpo_datacollector.CollectData(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
	//	}
	//	
	//	customerid = custId;
	//}

	//if (g_pIRadarStrm)
	//{
	//	FLR4P& flr4p = FLR4P::getInstance();

	//	if (platform == UDP_PLATFORM_SRR6 || platform == UDP_PLATFORM_SRR6_PLUS || platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN || platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || platform == UDP_PLATFORM_STLA_FLR4)
	//	{

	//		if (stream_num == C2_CORE_MASTER_STREAM)// || stream_num == e_DET_LOGGING_STREAM)
	//		{

	//			if (trackerplot==1)
	//			{
	//				if (custId == HONDA_GEN5)
	//				{
	//					flr4p.CollectTrackerData_SRR6_Platform(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
	//					set_radarposition.insert(int(Radar_Posn));
	//					customerid = custId;
	//				}
	//			
	//				
	//			}
	//		

	//			if (detectionplot == 1)
	//			{
	//				if (custId == STLA_SRR6P)
	//				{


	//					flr4p.CollectDetectionData_SRR6_Platform(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
	//					set_radarposition.insert(int(Radar_Posn));
	//					customerid = custId;


	//				}
	//				if (custId == STLA_FLR4)  
	//				{

	//					flr4p.CollectDetectionData_FLR4_Platform(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
	//					set_radarposition.insert(int(Radar_Posn));
	//					customerid = custId;
	//				}
	//			}
	//		}

	//	

	//		
	//	}

	//}



	//if (g_pIRadarStrm)
	//{
	//if (platform == UDP_PLATFORM_FLR4_PLUS || platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
	//	{

	//		if (stream_num == e_TRACKER_LOGGING_STREAM)
	//		{
	//			
	//			FLR4P& flr4p = FLR4P::getInstance();
	//			if (trackerplot == 1)
	//			{
	//				
	//				flr4p.CollectTrackerData(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
	//				set_radarposition.insert(int(Radar_Posn));
	//			}
	//			
	//			
	//		}

	//		if ((stream_num == e_DET_LOGGING_STREAM))
	//		{
	//			if (detectionplot == 1)
	//			{    
	//				FLR4P& flr4p = FLR4P::getInstance();
	//				flr4p.CollectDetectionData(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
	//				set_radarposition.insert(int(Radar_Posn));
	//			}
	//		}
	//		customerid = STLA_FLR4P;
	//}
	//}
	//

	//if (custId == BMW_SAT)
	//{
	//	if (stream_num == 12)
	//	{
	//		//cout << "\n BMW_SAT";
	//	}

	//}
	
	
}

void DataProxy::Signal_DataCollectionComplete(const char* f_name, const char* pltfolder,int fcount,std::string reportpath)
{
	cout << "\n-----------------------------------------------------------------------------------------------";
	cout << "\nTrigger ---> Signal_DataCollectionComplete--from ---Core----to---DataProxy";
	cout << "\n-----------------------------------------------------------------------------------------------";
	//cout << "\nfname" << f_name;
	//if (customerid== BMW_LOW)
	//{
	//	//BMWLowDataCollector& bmwlow_datacollector = BMWLowDataCollector::getInstance();
	//	//bmwlow_datacollector.Generateplot(f_name, pltfolder, reportpath);
	//}

	//if (customerid == HONDA_GEN5)
	//{
	//	//HondaDataCollector& honda_datacollector = HondaDataCollector::getInstance();
	//	//honda_datacollector.Generateplot(f_name, pltfolder, reportpath);
	//}

	//if (customerid == PLATFORM_GEN5)
	//{
	//	Gen5PlatformDataCollector& gpo_datacollector = Gen5PlatformDataCollector::getInstance();
	//	gpo_datacollector.Generateplot(f_name, pltfolder, reportpath);

	//	//cout << "\nf_name " << f_name << endl;
	//	//cout << "\npltfolder" << pltfolder << endl;
	//	//cout << "\nreportpath " << reportpath << endl;
	//}

	//if (customerid == STLA_FLR4P)
	//{
	//	FLR4P& flr4p = FLR4P::getInstance();
	//	flr4p.Generateplot(f_name, pltfolder, reportpath);
	//}


	//if (customerid == STLA_SRR6P || customerid == HONDA_GEN5|| customerid == STLA_FLR4)
	//{
	//	FLR4P& flr4p = FLR4P::getInstance();
	//	flr4p.Generateplot(f_name, pltfolder, reportpath);
	//}

	//if (customerid == BMW_SAT)
	//{

	//	FLR4P& flr4p = FLR4P::getInstance();
	//	flr4p.Generateplot(f_name, pltfolder, reportpath);
	//}


	//

	//cout << "\n------------------------------------------------------------------------------------";

	//cout << "\n\t\tCompleted HTML Report for pair  "<< fcount+1;
	//cout << "\n------------------------------------------------------------------------------------";

}

void DataProxy::AllocateContainerMem()
{

	if (customerid == HONDA_GEN5)
	{
		//HondaDataCollector& honda_datacollector = HondaDataCollector::getInstance();
		//honda_datacollector.PlotParaMemAllocation();

	}
}

void DataProxy::DeallocateContainerMem()
{
	////cout << "\nDeallocateContainerMem";
	//if (customerid == PLATFORM_GEN5)
	//{
	//	////HondaDataCollector& honda_datacollector = HondaDataCollector::getInstance();
	//	////honda_datacollector.PlotParaMemDeallocation();
	//	////honda_datacollector.ref_SI_end = 0.00;
	//	////honda_datacollector.ref_SI_end = 0.00;
	//	//honda_datacollector.vFLscanindex.clear();
	//	//honda_datacollector.vFRscanindex.clear();
	//	//honda_datacollector.vRLscanindex.clear();
	//	//honda_datacollector.vRRscanindex.clear();

	//	Gen5PlatformDataCollector& gpo_datacollector = Gen5PlatformDataCollector::getInstance();
	//	gpo_datacollector.PlotParaMemDeallocation();

	//}

	//if (customerid == STLA_FLR4P|| customerid == STLA_SRR6P)
	//{
	//	FLR4P& flr4p = FLR4P::getInstance();
	//	flr4p.PlotParaMemDeallocation();
	//}


}

DataProxy::~DataProxy()
{
}
