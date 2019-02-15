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
	TIME_CUT                  =  15.0 ;  // ns
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
	
	// extract the CCAL Geometry
	vector<const DCCALGeometry*> ccalGeomVect;
    	eventLoop->Get( ccalGeomVect );
    	if (ccalGeomVect.size() < 1)
      	  return OBJECT_NOT_AVAILABLE;
    	const DCCALGeometry& ccalGeom = *(ccalGeomVect[0]);
	
	for(int icol = 1; icol <= MCOL; ++icol) {
	  for(int irow = 1; irow <= MROW; ++irow) {
	    
	    int id = (12-icol) + (12-irow)*12;
	    DVector2 pos = ccalGeom.positionOnFace(12-irow,12-icol);
	    
	    blockINFO[id].id     = id;
	    blockINFO[id].x      = pos.X();
	    blockINFO[id].y      = pos.Y();
	    blockINFO[id].sector = 0;
	    blockINFO[id].row    = 12-irow;
	    blockINFO[id].col    = 12-icol;
	  }
	}

	return NOERROR;
}


//------------------
// evnt
//------------------
jerror_t DCCALShower_factory::evnt(JEventLoop *eventLoop, uint64_t eventnumber)
{
    	
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
	
	for(int icol = 1; icol <= MCOL; ++icol) {
	  for(int irow = 1; irow <= MROW; ++irow) {
	    ECH(icol,irow)     = 0;
	    TIME(icol,irow)    = 0;
	    STAT_CH(icol,irow) = 0;
	    if(icol>=6 && icol<=7 && irow>=6 && irow<=7) { STAT_CH(icol,irow) = -1; }	
	    
	    ich[icol-1][irow-1] = (12-icol)+(12-irow)*12;
	        
	  }
	}
	
	NCOL = 12; NROW = 12;
	SET_XSIZE = CRYS_SIZE_X; SET_YSIZE = CRYS_SIZE_Y;
	
	for (unsigned int i = 0; i < ccalhits.size(); i++) {	  
	  const DCCALHit *ccalhit = ccalhits[i];

	  if(SHOWER_DEBUG == 1){
	    cout << "ccalhit row col e = " << ccalhit->row << " " << ccalhit->column << " " << ccalhit->E << endl;
	  }

	  tot_en += ccalhit->E;
	  
	  int row   = 12-(ccalhit->row);
	  int col   = 12-(ccalhit->column);
	  
	  ECH(col,row) = int(ccalhit->E*10.+0.5);
	  TIME(col,row) = ccalhit->t;
	}	
	
	main_island_();
	
	n_h_clusters = adcgam_cbk_.nadcgam; 	
	for(int k = 0; k < n_h_clusters; ++k)  {

	  // results of main_island_():
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
	  
	  
	  
	  //------------------------------------------------------------
	  //  Do energy and position corrections:
	  
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
	  float xmax    = blockINFO[idmax].x;
	  float ymax    = blockINFO[idmax].y;
	  
	  int ccal_id;
	  for(int j = 0; j < (dime>MAX_CC ? MAX_CC : dime); ++j) {
	    int id = ICL_INDEX(k,j);
	    
	    int kx = (id/100), ky = id%100;
	    ccal_id = ich[kx-1][ky-1];
	    
	    float ecell = 1.e-4*(float)ICL_IENER(k,j);
	    float xcell = blockINFO[ccal_id].x;
	    float ycell = blockINFO[ccal_id].y;
	    
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
	  ccalcluster[k].type   = type;
	  ccalcluster[k].nhits  = dime;
	  ccalcluster[k].id     = idmax;
	  ccalcluster[k].E      = e;
	  ccalcluster[k].time   = time;
	  ccalcluster[k].x      = x;
	  ccalcluster[k].y      = y;
	  ccalcluster[k].chi2   = chi2;
	  ccalcluster[k].x1     = x1;
	  ccalcluster[k].y1     = y1;
	  ccalcluster[k].emax   = ecellmax;
	  ccalcluster[k].status = status;
	
	}
	
	glue_transition_island(); // find adjacent clusters and glue 

	final_cluster_processing(); 
	

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

	  _data.push_back( shower );
	}


	// Release the lock	
	//japp->Unlock("CCALShower_factory");
	pthread_mutex_unlock(&mutex);

	return NOERROR;
	
}




