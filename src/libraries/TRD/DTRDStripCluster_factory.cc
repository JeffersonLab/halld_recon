
#include "DTRDStripCluster_factory.h"

#include <JANA/JEvent.h>
#include "DTRDHit.h"

static bool DTRDHit_cmp(const DTRDHit* a, const DTRDHit* b) {
  if (a->plane==b->plane){
    return a->t < b->t;
  }
  return a->plane < b->plane;
}


///
/// DTRDHit_strip_cmp():
///	a non-member function passed to std::sort() for sorting DTRDHit pointers by
/// their strip (wire or strip) numbers. Typically only used for a single layer
/// of hits.
///
static bool DTRDHit_strip_cmp(const DTRDHit* a, const DTRDHit* b) {
	if(a->strip != b->strip) return a->strip < b->strip;
	if(a->t       != b->t      ) return a->t < b->t;
	return a->pulse_height < b->pulse_height;
}

///
/// DTRDHit_time_cmp()
///    a non-member function passed to std::stable_sort() for sorting DTRDHit 
/// pointers in order of increasing time, provided that the time difference is
/// significant.
///

static bool DTRDHit_time_cmp(const DTRDHit* a, const DTRDHit* b) {
  if (fabs(a->t-b->t)>HIT_TIME_DIFF_MIN && (a->t < b->t))
    return true;
  return false;
}

///
/// DTRDStripCluster_gPlane_cmp():
/// a non-member function passed to std::sort() for sorting DTRDStripCluster pointers
/// by their gPlane (plane number over all modules, 1-74) attributes.
///
static bool DTRDStripCluster_gPlane_cmp(	const DTRDStripCluster* a, 
					const DTRDStripCluster* b) {
	return a->plane < b->plane;
}

///
/// Initialization
///
void DTRDStripCluster_factory::Init()
{
	auto app = GetApplication();

	MINIMUM_HITS_FOR_CLUSTERING = 10;
    app->SetDefaultParameter("TRDCLUSTER:MINIMUM_HITS_FOR_CLUSTERING",MINIMUM_HITS_FOR_CLUSTERING);

	CLUSTERING_THRESHOLD = 1.2;
    app->SetDefaultParameter("TRDCLUSTER:CLUSTERING_THRESHOLD",CLUSTERING_THRESHOLD);

	MinClustSize=10;
	MinClustWidth=0.001;
	MinClustLength=0.01;
	zStart =  0.; // mm
	zEnd   = 40.; // mm

    app->SetDefaultParameter("TRDCLUSTER:MIN_CLUST_SIZE",MinClustSize);
    app->SetDefaultParameter("TRDCLUSTER:MIN_CLUST_WIDTH",MinClustWidth);
    app->SetDefaultParameter("TRDCLUSTER:MIN_CLUST_LENGTH",MinClustLength);
    app->SetDefaultParameter("TRDCLUSTER:ZSTART",zStart);
    app->SetDefaultParameter("TRDCLUSTER:ZEND",zEnd);

    return;	
}


///
/// DTRDStripCluster_factory::StripToPosition():
/// calculate position along plane
///
double DTRDStripCluster_factory::StripToPosition(int iplane, const DTRDHit *hit)
{
	// better to pull this from CCDB, also probably the pitch as well
	double ny = 0;
	if(iplane == 0) {
		ny = NUM_X_PLANES;
	} else if(iplane == 1) {
		ny = NUM_Y_PLANES;
	} 
	double ymi = 0.;     double yma = ny;
	double biny = (yma-ymi)/ny;

	return double(hit->strip)/double(ny)*(yma-ymi)+ymi-biny/2.; 
}


