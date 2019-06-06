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
#include <mutex>
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

static mutex CCAL_MUTEX;
static bool CCAL_PROFILE_LOADED = false;

//------------------
// LoadCCALProfileData
//------------------
bool DCCALShower_factory::LoadCCALProfileData(JApplication *japp, int32_t runnumber)
{
	/*
	static bool first = true;
	
	// we are just setting some global stuff inside island.F so it's OK
	if(!first) {
		return true;
	} else {
		first = false;
	}
	*/
	if(CCAL_PROFILE_LOADED)
		return true;
	else
		CCAL_PROFILE_LOADED = true;

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
	MIN_CLUSTER_ENERGY        =  0.05;   // GeV
	MAX_CLUSTER_ENERGY        =  15.9;   // GeV
	TIME_CUT                  =  15.0 ;  // ns
	MAX_HITS_FOR_CLUSTERING   =  80;
	SHOWER_DEBUG              =  0;
	DO_NONLINEAR_CORRECTION   =  0;
	CCAL_RADIATION_LENGTH     =  0.86;
	CCAL_CRITICAL_ENERGY      =  1.1e-3;
	

	gPARMS->SetDefaultParameter("CCAL:SHOWER_DEBUG",   SHOWER_DEBUG);
	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_BLOCK_COUNT", MIN_CLUSTER_BLOCK_COUNT);
	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_SEED_ENERGY", MIN_CLUSTER_SEED_ENERGY);
	gPARMS->SetDefaultParameter("CCAL:MIN_CLUSTER_ENERGY", MIN_CLUSTER_ENERGY);
	gPARMS->SetDefaultParameter("CCAL:MAX_CLUSTER_ENERGY", MAX_CLUSTER_ENERGY);
	gPARMS->SetDefaultParameter("CCAL:MAX_HITS_FOR_CLUSTERING", MAX_HITS_FOR_CLUSTERING);
	gPARMS->SetDefaultParameter("CCAL:TIME_CUT",TIME_CUT,"time cut for associating CCAL hits together into a cluster");
	gPARMS->SetDefaultParameter("CCAL:DO_NONLINEAR_CORRECTION", DO_NONLINEAR_CORRECTION);
	gPARMS->SetDefaultParameter("CCAL:CCAL_RADIATION_LENGTH", CCAL_RADIATION_LENGTH);
	gPARMS->SetDefaultParameter("CCAL:CCAL_CRITICAL_ENERGY", CCAL_CRITICAL_ENERGY);
	
}


