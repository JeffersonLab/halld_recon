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
#include <JANA/JCalibration.h>
#include <JANA/JResourceManager.h>
using namespace jana;

#include "DIRC/DDIRCLutReader.h"
#include "CCAL/DCCALShower.h"
#include "CCAL/DCCALShower_factory.h"
#include "CCAL/DCCALHit.h"
#include "CCAL/DCCALGeometry.h"
#include "HDGEOMETRY/DGeometry.h"

#include "hycal.h"

//------------------
// LoadCCALProfileData
//------------------
bool DCCALShower_factory::LoadCCALProfileData(JApplication *japp, int32_t runnumber)
{
	string ccal_profile_file;
	gPARMS->SetDefaultParameter("CCAL_PROFILE_FILE", ccal_profile_file, "CCAL profile data file name");
		
	// make sure we have a pointer to the JApplication
	if(japp == nullptr)
		return false;
	
	// follow similar procedure as other resources (DMagneticFieldMapFineMesh)
	map<string,string> profile_file_name;
	JCalibration *jcalib = japp->GetJCalibration(runnumber);
	if(jcalib->GetCalib("/CCAL/profile_data/profile_data_map", profile_file_name)) 
		jout << "Can't find requested /CCAL/profile_data/profile_data_map in CCDB for this run!" << endl;
	else if(profile_file_name.find("map_name") != profile_file_name.end() 
				&& profile_file_name["map_name"] != "None") {
		JResourceManager *jresman = japp->GetJResourceManager(runnumber);
		ccal_profile_file = jresman->GetResource(profile_file_name["map_name"]);
	}
	
	jout<<"Reading CCAL profile data from "<<ccal_profile_file<<" ..."<<endl;
	
	// check to see if we actually have a file
	if(ccal_profile_file.empty()) {
		jerr << "Empty file..." << endl;
		return false;
	}
	
	
	char fname_buf[1000];
	sprintf(fname_buf,"%s",ccal_profile_file.c_str());
	int ccal_profile_file_size = (int)ccal_profile_file.size();
	init_island_(fname_buf, &ccal_profile_file_size);

/*
	ifstream ccal_profile(ccal_profile_file.c_str());
	for(int i=0; i<=500; i++) {
		for(int j=0; j<i; j++) {
			int id1, id2;
			float fcell_hyc, fd2c;
			
			ccal_profile >> id1 >> id2 >> fcell_hyc >> fd2c;
		
			//cout << id1 << " " << id2 << " " << fcell_hyc << " " << fd2c << endl;
		
			acell[0][i][j] = fcell_hyc;
			acell[0][j][i] = fcell_hyc;
			ad2c[0][i][j] = fd2c;
			ad2c[0][j][i] = fd2c;
		}
	}
	
	// copy the results to the FORTRAN routines
	init_island_(acell, ad2c);
	
	// cleanup
	ccal_profile.close();	
*/
	
	return true;
}


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

	pthread_mutex_init(&mutex, NULL);

}

//------------------
// brun
//------------------
jerror_t DCCALShower_factory::brun(JEventLoop *eventLoop, int32_t runnumber)
{
	LoadCCALProfileData(eventLoop->GetJApplication(), runnumber);
	
	DApplication *dapp = dynamic_cast<DApplication*>(eventLoop->GetJApplication());
    	const DGeometry *geom = dapp->GetDGeometry(runnumber);
	
	vector<double> CCALpos;
	geom->Get("//composition[@name='ComptonEMcal']/posXYZ[@volume='comptonEMcal']/@X_Y_Z",CCALpos);
    
    	if (geom) {
      	  geom->GetTargetZ(m_zTarget);
      	  geom->GetCCALZ(m_CCALfront);
    	}
    	else{
      	  cerr << "No geometry accessbile." << endl;
      	  return RESOURCE_UNAVAILABLE;
    	}

	return NOERROR;
}