///
/// DTRDStripCluster_factory::evnt():
/// This (along with DTRDStripCluster_factory::pique()) 
/// is the place cathode hits are associated into cathode clusters.  
///
void DTRDStripCluster_factory::Process(const std::shared_ptr<const JEvent>& event) 
{
	vector<DTRDStripCluster*> results;

	vector<const DTRDHit*> allHits;
	vector<const DTRDHit*> planeHits[2];
	
	event->Get<DTRDHit>(allHits);
	
	if (allHits.size() == 0) 
		return;


	// require a minimum number of hits
    if (allHits.size() < MINIMUM_HITS_FOR_CLUSTERING) {
    	return;
    }

	// Sort hits by layer number and by time
	sort(allHits.begin(),allHits.end(),DTRDHit_cmp);
	
	// Sift through all hits and select out X and Y hits.
	// Also apply the raw hit analysis
	for (vector<const DTRDHit*>::iterator i = allHits.begin(); i != allHits.end(); ++i) {
		// sort hits
		int stripPlane = (*i)->plane-1;
		int strip = (*i)->strip-1;
		if( (stripPlane<0) || (stripPlane>=2) ){ // only two planes
			static int Nwarn = 0;
			if( Nwarn<10 ){
				jerr << " stripPlane is outside of array bounds!! stripPlane="<< stripPlane << std::endl;
				if( ++Nwarn==10 )jerr << " LAST WARNING!" << std::endl;
			}
			continue;
		}
		planeHits[stripPlane].push_back(*i);
		
	}

	// do the clustering
	vector<DTRDStripCluster *> clusters; 
	
 	for(uint iplane = 0; iplane < 2; iplane++) {
 
 		if(planeHits[iplane].size()>0){
 		
 			for(int ihit=0; ihit < planeHits[iplane].size(); ihit++) {
 				const DTRDHit* hit = planeHits[iplane][ihit];
 				
				const float CL_DIST=2.7; // mm
							
			double c1 = hit->q;          // energy
			double x1=hit->t;         // UPDATE: probably need to convert
			double y1=StripToPosition(iplane, hit);
			
			if (c1 < CLUSTERING_THRESHOLD) continue;

			// first iteration if c1 > threshold, set clust_Xpos and clust_Zpos
			if (clusters.size() == 0) {
				DTRDStripCluster *new_cluster = new DTRDStripCluster;
				new_cluster->plane = iplane+1;
				new_cluster->q_tot = c1;
				new_cluster->t_avg = hit->t;
				new_cluster->num_hits = 1;
				new_cluster->pos.SetXYZ(y1, 0, x1);
				new_cluster->width.SetXYZ(y1, y1, 0);
				new_cluster->length.SetXYZ(x1, x1, 0);
				
// 					jerr << "NEW cluster" << endl;
// 					jerr << " q_tot = " << new_cluster->q_tot << endl;
// 					jerr << " ";  new_cluster->pos.Print();
				
				clusters.push_back(new_cluster);
				continue;
			}
			
			  int added=0;
			  for (int k=0; k<clusters.size(); k++) {
				double dist=sqrt(pow((y1-clusters[k]->pos.x()),2.)+pow((x1-clusters[k]->pos.z()),2.)); //--- dist hit to clusters
				// check the distance from the x1,y1 to the center of the cluster based on the radius (2.7 mm)
				if (dist<CL_DIST) {
				  // if it's within the radius set this as a new position using weighted average to approximate the new central position
				  clusters[k]->pos.SetX( (y1*c1+clusters[k]->pos.x()*clusters[k]->q_tot)/(c1+clusters[k]->q_tot) );  //--  new X pos
				  clusters[k]->pos.SetZ( (x1*c1+clusters[k]->pos.z()*clusters[k]->q_tot)/(c1+clusters[k]->q_tot) );  //--  new Z pos
				  clusters[k]->t_avg = (c1*hit->t+clusters[k]->t_avg*clusters[k]->q_tot)/(c1+clusters[k]->q_tot);  // time
				  // new dedx is the sum of the two weighted averaged amplitude
				  clusters[k]->q_tot += c1;  // new dEdx
				  clusters[k]->num_hits++;

				  // update cluster width in x and y
				  if (y1<clusters[k]->width.x()) clusters[k]->width.SetX(y1); 
				  if (y1>clusters[k]->width.y()) clusters[k]->width.SetY(y1); 
				  clusters[k]->width.SetZ( clusters[k]->width.y() - clusters[k]->width.x() );
				  if (x1<clusters[k]->length.x()) clusters[k]->length.SetX(x1); 
				  if (x1>clusters[k]->length.y()) clusters[k]->length.SetY(x1); 
				  clusters[k]->length.SetZ( clusters[k]->length.y() - clusters[k]->length.x() );

// 					  jerr << "UPDATE cluster" << endl;
// 					  jerr << " q_tot = " << clusters[k]->q_tot << endl;
// 					  jerr << " ";  clusters[k]->pos.Print();
				
				  added=1; break;
				}
			  }

			// if it's outside the radius, set this as a new center
			  if (added==0) {					
				DTRDStripCluster *new_cluster = new DTRDStripCluster;
				new_cluster->plane = iplane+1;
				new_cluster->q_tot = c1;
				new_cluster->t_avg = hit->t;
				new_cluster->num_hits = 1;
				new_cluster->pos.SetXYZ(y1, 0, x1);
				new_cluster->width.SetXYZ(y1, y1, 0);
				new_cluster->length.SetXYZ(x1, x1, 0);

// 					jerr << "NEW cluster" << endl;
// 					jerr << " q_tot = " << new_cluster->q_tot << endl;
// 					jerr << " ";  new_cluster->pos.Print();
			  }
			}
		} //----------- end  clustering loop ---------------
		
	}


	// Apply quality cuts before we save the clusters
	for (int k=0; k<clusters.size(); k++) {
	  //-------------  Cluster Filter -----------------
	  if (clusters[k]->num_hits>= MinClustSize && zStart < clusters[k]->pos.z() && clusters[k]->pos.z() < zEnd 
	  		&& clusters[k]->width.z()>MinClustWidth )
		results.push_back(clusters[k]);
 	  else 
		delete clusters[k];

	}

	// Ensure that the data are still in order of planes.
	std::sort(results.begin(), results.end(), DTRDStripCluster_gPlane_cmp);
	
	Set(results);
	
	return;	
}			

//-----------------------------
// pique
//-----------------------------
void DTRDStripCluster_factory::cluster(vector<const DTRDHit*>& H)
{
	/// Find clusters within GEM plane.
	///
	/// Upon entry, the vector "H" should already be sorted
	/// by strip number and should only contains hits from
	/// the same plane 

	// Loop over hits
	for(uint32_t istart=0; istart<H.size(); istart++){
		const DTRDHit *first_hit = H[istart];
		
		// Find end of contiguous section
		uint32_t iend=istart+1;
		for(; iend<H.size(); iend++){
			if(iend>=H.size()) break;
			if( (H[iend]->strip - H[iend-1]->strip) > 1 ) break;
		}
		if( (iend-istart)<2 ) continue; // don't allow single strip clusters
		
		// istart should now point to beginning of cluster 
		// and iend to one past end of cluster
		DTRDStripCluster* newCluster = new DTRDStripCluster();
		newCluster->q_tot   = 0.0;
		newCluster->plane   = first_hit->plane;
		for(uint32_t i=istart; i<iend; i++){
			newCluster->q_tot += H[i]->pulse_height;
			newCluster->members.push_back(H[i]);
		}
		Insert(newCluster);
		
		istart = iend-1;
	}
}

