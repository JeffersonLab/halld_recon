/*
 *  File: DCCALHit_factory.cc
 *
 * Created on 11/25/18 by A.S. 
 * use structure similar to FCAL
 */

#include <iostream>
#include <fstream>
#include <math.h>
#include <DVector3.h>
using namespace std;

#include <JANA/JEvent.h>
using namespace jana;

#include "CCAL/DCCALShower.h"
#include "CCAL/DCCALShower_factory.h"
#include "CCAL/DCCALHit.h"
#include "CCAL/DCCALGeometry.h"
#include "HDGEOMETRY/DGeometry.h"

#include "hycal.h"

//----------------
// Constructor
//----------------
DCCALShower_factory::DCCALShower_factory()
{
	// Set defaults
        MIN_CLUSTER_BLOCK_COUNT   =  2;
        MIN_CLUSTER_SEED_ENERGY   =  0.035;  // GeV
	TIME_CUT                  =  15.0 ;  //ns
	MAX_HITS_FOR_CLUSTERING   =  200;
	SHOWER_DEBUG              =  0;

	gPARMS->SetDefaultParameter("CCAL:SHOWER_DEBUG",   SHOWER_DEBUG);
	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_BLOCK_COUNT", MIN_CLUSTER_BLOCK_COUNT);
	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_SEED_ENERGY", MIN_CLUSTER_SEED_ENERGY);
	gPARMS->SetDefaultParameter("CCAL:MAX_HITS_FOR_CLUSTERING", MAX_HITS_FOR_CLUSTERING);
	gPARMS->SetDefaultParameter("CCAL:TIME_CUT",TIME_CUT,"time cut for associating CCAL hits together into a cluster");

}

//------------------
// brun
//------------------
jerror_t DCCALShower_factory::brun(JEventLoop *eventLoop, int32_t runnumber)
{

	return NOERROR;
}


jerror_t DCCALShower_factory::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
  vector<const DCCALHit*> ccalhits;
	eventLoop->Get(ccalhits);
	
	if(ccalhits.size() > MAX_HITS_FOR_CLUSTERING) return NOERROR;
	
	double tot_en = 0;
	

	SET_EMIN   =  0.05;    // banks->CONFIG->config->CLUSTER_ENERGY_MIN;
	SET_EMAX   =  15.9;    // banks->CONFIG->config->CLUSTER_ENERGY_MAX;
	SET_HMIN   =  2;       // banks->CONFIG->config->CLUSTER_MIN_HITS_NUMBER;
	SET_MINM   =  0.08;    // banks->CONFIG->config->CLUSTER_MAX_CELL_MIN_ENERGY;
	ZHYCAL     =  1.e9;    // ALignment[1].al.z;
	ISECT      =  0;       //  PWO

	for(int icol = 1; icol <= MCOL; ++icol)
	  for(int irow = 1; irow <= MROW; ++irow) {
	    ECH(icol,irow) = 0;
	    STAT_CH(icol,irow) = 0;
	    if(icol>=17 && icol<=18 && irow>=17 && irow<=18) STAT_CH(icol,irow) = -1;
	  }
	
	NCOL = 34; NROW = 34;

	SET_XSIZE = CRYS_SIZE_X; SET_YSIZE = CRYS_SIZE_Y;
	
	for (unsigned int i = 0; i < ccalhits.size(); i++) {	  
	  const DCCALHit *ccalhit = ccalhits[i];

	  if(SHOWER_DEBUG == 1){
	    cout << "ccalhit row col e = " << ccalhit->row << " " << ccalhit->column << " " << ccalhit->E << endl;
	  }

	  tot_en += ccalhit->E;
	  
	  int myrow   = 23-ccalhit->row;
	  int mycol   = 23-ccalhit->column;
	  int idhycal = 1000 + mycol + (myrow-1)*34;
	  int column, row;
	  column = (idhycal-1001)%34+1;
	  row    = (idhycal-1001)/34+1;
	  
	  ECH(column,row) = int(ccalhit->E*10.+0.5);
	}
	
	main_island_();
	
	for(int k = 0; k < adcgam_cbk_.nadcgam; ++k)  {
	  

	  /*
	    hycalcluster[n].type    = type;
	    hycalcluster[n].nhits   = dime;
	    hycalcluster[n].E       = e;
	    hycalcluster[n].time    = 0.0;
	    hycalcluster[n].x       = x;        // biased, unaligned
	    hycalcluster[n].y       = y;
	    hycalcluster[n].chi2    = chi2;
	    hycalcluster[n].sigma_E = 0.0;
	    hycalcluster[n].emax    = 0.0;
	    hycalcluster[n].status  = status;
	    hycalcluster[n].E1      = 0.;
	    hycalcluster[n].E2      = 0.;
	    
	    hycalcluster[n].x1      = 0.;
	    hycalcluster[n].y1      = 0.;
	    hycalcluster[n].id      = 0;
	  */

	  // Build hit object
	  DCCALShower *shower = new DCCALShower;
	  
	  shower->E       =   adcgam_cbk_.u.fadcgam[k][0];
	  shower->x       =   adcgam_cbk_.u.fadcgam[k][1];
	  shower->y       =   adcgam_cbk_.u.fadcgam[k][2];
	  shower->x1      =   0;
	  shower->y1      =   0;

	  shower->chi2    =   adcgam_cbk_.u.fadcgam[k][6];
	  shower->type    =   adcgam_cbk_.u.iadcgam[k][7];
	  shower->dime    =   adcgam_cbk_.u.iadcgam[k][8];
	  shower->status  =   adcgam_cbk_.u.iadcgam[k][10];

	  _data.push_back( shower );
	}
	
	return NOERROR;
	
}