//------------------
// brun
//------------------
jerror_t DCCALShower_factory::brun(JEventLoop *eventLoop, int32_t runnumber)
{

	DApplication *dapp = dynamic_cast<DApplication*>(eventLoop->GetJApplication());
    	const DGeometry *geom = dapp->GetDGeometry(runnumber);
	
	//vector<double> CCALpos;
	//geom->Get("//composition[@name='ComptonEMcal']/posXYZ[@volume='comptonEMcal']/@X_Y_Z",CCALpos);
    
    	if (geom) {
      	  geom->GetTargetZ(m_zTarget);
      	  geom->GetCCALZ(m_CCALfront);
    	}
    	else{
      	  cerr << "No geometry accessbile." << endl;
      	  return RESOURCE_UNAVAILABLE;
    	}

	
	//------------------------------------------------------------
	// read in island algorithm configurations
	
	std::unique_lock<std::mutex> lck(CCAL_MUTEX);
	LoadCCALProfileData(eventLoop->GetJApplication(), runnumber);	
	lck.unlock();

	//------------------------------------------------------------
	// read in the nonlinearity parameters:
	
	vector< vector<float> > nonlin_params;
	if( eventLoop->GetCalib("/CCAL/nonlinear_energy_correction",nonlin_params) )
	  jout << "Error loading /CCAL/nonlinear_energy_correction !" << endl;
	else {
	  if( (int)nonlin_params.size() != CCAL_CHANS ) {
	    cout << "DCCALShower_factory: Wrong number of entries to nonlinear energy correction table (should be 144)." << endl;
	    for( int ii = 0; ii < CCAL_CHANS; ++ii ) {
	      Nonlin_p0.push_back(0.0);
	      Nonlin_p1.push_back(0.0);
	      Nonlin_p2.push_back(0.0);
	      Nonlin_p3.push_back(0.0);
	    }
	  } else {
	    for( vector< vector<float> >::const_iterator iter = nonlin_params.begin(); iter != nonlin_params.end(); ++iter ) {
	      if( iter->size() != 4 ) {
	        cout << "DCCALShower_factory: Wrong number of values in nonlinear energy correction table (should be 4)" << endl;
                continue;
	      }
	    
	      Nonlin_p0.push_back( (*iter)[0] );
	      Nonlin_p1.push_back( (*iter)[1] );
	      Nonlin_p2.push_back( (*iter)[2] );
	      Nonlin_p3.push_back( (*iter)[3] );
	    
	    }
	  }
	}
	

	//------------------------------------------------------------
	// read in shower timewalk parameters
	
	vector< vector<float> > timewalk_params;
	if( eventLoop->GetCalib("/CCAL/shower_timewalk_correction",timewalk_params) )
	  jout << "Error loading /CCAL/shower_timewalk_correction !" << endl;
	else {
	  if( (int)timewalk_params.size() != 2 ) {
	    cout << "DCCALShower_factory: Wrong number of entries to timewalk correction table (should be 144)." << endl;
	    for( int ii = 0; ii < 2; ++ii ) {
	      timewalk_p0.push_back(0.0);
	      timewalk_p1.push_back(0.0);
	      timewalk_p2.push_back(0.0);
	      timewalk_p3.push_back(0.0);
	    }
	  } else {
	    for( vector< vector<float> >::const_iterator iter = timewalk_params.begin(); iter != timewalk_params.end(); ++iter ) {
	      if( iter->size() != 4 ) {
	        cout << "DCCALShower_factory: Wrong number of values in timewalk correction table (should be 4)" << endl;
                continue;
	      }
	    
	      timewalk_p0.push_back( (*iter)[0] );
	      timewalk_p1.push_back( (*iter)[1] );
	      timewalk_p2.push_back( (*iter)[2] );
	      timewalk_p3.push_back( (*iter)[3] );
	    
	    }
	  }
	}
	
	if( SHOWER_DEBUG ) {
	  cout << "\n\nNONLIN_P0  NONLIN_P1  NONLIN_P2  NONLIN_P3" << endl;
	  for(int ii = 0; ii < (int)Nonlin_p0.size(); ii++) {
	    cout << Nonlin_p0[ii] << " " << Nonlin_p1[ii] << " " << Nonlin_p2[ii] << " " << Nonlin_p3[ii] << endl;
	  }
	  cout << "\n\n";
	}
	//------------------------------------------------------------
	
	// LoadCCALProfileData() runs some Fortran routines which need access to globals - dangerous!
	//std::lock_guard<std::mutex> lck(CCAL_MUTEX);

	//LoadCCALProfileData(eventLoop->GetJApplication(), runnumber);	

	return NOERROR;
}