//------------------
// evnt
//------------------
jerror_t DCCALShower_factory::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{

	// extract the CCAL Geometry
	vector<const DCCALGeometry*> ccalGeomVect;
    	eventLoop->Get( ccalGeomVect );
    	if (ccalGeomVect.size() < 1)
      	  return OBJECT_NOT_AVAILABLE;
    	const DCCALGeometry& ccalGeom = *(ccalGeomVect[0]);
    	
	// for now, use center of target as vertex
	DVector3 vertex(0.0, 0.0, m_zTarget);
	
    	vector<const DCCALHit*> ccalhits;
	eventLoop->Get(ccalhits);
	
	if(ccalhits.size() > MAX_HITS_FOR_CLUSTERING) return NOERROR;
	
	// The Fortran code below uses common blocks, so we need to set a lock
	// so that different threads are not running on top of each other
	//japp->WriteLock("CCALShower_factory");
	pthread_mutex_lock(&mutex);
	
	double tot_en = 0;

	SET_EMIN   =  0.05;    // banks->CONFIG->config->CLUSTER_ENERGY_MIN;
	SET_EMAX   =  15.9;    // banks->CONFIG->config->CLUSTER_ENERGY_MAX;
	SET_HMIN   =  2;       // banks->CONFIG->config->CLUSTER_MIN_HITS_NUMBER;
	SET_MINM   =  0.08;    // banks->CONFIG->config->CLUSTER_MAX_CELL_MIN_ENERGY;
	ZHYCAL     =  1.e9;    // ALignment[1].al.z;
	ISECT      =  0;       //  PWO

	static int ich[MCOL][MROW]; 
	
	// fill array blockINFO with positions of blocks.
	// for now, use the DCCALGeometry::positionOnFace,
	// in future, update to read in geometry from ccdb
	blockINFO_t blockINFO[T_BLOCKS];

	for(int icol = 1; icol <= MCOL; ++icol) {
	  for(int irow = 1; irow <= MROW; ++irow) {
	    ECH(icol,irow) = 0;
	    TIME(icol,irow) = 0;
	    STAT_CH(icol,irow) = 0;
	    if(icol>=17 && icol<=18 && irow>=17 && irow<=18) { STAT_CH(icol,irow) = -1; }
	    
	    if(icol>=12 && icol<=23 && irow>=12 && irow<=23) {
	      int idhycal = 1000 + icol + (irow-1)*34;
	      int ccal_row = 23-irow;
	      int ccal_col = 23-icol;
	      DVector2 pos = ccalGeom.positionOnFace(ccal_row, ccal_col);
	      int chan     = ccalGeom.channel(ccal_row, ccal_col);
	      
	      blockINFO[idhycal-1].id     = chan;
	      blockINFO[idhycal-1].x      = pos.X();
	      blockINFO[idhycal-1].y      = pos.Y();
	      blockINFO[idhycal-1].sector = 0;
	      blockINFO[idhycal-1].row    = ccal_row;
	      blockINFO[idhycal-1].col    = ccal_col;
	    }
	  }
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
	  TIME(column,row) = ccalhit->t;
	  ich[column-1][row-1] = idhycal;
	}
	
	main_island_();
	
	cluster_t cluster_storage[MAX_CLUSTERS]; 	
	for(int k = 0; k < adcgam_cbk_.nadcgam; ++k)  {

	  // result of main_island_():
	  float e     = adcgam_cbk_.u.fadcgam[k][0];
	  float x     = adcgam_cbk_.u.fadcgam[k][1];
	  float y     = adcgam_cbk_.u.fadcgam[k][2];
	  float xc    = adcgam_cbk_.u.fadcgam[k][4];
	  float yc    = adcgam_cbk_.u.fadcgam[k][5];
	  float time  = adcgam_cbk_.u.fadcgam[k][6];
	  float chi2  = adcgam_cbk_.u.fadcgam[k][7];
	  int type    = adcgam_cbk_.u.iadcgam[k][8];
	  int dime    = adcgam_cbk_.u.iadcgam[k][9];
	  int status  = adcgam_cbk_.u.iadcgam[k][11];
	  
	  
	  // do energy and position corrections:
	  float ecellmax = -1; int idmax = -1;
	  float sW   = 0.0;
	  float xpos = 0.0;
	  float ypos = 0.0;
	  float e1   = 0.0;
	  float e2   = 0.0;
	  float W;
	  
	  // get id of cell with max energy:
	  for(int j = 0; j < (dime>MAX_CC ? MAX_CC : dime); ++j) {
	    float ecell = 1.e-4*(float)ICL_IENER(k,j);
	    int id = ICL_INDEX(k,j);
	    int kx = (id/100), ky = id%100;
	    id = ich[kx-1][ky-1];
	    e1 += ecell;
	    if(ecell>ecellmax) {
	      ecellmax = ecell;
	      idmax = id;
	    }
	  }
	  
	  // x and y pos of max cell
	  float xmax    = blockINFO[idmax-1].x;
	  float ymax    = blockINFO[idmax-1].y;
	  
	  
	  for(int j = 0; j < (dime>MAX_CC ? MAX_CC : dime); ++j) {
	    int id = ICL_INDEX(k,j);
	    int kx = (id/100), ky = id%100;
	    id = ich[kx-1][ky-1];
	    
	    float ecell = 1.e-4*(float)ICL_IENER(k,j);
	    float xcell = blockINFO[id-1].x;
	    float ycell = blockINFO[id-1].y;
	    
	    if(type%10 == 1 || type%10 == 2) {
	      xcell += xc;
	      ycell += yc;
	    }
	    
	    cluster_storage[k].id[j] = id;
	    cluster_storage[k].E[j]  = ecell;
	    cluster_storage[k].x[j]  = xcell;
	    cluster_storage[k].y[j]  = ycell;
	    
	    if(ecell>0.009 && fabs(xcell-xmax)<6. && fabs(ycell-ymax)<6.) {
	      W = 4.2 + log(ecell/e); 
	      if(W > 0) { // i.e. if cell has > 1.5% of cluster energy
		sW   += W;
		xpos += xcell*W;
		ypos += ycell*W;
		e2   += ecell;
	      }
	    }	    
	  }
	  
	  for(int j = dime; j < MAX_CC; ++j)  // zero the rest
	    cluster_storage[k].id[j] = 0;
	  
	  
	  //-----------------------------------------------------
	  // Apply correction for finite depth of hit:
	  
	  float x1, y1;
	  float zV = vertex.Z();
	  float z0 = m_CCALfront - zV;
	  if(sW) {
	    float dz;
	    dz = shower_depth(e);
	    float zk = 1. / (1. + dz/z0);
	    x1 = zk*xpos/sW;
	    y1 = zk*ypos/sW;
	  } else {
	    printf("WRN bad cluster log. coord, center id = %i %f\n", idmax, e);
	    x1 = 0.0;
	    y1 = 0.0;
	  }


	  // Build hit object
	  DCCALShower *shower = new DCCALShower;
	  
	  shower->E       =   adcgam_cbk_.u.fadcgam[k][0];
	  shower->x       =   adcgam_cbk_.u.fadcgam[k][1];
	  shower->y       =   adcgam_cbk_.u.fadcgam[k][2];
	  shower->z       =   m_CCALfront;
	  shower->x1      =   x1;
	  shower->y1      =   y1;
	  shower->time    =   adcgam_cbk_.u.iadcgam[k][6];

	  shower->chi2    =   adcgam_cbk_.u.fadcgam[k][7];
	  shower->type    =   adcgam_cbk_.u.iadcgam[k][8];
	  shower->dime    =   adcgam_cbk_.u.iadcgam[k][9];
	  shower->status  =   adcgam_cbk_.u.iadcgam[k][11];
	  
	  if(ecellmax > 0.) shower->Emax = ecellmax;
	  if(idmax > 0) shower->idmax    = idmax;

	  _data.push_back( shower );
	}


	// Release the lock	
	//japp->Unlock("CCALShower_factory");
	pthread_mutex_unlock(&mutex);

	return NOERROR;
	
}


//------------------
// shower_depth
//------------------
float shower_depth(float energy) {

	float z0 = 0.86, e0 = 1.1e-3;
	float res = (energy > 0.) ? z0*log(1.+energy/e0) : 0.;
	return res;

}