//----------------------------
//   glue_transition_island
//----------------------------
void DCCALShower_factory::glue_transition_island() {

	int ngroup = 0;
	int group_number[MAX_CLUSTERS];
	int group_size[MAX_CLUSTERS];
	int group_member_list[MAX_CLUSTERS][MAX_CLUSTERS];
	
        for(int i = 0; i < MAX_CLUSTERS; ++i) { group_number[i] = 0; }

        for(int i = 0; i < n_h_clusters; ++i)  {
      	  int  idi = ccalcluster[i].id;

    	  for(int j = i+1; j < n_h_clusters; ++j) {
      	    int  idj = ccalcluster[j].id;
      	    if(blockINFO[idi].sector == blockINFO[idj].sector) continue;

      	    if(clusters_mindist(i,j)) continue;

      	    int igr = group_number[i];
      	    int jgr = group_number[j];

	    // create new group and put theese two clusters into it
    	    if(!igr && !jgr) {
              ++ngroup;
              group_number[i] = ngroup; group_number[j] = ngroup;
              group_member_list[ngroup][0] = i;
              group_member_list[ngroup][1] = j;
              group_size[ngroup] = 2;
              continue;
      	    }
	    // add jth cluster into group which ith cluster belongs to
     	    if(igr && !jgr) {
              group_number[j] = igr;
              group_member_list[igr][group_size[igr]] = j;
              group_size[igr] += 1;
              continue;
            }
	    // add ith cluster into group which jth cluster belongs to
            if(!igr && jgr) {
              group_number[i] = jgr;
              group_member_list[jgr][group_size[jgr]] = i;
              group_size[jgr] += 1;
              continue;
            }

	    // add jth group to ith than discard
            if(igr && jgr) {
              if(igr == jgr) continue; // nothing to do
	      
              int ni = group_size[igr];
              int nj = group_size[jgr];

              for(int k = 0; k < nj; ++k) {
                group_member_list[igr][ni+k] = group_member_list[jgr][k];
                group_number[group_member_list[jgr][k]] = igr;
              }
              group_size[igr] += nj;
              group_size[jgr]  = 0;

              continue;
            }
          }
        }

        for(int igr = 1; igr <= ngroup; ++igr) {

          if(group_size[igr]<=0) continue;

          // sort in increasing cluster number order:
          int list[MAX_CLUSTERS];
          for(int i = 0; i < group_size[igr]; ++i) { list[i] = i; }

          for(int i = 0; i < group_size[igr]; ++i) {
            for(int j = i+1; j < group_size[igr]; ++j) {
              if(group_member_list[igr][list[j]] < group_member_list[igr][list[i]]) {
                int l   = list[i];
                list[i] = list[j];
                list[j] = l;
              }
	    }
	  }

          int i0 = group_member_list[igr][list[0]];
          if(ccalcluster[i0].status==-1) continue;

          for(int k = 1; k < group_size[igr]; ++k) {
            int k0 = group_member_list[igr][list[k]];
            if(ccalcluster[k0].status==-1) {
              printf("glue island warning neg. status\n");
              continue;
            }
            merge_clusters(i0,k0);
            ccalcluster[k0].status = -1;
          }
        }
  
  
        int ifdiscarded;

        // discrard clusters merged with others:
        do {
          ifdiscarded = 0;
          for(int i = 0; i < n_h_clusters; ++i)  {
            if(ccalcluster[i].status == -1 || \
                ccalcluster[i].E       < 0.08 || \
                ccalcluster[i].E       > 15.0 || \
                ccalcluster[i].nhits   < MIN_CLUSTER_BLOCK_COUNT || \
                ccalcluster[i].emax    < MIN_CLUSTER_SEED_ENERGY) {

              n_h_clusters -= 1;
              int nrest = n_h_clusters - i;
              if(nrest) {
                memmove((&ccalcluster[0]+i), (&ccalcluster[0]+i+1), nrest*sizeof(ccalcluster_t));
                memmove((&cluster_storage[0]+i), (&cluster_storage[0]+i+1), nrest*sizeof(cluster_t));
                ifdiscarded = 1;
              }
            }
          }} while(ifdiscarded);


	return;
}