//------------------
// evnt
//------------------
jerror_t DCCALShower_factory::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{

	vector< const DCCALHit* > ccalhits;
	eventLoop->Get( ccalhits );
	int n_h_hits = (int)ccalhits.size();
	if( n_h_hits > MAX_HITS_FOR_CLUSTERING ) return NOERROR;



	// Geometry:
	
	vector< const DCCALGeometry* > ccalGeomVect;
    	eventLoop->Get( ccalGeomVect );
    	if (ccalGeomVect.size() < 1)
      	  return OBJECT_NOT_AVAILABLE;
    	const DCCALGeometry& ccalGeom = *(ccalGeomVect[0]);

	DVector3 vertex(0.0, 0.0, m_zTarget); // for now, use center of target as vertex


 	int n_h_clusters = 0;
	vector< ccalcluster_t > ccalcluster; // will hold all clusters
	vector< cluster_t > cluster_storage; // will hold elements of every cluster


	// The Fortran code below uses common blocks, so we need to set a lock
	// so that different threads are not running on top of each other
	
	std::unique_lock<std::mutex> lck(CCAL_MUTEX);
	
		
	// if a single module has multiple hits in same event, one will overwrite the 
	// other in the cluster pattern. for now just keep hit with larger energy:
	
	vector< const DCCALHit* > hit_storage;
	cleanHitPattern( ccalhits, hit_storage );
	n_h_hits = (int)hit_storage.size();
	
	
	static int ich[MCOL][MROW];
	for(int icol = 1; icol <= MCOL; ++icol) {
	  for(int irow = 1; irow <= MROW; ++irow) {
	    ECH(icol,irow)     = 0;
	    STAT_CH(icol,irow) = 0;
	    if(icol>=6 && icol<=7 && irow>=6 && irow<=7) { STAT_CH(icol,irow) = -1; }
	    ich[icol-1][irow-1] = (12-icol)+(12-irow)*12;
	  }
	}
	  
	NCOL = 12; NROW = 12;
	SET_XSIZE = CRYS_SIZE_X; SET_YSIZE = CRYS_SIZE_Y;
	  
	for (int i = 0; i < n_h_hits; i++) {
	  const DCCALHit *ccalhit = hit_storage[i];  
	  int row   = 12-ccalhit->row;
	  int col   = 12-ccalhit->column;
	  ECH(col,row) = int(ccalhit->E*10.+0.5);
	}
	  
	main_island_();
	  
	int init_clusters = adcgam_cbk_.nadcgam;
	for(int k = 0; k < init_clusters; ++k)  {
	  
	  cluster_t clust_storage; // stores hit information of cluster cells
	  ccalcluster_t clust;     // stores cluster parameters

	  // results of main_island_():
	  float e     = adcgam_cbk_.u.fadcgam[k][0];
	  float x     = adcgam_cbk_.u.fadcgam[k][1];
	  float y     = adcgam_cbk_.u.fadcgam[k][2];
	  float xc    = adcgam_cbk_.u.fadcgam[k][4];
	  float yc    = adcgam_cbk_.u.fadcgam[k][5];
	  float chi2  = adcgam_cbk_.u.fadcgam[k][6];
	  int type    = adcgam_cbk_.u.iadcgam[k][7];
	  int dime    = adcgam_cbk_.u.iadcgam[k][8];
	  int status  = adcgam_cbk_.u.iadcgam[k][10];
	    
	  if( dime < MIN_CLUSTER_BLOCK_COUNT ) { continue; } 
	  if( e < MIN_CLUSTER_ENERGY || e > MAX_CLUSTER_ENERGY ) { continue; }
	  n_h_clusters += 1;
	  
	  
	  //------------------------------------------------------------
	  //  Do energy and position corrections:
	  
	  float ecellmax = -1; int idmax = -1;
	  float sW   = 0.0;
	  float xpos = 0.0;
	  float ypos = 0.0;
	  float e1   = 0.0;
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
	  float xmax    = ccalGeom.positionOnFace(idmax).X();
	  float ymax    = ccalGeom.positionOnFace(idmax).Y();
	  
	  
	  // loop over hits in cluster and fill clust_storage
	  int ccal_id;
	  for(int j = 0; j < (dime>MAX_CC ? MAX_CC : dime); ++j) {
	    int id = ICL_INDEX(k,j);
	    int kx = (id/100), ky = id%100;
	    ccal_id = ich[kx-1][ky-1];
	  
	    float ecell  = 1.e-4*(float)ICL_IENER(k,j);
	    float xcell  = ccalGeom.positionOnFace(ccal_id).X();
	    float ycell  = ccalGeom.positionOnFace(ccal_id).Y();
	    
	    if(type%10 == 1 || type%10 == 2) {
	      xcell += xc;
	      ycell += yc;
	    }
	    
	      
	    float hittime = 0.;
	    for(int ihit = 0; ihit < n_h_hits; ihit++) {
	      int trialid = 12*(hit_storage[ihit]->row) + hit_storage[ihit]->column;
	      if(trialid == ccal_id) {
	        hittime = hit_storage[ihit]->t;
		//iop = hit_storage[ihit]->intOverPeak;
		break;
	      }
	    }
	    
	    if( ecell > 0.009 && fabs(xcell-xmax) < 6. && fabs(ycell-ymax) < 6.) {
	      W = 4.2 + log(ecell/e);
	      if(W > 0) { // i.e. if cell has > 1.5% of cluster energy
		sW   += W;
		xpos += xcell*W;
		ypos += ycell*W;
	      }
	    }
	    
	    clust_storage.id[j] = ccal_id;
	    clust_storage.E[j]  = ecell;
	    clust_storage.x[j]  = xcell;
	    clust_storage.y[j]  = ycell;
	    clust_storage.t[j]  = hittime;
	    
	  }

	  for(int j = dime; j < MAX_CC; ++j)  // zero the rest
	    clust_storage.id[j] = -1;
	  
	  float weightedTime = getEnergyWeightedTime( clust_storage, dime );
	  float showerTime = getCorrectedTime( weightedTime, e );
	  
	  
	  //------------------------------------------------------------
	  //  Apply correction for finite depth of hit:
	  
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
	  
	  // fill cluster bank for further processing:	
	  clust.type   = type;
	  clust.nhits  = dime;
	  clust.id     = idmax;
	  clust.E      = e;
	  clust.time   = showerTime;
	  clust.x      = x;
	  clust.y      = y;
	  clust.chi2   = chi2;
	  clust.x1     = x1;
	  clust.y1     = y1;
	  clust.emax   = ecellmax;
	  clust.status = status;
	  
	  cluster_storage.push_back(clust_storage);
	  ccalcluster.push_back(clust);
	
	}


	// Release the lock
	lck.unlock();
	

	if(n_h_clusters == 0) return NOERROR;
	final_cluster_processing(ccalcluster, n_h_clusters); 
	
	
	//------------------------------------------------------------
	//  Fill Shower Object:
	
	for(int k = 0; k < n_h_clusters; ++k) {
	
	  // Build hit object
	  DCCALShower *shower = new DCCALShower;
	  
	  shower->E        =   ccalcluster[k].E;
	  shower->x        =   ccalcluster[k].x;
	  shower->y        =   ccalcluster[k].y;
	  shower->z        =   m_CCALfront;
	  shower->x1       =   ccalcluster[k].x1;
	  shower->y1       =   ccalcluster[k].y1;
	  shower->time     =   ccalcluster[k].time;
	  shower->chi2     =   ccalcluster[k].chi2;
	  shower->type     =   ccalcluster[k].type;
	  shower->dime     =   ccalcluster[k].nhits;
	  shower->status   =   ccalcluster[k].status;
	  shower->sigma_E  =   ccalcluster[k].sigma_E;
	  shower->Emax     =   ccalcluster[k].emax;
	  shower->idmax    =   ccalcluster[k].id;
	  
	  for(int icell=0; icell<ccalcluster[k].nhits; icell++) {
	    shower->id_storage[icell] = cluster_storage[k].id[icell];
	    shower->en_storage[icell] = cluster_storage[k].E[icell];
	    shower->t_storage[icell] = cluster_storage[k].t[icell];
	    
	   // if(cluster_storage[k].E[icell] > 0.)  {   // maybe redundant if-statement???
  	   // 	shower->AddAssociatedObject(ccalhits[ cluster_storage[k].id[icell] ]);
	   //	}
	  }

	  _data.push_back( shower );
	}


	return NOERROR;
	
}




