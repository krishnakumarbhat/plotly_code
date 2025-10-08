

#ifndef DATA_PROXY
#define DATA_PROXY

#include<iostream>
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include<set>
class DataProxy
{

    DataProxy();
public:
    DataProxy(DataProxy&) = delete;
    DataProxy& operator=(DataProxy&) = delete;
    static DataProxy& getInstance();

    void CollectData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform,bool IsRECU);
    void Signal_DataCollectionComplete(const char* f_name,const char* pltfolder,int filecount, std::string reportpath);


    void AllocateContainerMem();
    void DeallocateContainerMem();

    Customer_T customerid;

    virtual ~DataProxy();

    bool veh_sil;
    bool veh_hil;
    bool sil_hil;

    bool trackerplot;
    bool detectionplot;
    bool scanindex_scaling;

    std::string vehiclefilename;
    std::string resimfilename;

    std::set<int> set_radarposition;
};
#endif