//----------------------------
//   merge_clusters
//----------------------------
void DCCALShower_factory::merge_clusters(int i, int j) {

  	// merge jth cluster into ith cluster and discard jth

  	// leave cluster with greater energy deposition in central cell, discard the second one:

  	int i1 = (ccalcluster[i].emax > ccalcluster[j].emax) ? i : j;

  	int dime1 = ccalcluster[i].nhits;
	int dime2 = ccalcluster[j].nhits;
  	float e = ccalcluster[i].E + ccalcluster[j].E;

  	ccalcluster[i].id      = ccalcluster[i1].id;
  	ccalcluster[i].sigma_E = ccalcluster[i1].sigma_E;
  	ccalcluster[i].emax    = ccalcluster[i1].emax;
  	ccalcluster[i].x       = ccalcluster[i1].x;
  	ccalcluster[i].y       = ccalcluster[i1].y;
  	ccalcluster[i].chi2    = (ccalcluster[i].chi2*dime1 + ccalcluster[j].chi2*dime2)/(dime1+dime2);

  	if(ccalcluster[i].status>=2)
  	  ccalcluster[i].status += 1; // glued
  	ccalcluster[i].nhits   = dime1+dime2;
  	ccalcluster[i].E       = e;
  	ccalcluster[i].type    = ccalcluster[i1].type;

  	for(int k1 = 0; k1 < (dime1>MAX_CC ? MAX_CC : dime1); ++k1) {
    	  for(int k2 = 0; k2 < (dime2>MAX_CC ? MAX_CC : dime2); ++k2) {
      	    if(cluster_storage[i].id[k1] == cluster_storage[j].id[k2]) {
              if(cluster_storage[j].E[k2] > 0.) {
                cluster_storage[i].E[k1] += cluster_storage[j].E[k2];
                cluster_storage[i].x[k1] += cluster_storage[j].x[k2];
                cluster_storage[i].x[k1] *= 0.5;
                cluster_storage[i].y[k1] += cluster_storage[j].y[k2];
                cluster_storage[i].y[k1] *= 0.5;

                cluster_storage[j].E[k2] = 0.;
                cluster_storage[j].x[k2] = 0.;
                cluster_storage[j].y[k2] = 0.;
              }
              continue;
      	    }
    	  }
  	}

  	float sW      = 0.0;
  	float xpos    = 0.0;
  	float ypos    = 0.0;
  	float W;

  	for(int k1 = 0; k1 < (dime1>MAX_CC ? MAX_CC : dime1); ++k1) {
    	  float ecell = cluster_storage[i].E[k1];
    	  float xcell = cluster_storage[i].x[k1];
    	  float ycell = cluster_storage[i].y[k1];
    	  if(ecell>0.) {
      	    W  = 4.2 + log(ecell/e);
      	    if(W > 0) {
              sW += W;
              xpos += xcell*W;
              ypos += ycell*W;
      	    }
    	  }
  	}

  	for(int k2 = 0; k2 < (dime2>MAX_CC ? MAX_CC : dime2); ++k2) {
    	  float ecell = cluster_storage[j].E[k2];
    	  float xcell = cluster_storage[j].x[k2];
    	  float ycell = cluster_storage[j].y[k2];

    	  if(ecell>0.) {
      	    W  = 4.2 + log(ecell/e);
      	    if(W > 0) {
              sW += W;
              xpos += xcell*W;
              ypos += ycell*W;
      	    }
    	  }
  	}

  	if(sW) {
    	  float dx = ccalcluster[i1].x1;
    	  float dy = ccalcluster[i1].y1;
    	  ccalcluster[i].x1 = xpos/sW;
    	  ccalcluster[i].y1 = ypos/sW;
    	  dx = ccalcluster[i1].x1 - dx;
    	  dy = ccalcluster[i1].y1 - dy;
    	  ccalcluster[i].x += dx;      // shift x0,y0 for glued cluster by x1,y1 shift values
    	  ccalcluster[i].y += dy;
  	} else {
    	  printf("WRN bad cluster log. coord\n");
    	  ccalcluster[i].x1 = 0.;
    	  ccalcluster[i].y1 = 0.;
  	}

	// update cluster_storage

  	int kk = dime1;

  	for(int k = 0; k < (dime2>MAX_CC-dime1 ? MAX_CC-dime1 : dime2); ++k) {
    	  if(cluster_storage[j].E[k] <= 0.) continue;
    	  if(kk >= MAX_CC) {
      	    printf("WARINING: number of cells in cluster exceeded max %i\n", dime1+dime2);
      	    break;
    	  }
    	  cluster_storage[i].id[kk] = cluster_storage[j].id[k];
    	  cluster_storage[i].E[kk]  = cluster_storage[j].E[k];
    	  cluster_storage[i].x[kk]  = cluster_storage[j].x[k];
    	  cluster_storage[i].y[kk]  = cluster_storage[j].y[k];
    	  cluster_storage[j].id[k]  = 0;
    	  cluster_storage[j].E[k]   = 0.;
    	  ++kk;
  	}

  	ccalcluster[i].nhits = kk;

  	return;
	
}



