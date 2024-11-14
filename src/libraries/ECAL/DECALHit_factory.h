/*
 *  File: DECALHit_factory.h
 *
 *  Created on 01/16/2024 by A.S.  
 */


#ifndef _DECALHit_factory_
#define _DECALHit_factory_

#include <vector>
using namespace std;

#include <JANA/JFactory.h>
#include "TTAB/DTranslationTable.h"

#include "DECALDigiHit.h"
#include "DECALHit.h"

typedef  vector< vector<double> >  ecal_constants_t;

class DECALHit_factory:public jana::JFactory<DECALHit>{
public:
  DECALHit_factory();
  ~DECALHit_factory(){};
  
  ecal_constants_t  gains;
  ecal_constants_t  pedestals;		
  ecal_constants_t  time_offsets;
  ecal_constants_t  adc_offsets;
  
  double adc_en_scale;
  double adc_time_scale;
  
  double base_time;

  bool isBlockActive( int row, int column ) const {
    if( row < 0 ||  row >= kECALBlocksTall )return false;
    if( column < 0 ||  column >= kECALBlocksWide )return false;
    
    return m_activeBlock[row][column];    
  }

private:
  jerror_t init(void);						///< Called once at program start.2
  jerror_t brun(jana::JEventLoop *eventLoop, int32_t runnumber);	///< Called everytime a new run number is detected.
  jerror_t evnt(jana::JEventLoop *eventLoop, uint64_t eventnumber);	///< Called every event.
  jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
  jerror_t fini(void);						///< Called after last event of last event source has been processed.
  
  void LoadECALConst(ecal_constants_t &table,
		     const vector<double> &ecal_const_ch);    

  static const int kECALBlocksWide   = 40;
  static const int kECALBlocksTall   = 40;
  static const int kECALMidBlock     = 20;
  static const int kECALMaxChannels  = kECALBlocksWide * kECALBlocksTall;

  unsigned int DB_PEDESTAL;
  unsigned int HIT_DEBUG;

  bool   m_activeBlock[kECALBlocksTall][kECALBlocksWide];
  int    m_channelNumber[kECALBlocksTall][kECALBlocksWide];
  int    m_row[kECALMaxChannels];
  int    m_column[kECALMaxChannels];
  int    m_numActiveBlocks;
};

#endif // _DECALHit_factory_