//------------------------
// cleeanHitPattern
//------------------------
void DCCALShower_factory::cleanHitPattern( vector< const DCCALHit* > hitarray, vector< const DCCALHit* > &hitarrayClean ) 
{

	for( vector< const DCCALHit* >::const_iterator iHit = hitarray.begin(); iHit != hitarray.end(); ++iHit ) {
	  int id12 = ((*iHit)->row)*12 + (*iHit)->column;
	  int findVal = -1;
	  for( vector<const DCCALHit*>::size_type ii = 0; ii != hitarrayClean.size(); ++ii ) {
	    int id = 12*(hitarrayClean[ii]->row) + hitarrayClean[ii]->column;
	    if( id == id12 ) { findVal = (int)ii; break; }
	  }
	  if( findVal >= 0 ) {
	    if( (*iHit)->E > hitarrayClean[findVal]->E ) {
	      hitarrayClean.erase( hitarrayClean.begin()+findVal );
	      hitarrayClean.push_back( (*iHit) );
	    }
	  } else {
	    hitarrayClean.push_back( (*iHit) );
	  }
 	}

	return;

}




//----------------------------
//   final_cluster_processing
//----------------------------
void DCCALShower_factory::final_cluster_processing( vector< ccalcluster_t > &ccalcluster, int n_h_clusters ) 
{


	//--------------------------
	// final cluster processing:
	//  - do nolinear energy correction
	//  - add status and energy resolution

    	for(int i = 0; i < n_h_clusters; ++i)  {
	  float e     = ccalcluster[i].E;
	  int idmax   = ccalcluster[i].id;
	  float ecorr = e;
	  if(DO_NONLINEAR_CORRECTION) ecorr = energy_correct( e, idmax );
	  
	  if( SHOWER_DEBUG ) {
	    cout << "\n\nShower energy before correction: " << e << " GeV" << endl;
	    cout << "Shower energy after  correction: " << ecorr << " GeV\n\n" << endl;
	  }

      	  //float x   = ccalcluster[i].x1;
      	  //float y   = ccalcluster[i].y1;
          // coord_align(i, e, idmax);
	  
      	  int status = ccalcluster[i].status;
      	  int type   = ccalcluster[i].type;
      	  int dime   = ccalcluster[i].nhits;

      	  if(status < 0) {
            printf("error in island : cluster with negative status");
            exit(1);
          }
          int itp, ist;
          itp  = 0;
      	  if(status==1) itp += 1;

      	  for(int k = 0; k < (dime>MAX_CC ? MAX_CC : dime); ++k) {
            if(itp>=10) {itp = 12; break;}
	  }

      	  ist = type;
      	  if(status>2) ist += 20;

      	  double se = sqrt(0.9*0.9*e*e+2.5*2.5*e+1.0); // from HYCAL reconstruction, may need to tune
      	  se /= 100.;
      	  if(itp%10==1)
            se *= 1.5;
      	  else if(itp%10==2) {
            if(itp>10)
              se *= 0.8;
            else
              se *=1.25;
          }
	  ccalcluster[i].E = ecorr;
	  ccalcluster[i].type = itp;
	  ccalcluster[i].status = ist;
      	  ccalcluster[i].sigma_E = se;
	}

	return;

}