//----------------------------
//   final_cluster_processing
//----------------------------
void DCCALShower_factory::final_cluster_processing() {

	//  final cluster processing (add status and energy resolution sigma_E):

    	for(int i = 0; i < n_h_clusters; ++i)  {
      	  float e   = ccalcluster[i].E;
      	  int idmax = ccalcluster[i].id;

      	  // apply 1st approx. non-lin. corr. (was obtained for PWO):
      	  // e *= 1. + 200.*pi/pow((pi+e),7.5);
      	  // ccalcluster[i].E = e;

      	  float x   = ccalcluster[i].x1;
      	  float y   = ccalcluster[i].y1;

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

      	  /*for(int k = 0; k < (dime>MAX_CC ? MAX_CC : dime); ++k) {
            if(itp<10) {
              if(cluster_storage[i].id[k] < 1000) {itp =  2; break;}
            } else {
              if(cluster_storage[i].id[k] > 1000) {itp = 12; break;}
            }
	  }*/
          ccalcluster[i].type = itp;

      	  ist = type;
      	  if(status>2) ist += 20; // glued
      	  ccalcluster[i].status = ist;

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
      	  ccalcluster[i].sigma_E = se;
	}

	return;

}



//-----------------------
//   clusters_mindist
//-----------------------
bool DCCALShower_factory::clusters_mindist(int i, int j) {

	double mindx_cut = 1.1, mindy_cut = 1.1; // maybe should be tuned

	// min distance between two clusters cells cut

  	double mindist = 1.e6, mindx = 1.e6, mindy = 1.e6, dx, dy, sx, sy;
  	int dime1 = ccalcluster[i].nhits, dime2 = ccalcluster[j].nhits;

	sx = CRYS_SIZE_X; sy = CRYS_SIZE_Y;

  	for(int k1 = 0; k1 < (dime1>MAX_CC ? MAX_CC : dime1); ++k1) {
    	  if(cluster_storage[i].E[k1] <= 0.) continue;

    	  for(int k2 = 0; k2 < (dime2>MAX_CC ? MAX_CC : dime2); ++k2) {
      	    if(cluster_storage[j].E[k2] <= 0.) continue;

      	    dx = cluster_storage[i].x[k1] - cluster_storage[j].x[k2];
      	    dx = 0.5*dx*(1./sx+1./sx);
      	    dy = cluster_storage[i].y[k1] - cluster_storage[j].y[k2];
      	    dy = 0.5*dy*(1./sy+1./sy);

      	    if(dx*dx+dy*dy<mindist) { mindist = dx*dx+dy*dy; mindx = fabs(dx); mindy = fabs(dy); }
    	  }
  	}

  	return (mindx>mindx_cut || mindy>mindy_cut);

}



//------------------------
// shower_depth
//------------------------
float shower_depth(float energy) {

	float z0 = 0.86, e0 = 1.1e-3;
	float res = (energy > 0.) ? z0*log(1.+energy/e0) : 0.;
	return res;

}
