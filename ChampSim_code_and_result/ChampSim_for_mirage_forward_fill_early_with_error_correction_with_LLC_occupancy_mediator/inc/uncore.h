#ifndef UNCORE_H
#define UNCORE_H

#include "champsim.h"
#include "cache.h"
#include "dram_controller.h"
//#include "drc_controller.h"

//#define DRC_MSHR_SIZE 48

// uncore
class UNCORE {
  public:

    // LLC
    CACHE LLC{"LLC", LLC_SET, LLC_WAY, LLC_SET*LLC_WAY, LLC_WQ_SIZE, LLC_RQ_SIZE, LLC_PQ_SIZE, LLC_MSHR_SIZE};
	
	// Data LLC
	CACHE data_LLC{"data_LLC", data_LLC_SET, data_LLC_WAY, data_LLC_SET*data_LLC_WAY, data_LLC_WQ_SIZE, data_LLC_RQ_SIZE, data_LLC_PQ_SIZE, data_LLC_MSHR_SIZE};

    // DRAM
    MEMORY_CONTROLLER DRAM{"DRAM"}; 

    UNCORE(); 
};

extern UNCORE uncore;

#endif