//------------------------
// getEnergyWeightedTime
//------------------------
float DCCALShower_factory::getEnergyWeightedTime( cluster_t cluster_storage, int nHits )
{

	float weightedtime = 0.;
	float totEn = 0;
	for(int j = 0; j < (nHits > MAX_CC ? MAX_CC : nHits); ++j) {
	  weightedtime += cluster_storage.t[j]*cluster_storage.E[j];
	  totEn += cluster_storage.E[j];
	}
	weightedtime /= totEn;
	  
	return weightedtime;

}



//------------------------
// getCorrectedTime
//------------------------

float DCCALShower_factory::getCorrectedTime( float time, float energy ) 
{
	// timewalk correction:
	// t + p0*exp(p1 + p2*E) + p3
	
	int iPar;
	if( energy < 1.0 ) iPar = 0;
	else iPar = 1;
	
	float dt = timewalk_p0[iPar]*exp(timewalk_p1[iPar] + timewalk_p2[iPar]*energy) + timewalk_p3[iPar];
	float t_cor = time - dt;
	
	return t_cor;
	
}





//------------------------
// shower_depth
//------------------------
float DCCALShower_factory::shower_depth( float energy ) 
{

	float z0 = CCAL_RADIATION_LENGTH, e0 = CCAL_CRITICAL_ENERGY;
	float depth = (energy > 0.) ? z0*log(1.+energy/e0) : 0.;
	return depth;

}



//------------------------
// energy_correct
//------------------------
float DCCALShower_factory::energy_correct( float energy, int id ) 
{

	if( Nonlin_p1[id] == 0. && Nonlin_p2[id] == 0. && Nonlin_p3[id] == 0.) return energy;
  	if( energy < 0.5 || energy > 12. ) return energy;


  	float emin = 0., emax = 12.;
  	float e0 = (emin+emax)/2.;

  	float de1 = energy - emin*f_nonlin( emin, id );
  	float de2 = energy - emax*f_nonlin( emax, id );
  	float de  = energy - e0*f_nonlin( e0, id );

  	while( fabs(emin-emax) > 1.e-5 ) {
    	  if( de1*de > 0. && de2*de < 0.) {
      	    emin = e0;
      	    de1 = energy - emin*f_nonlin( emin, id );
    	  } else {
      	    emax = e0;
      	    de2 = energy - emax*f_nonlin( emax, id );
    	  }
    	  e0 = (emin+emax)/2.;
    	  de  = energy - e0*f_nonlin( e0, id );
  	}
	
  	return e0;

}


//------------------------
// f_nonlin
//------------------------
float DCCALShower_factory::f_nonlin( float e, int id ) 
{

  	return pow( (e/Nonlin_p0[id]), Nonlin_p1[id] + Nonlin_p2[id]*e + Nonlin_p3[id]*e*e );

}

