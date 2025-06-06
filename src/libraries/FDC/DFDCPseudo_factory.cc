//********************************************************
// DFDCPseudo_factory.cc - factory producing first-order 
// reconstructed points for the FDC
// Author: Craig Bookwalter (craigb at jlab.org)
// Date:   March 2006
// UVX cathode-wire matching revised by Simon Taylor, Aug 2006
//********************************************************

#include "DFDCPseudo_factory.h"

#include <JANA/JEvent.h>
#include <JANA/Calibrations/JCalibrationManager.h>
#include <JANA/Services/JLockService.h>
#include "DANA/DGeometryManager.h"
#include "HDGEOMETRY/DGeometry.h"

#include "DFDCGeometry.h"
#include <TRACKING/DTrackHitSelectorTHROWN.h>
#include <TROOT.h>
#include <FDC/DFDCCathodeDigiHit.h>

#define HALF_CELL 0.5
#define MAX_DEFLECTION 0.15
#define X0 0
#define QA 1
#define K2 2
#define ITER_MAX 100
#define TOLX 1e-4
#define TOLF 1e-4
#define A_OVER_H 0.4
#define ONE_OVER_H 2.0
#define ALPHA 1e-4 // rate parameter for Newton step backtracking algorithm
#define W_EFF 30.2e-9 // GeV
#define GAS_GAIN 8e4
#define ELECTRON_CHARGE 1.6022e-4 // fC


///
/// DFDCAnode_gLayer_cmp(): 
/// non-member function passed to std::sort() to sort DFDCHit pointers 
/// for the anode wires by their gLayer attributes.
///
bool DFDCAnode_gLayer_cmp(const DFDCHit* a, const DFDCHit* b) {
	return a->gLayer < b->gLayer;
}

bool static fdcxhit_cmp(const DFDCHit *a, const DFDCHit *b){
  if (a->element != b->element) return (a->element < b->element);
  return (a->t < b->t);
}


bool DFDCPseudo_cmp(const DFDCPseudo* a, const DFDCPseudo *b){
  if (a->wire->wire == b->wire->wire && a->wire->layer==b->wire->layer){
    return a->time<b->time;
  }
  if (a->wire->layer!=b->wire->layer) return a->wire->layer<b->wire->layer;
  
  return a->wire->wire<b->wire->wire;
}




///
/// DFDCPseudo_factory::DFDCPseudo_factory():
/// default constructor
///
DFDCPseudo_factory::DFDCPseudo_factory() {
}

///
/// DFDCPseudo_factory::~DFDCPseudo_factory():
/// default destructor
///
DFDCPseudo_factory::~DFDCPseudo_factory() {
  if (fdcwires.size()){
    for (unsigned int i=0;i<fdcwires.size();i++){
      for (unsigned int j=0;j<fdcwires[i].size();j++){
	delete fdcwires[i][j];
      }
    }    
  }
  if (fdccathodes.size()){
    for (unsigned int i=0;i<fdccathodes.size();i++){
      for (unsigned int j=0;j<fdccathodes[i].size();j++){
	delete fdccathodes[i][j];
      }
    }    
  }
}

//------------------
// Init
//------------------
void DFDCPseudo_factory::Init()
{
  RIN_FIDUCIAL = 1.5;
  ROUT_FIDUCIAL=48.0;
  MAX_ALLOWED_FDC_HITS=20000;
  STRIP_ANODE_TIME_CUT=25.;
  MIDDLE_STRIP_THRESHOLD=0.;
  CHARGE_THRESHOLD=0.3;
  DELTA_X_CUT=0.5;

  r2_out=ROUT_FIDUCIAL*ROUT_FIDUCIAL;
  r2_in=RIN_FIDUCIAL*RIN_FIDUCIAL;

  auto app = GetApplication();
  
  app->SetDefaultParameter("FDC:ROUT_FIDUCIAL",ROUT_FIDUCIAL, "Outer fiducial radius of FDC in cm"); 
  app->SetDefaultParameter("FDC:RIN_FIDUCIAL",RIN_FIDUCIAL, "Inner fiducial radius of FDC in cm");
  app->SetDefaultParameter("FDC:MAX_ALLOWED_FDC_HITS",MAX_ALLOWED_FDC_HITS, "Max. number of FDC hits (includes both cathode strips and wires hits) to allow before considering event too busy to attempt FDC tracking");
  app->SetDefaultParameter("FDC:STRIP_ANODE_TIME_CUT",STRIP_ANODE_TIME_CUT, "maximum time difference between strips and wires (in ns)"); 
  app->SetDefaultParameter("FDC:CHARGE_THRESHOLD",CHARGE_THRESHOLD,"Minimum average charge on both cathode planes (in pC)");
  app->SetDefaultParameter("FDC:DELTA_X_CUT",DELTA_X_CUT,"Maximum distance between reconstructed wire position and wire position");

  DEBUG_HISTS = false;
  app->SetDefaultParameter("FDC:DEBUG_HISTS",DEBUG_HISTS);
}


//------------------
// BeginRun
//------------------
void DFDCPseudo_factory::BeginRun(const std::shared_ptr<const JEvent>& event)
{
  auto run_number = event->GetRunNumber();
  auto app = event->GetJApplication();
  auto jcalib = app->GetService<JCalibrationManager>()->GetJCalibration(run_number);
  auto root_lock = app->GetService<JLockService>();
  auto geo_manager = app->GetService<DGeometryManager>();
  auto dgeom = geo_manager->GetDGeometry(run_number);

  USE_FDC=true;
  if (!dgeom->GetFDCWires(fdcwires)){
    _DBG_<< "FDC geometry not available!" <<endl;
    USE_FDC=false;
  }
  if (!dgeom->GetFDCCathodes(fdccathodes)){
    _DBG_<< "FDC geometry not available!" <<endl;
    USE_FDC=false;
  }
  if (USE_FDC){
     // Get package offsets
    vector<double>offsets;
    dgeom->Get("//posXYZ[@volume='forwardDC_package_1']/@X_Y_Z",offsets);
    dX[0]=offsets[0];
    dY[0]=offsets[1];
    dgeom->Get("//posXYZ[@volume='forwardDC_package_2']/@X_Y_Z",offsets);
    dX[1]=offsets[0];
    dY[1]=offsets[1]; 
    dgeom->Get("//posXYZ[@volume='forwardDC_package_3']/@X_Y_Z",offsets);
    dX[2]=offsets[0];
    dY[2]=offsets[1];
    dgeom->Get("//posXYZ[@volume='forwardDC_package_4']/@X_Y_Z",offsets);
    dX[3]=offsets[0];
    dY[3]=offsets[1];
  }
 
  // Get offsets tweaking nominal geometry from calibration database
  vector<map<string,double> >vals;
  if (jcalib->Get("FDC/cell_offsets",vals)==false){
    for(unsigned int i=0; i<vals.size(); i++){
      map<string,double> &row = vals[i];

      // Get the offsets from the calibration database 
      xshifts.push_back(row["xshift"]);
      yshifts.push_back(row["yshift"]);
    }
  }
  // Get FDC resolution parameters from database
  map<string, double> fdcparms;
  FDC_RES_PAR1=0.;
  FDC_RES_PAR2=0.;
  jcalib->Get("FDC/fdc_resolution_parms",fdcparms);
  FDC_RES_PAR1=fdcparms["res_par1"];
  FDC_RES_PAR2=fdcparms["res_par2"];
  
  if(DEBUG_HISTS){
    root_lock->RootWriteLock();

    // Histograms may already exist. (Another thread may have created them)
    // Try and get pointers to the existing ones.
    v_vs_u=(TH2F*)gROOT->FindObject("v_vs_u");
    if (!v_vs_u) v_vs_u=new TH2F("v_vs_u","v vs u",192,0.5,192.5,192,0.5,192.5);

    qv_vs_qu= (TH2F*) gROOT->FindObject("qv_vs_qu");
    if (!qv_vs_qu) qv_vs_qu=new TH2F("qv_vs_qu","Anode charge from each cathode",100,0,20000,100,0,20000);

    tv_vs_tu= (TH2F*) gROOT->FindObject("tv_vs_tu");
    if (!tv_vs_tu) tv_vs_tu=new TH2F("tv_vs_tu","t(v) vs t(u)",100,-100,250,100,-100,250);

    dtv_vs_dtu= (TH2F*) gROOT->FindObject("dtv_vs_dtu");
    if (!dtv_vs_dtu) dtv_vs_dtu=new TH2F("dtv_vs_dtu","t(wire)-t(v) vs t(wire)-t(u)",200,-100,100,200,-100,100);

    u_wire_dt_vs_wire=(TH2F *) gROOT->FindObject("u_wire_dt_vs_wire");
    if (!u_wire_dt_vs_wire) u_wire_dt_vs_wire=new TH2F("u_wire_dt_vs_wire","wire/u cathode time difference vs wire number",
						       96,0.5,96.5,100,-500,500);
    v_wire_dt_vs_wire=(TH2F *) gROOT->FindObject("v_wire_dt_vs_wire");
    if (!v_wire_dt_vs_wire) v_wire_dt_vs_wire=new TH2F("v_wire_dt_vs_wire","wire/v cathode time difference vs wire number",
						       96,0.5,96.5,100,-500,500);
    uv_dt_vs_u=(TH2F *) gROOT->FindObject("uv_dt_vs_u");
    if (!uv_dt_vs_u) uv_dt_vs_u=new TH2F("uv_dt_vs_u","uv time difference vs u",
					 192,0.5,192.5,100,-50,50); 
    uv_dt_vs_v=(TH2F *) gROOT->FindObject("uv_dt_vs_v");
    if (!uv_dt_vs_v) uv_dt_vs_v=new TH2F("uv_dt_vs_v","uv time difference vs v",
					 192,0.5,192.5,100,-50,50);
 
    ut_vs_u=(TH2F *) gROOT->FindObject("ut_vs_u");
    if (!ut_vs_u) ut_vs_u=new TH2F("ut_vs_u","u time  vs u",
				   192,0.5,192.5,100,0,1000); 
    vt_vs_v=(TH2F *) gROOT->FindObject("vt_vs_v");
    if (!vt_vs_v) vt_vs_v=new TH2F("vt_vs_v","v time  vs v",
				   192,0.5,192.5,100,0,1000); 
    
    dx_vs_dE=(TH2F*)gROOT->FindObject("dx_vs_dE");
    if (!dx_vs_dE) dx_vs_dE=new TH2F("dx_vs_dE","dx vs dE",100,0,25.0,
				     100,-0.2,0.2);

    u_cl_size=(TH1F*)gROOT->FindObject("u_cl_size");
    if (!u_cl_size) u_cl_size=new TH1F("u_cl_size","u_cl_size",20,.5,20.5);
    v_cl_size=(TH1F*)gROOT->FindObject("v_cl_size");
    if (!v_cl_size) v_cl_size=new TH1F("v_cl_size","v_cl_size",20,.5,20.5);

    u_cl_n=(TH1F*)gROOT->FindObject("u_cl_n");
    if (!u_cl_n) u_cl_n=new TH1F("u_cl_n","u_cl_n",20,.5,20.5);
    v_cl_n=(TH1F*)gROOT->FindObject("v_cl_n");
    if (!v_cl_n) v_cl_n=new TH1F("v_cl_n","v_cl_n",20,.5,20.5);

    x_dist_2=(TH1F*)gROOT->FindObject("x_dist_2");
    if (!x_dist_2) x_dist_2=new TH1F("x_dist_2","x_dist_2",400,-2,2);

    x_dist_3=(TH1F*)gROOT->FindObject("x_dist_3");
    if (!x_dist_3) x_dist_3=new TH1F("x_dist_3","x_dist_3",400,-2,2);

    x_dist_23=(TH1F*)gROOT->FindObject("x_dist_23");
    if (!x_dist_23) x_dist_23=new TH1F("x_dist_23","x_dist_23",400,-2,2);

    x_dist_33=(TH1F*)gROOT->FindObject("x_dist_33");
    if (!x_dist_33) x_dist_33=new TH1F("x_dist_33","x_dist_33",400,-2,2);

    d_uv=(TH1F*)gROOT->FindObject("d_uv");
    if (!d_uv) d_uv=new TH1F("d_uv","d_uv",100,0,50);


    for (unsigned int i=0;i<24;i++){
      char hname[80];
      sprintf(hname,"Hxy%d",i);
      Hxy[i]=(TH2F *) gROOT->FindObject(hname);
      if (!Hxy[i]){
	Hxy[i]=new TH2F(hname,hname,4000,-50,50,200,-50,50);
      }
    }
    root_lock->RootUnLock();
  }
}

void DFDCPseudo_factory::EndRun(){
  if (fdcwires.size()){
    for (unsigned int i=0;i<fdcwires.size();i++){
      for (unsigned int j=0;j<fdcwires[i].size();j++){
	delete fdcwires[i][j];
      }
    }    
  }
  fdcwires.clear();
  if (fdccathodes.size()){
    for (unsigned int i=0;i<fdccathodes.size();i++){
      for (unsigned int j=0;j<fdccathodes[i].size();j++){
	delete fdccathodes[i][j];
      }
    }    
  }
  fdccathodes.clear();
}


///
/// DFDCPseudo_factory::Process
/// this is the place that anode hits and DFDCCathodeClusters are organized into pseudopoints.
///
void DFDCPseudo_factory::Process(const std::shared_ptr<const JEvent>& event) {
  if (!USE_FDC) return;

	// Get all FDC hits (anode and cathode)	
	vector<const DFDCHit*> fdcHits;
	event->Get(fdcHits);
	if (fdcHits.size()==0) return;

	// For events with a very large number of hits, assume
	// we can't reconstruct them so bail early
	// Feb. 8, 2008  D.L. (updated to config param. Nov. 18, 2010 D.L.)
	if(fdcHits.size()>MAX_ALLOWED_FDC_HITS){
		_DBG_<<"Too many hits in FDC ("<<fdcHits.size()<<", max="<<MAX_ALLOWED_FDC_HITS<<")! Pseudopoint reconstruction in FDC bypassed for event "<<event->GetEventNumber()<<endl;
		return;
	}

	// Get cathode clusters
	vector<const DFDCCathodeCluster*> cathClus;
	event->Get(cathClus);
	if (cathClus.size()==0) return;

	// Sift through hits and select out anode hits.
	vector<const DFDCHit*> xHits;
	for (unsigned int i=0; i < fdcHits.size(); i++)
		if (fdcHits[i]->type == 0)
			xHits.push_back(fdcHits[i]);
	// Make sure the wires are also in order of ascending z position
	std::sort(xHits.begin(), xHits.end(), DFDCAnode_gLayer_cmp);
			
	// Sift through clusters and put U and V clusters into respective vectors.
	vector<const DFDCCathodeCluster*> uClus;	
	vector<const DFDCCathodeCluster*> vClus;
	for (unsigned int i=0; i < cathClus.size(); i++) {
		if (cathClus[i]->plane == 1)
			vClus.push_back(cathClus[i]);
		else
			uClus.push_back(cathClus[i]);
	}
	
	// If this is simulated data then we want to match up the truth hit
	// with this "real" hit. Ideally, this would be done at the
	// DFDCHit object level, but the organization of the data in HDDM
	// makes that difficult. Here we have the full wire definition so
	// we make the connection here.
	vector<const DMCTrackHit*> mctrackhits;
	event->Get(mctrackhits);

	vector<const DFDCCathodeCluster*>::iterator uIt = uClus.begin();
	vector<const DFDCCathodeCluster*>::iterator vIt = vClus.begin();
	vector<const DFDCHit*>::iterator xIt = xHits.begin();
	
	// For each layer, get its sets of V, X, and U hits, and then pass them to the geometrical
	// organization routine, DFDCPseudo_factory::makePseudo()
	vector<const DFDCCathodeCluster*> oneLayerU;
	vector<const DFDCCathodeCluster*> oneLayerV;
	vector<const DFDCHit*> oneLayerX;
	for (int iLayer=1; iLayer <= 24; iLayer++) {
	  for (; ((uIt != uClus.end() && (*uIt)->gLayer == iLayer)); uIt++)
	    oneLayerU.push_back(*uIt);
	  for (; ((vIt != vClus.end() && (*vIt)->gLayer == iLayer)); vIt++)
	    oneLayerV.push_back(*vIt);
	  for (; ((xIt != xHits.end() && (*xIt)->gLayer == iLayer)); xIt++)
	    oneLayerX.push_back(*xIt);
	  if (oneLayerU.size()>0 && oneLayerV.size()>0 && oneLayerX.size()>0)
	    makePseudo(oneLayerX, oneLayerU, oneLayerV,iLayer, mctrackhits);
	  oneLayerU.clear();
	  oneLayerV.clear();
	  oneLayerX.clear();
	}
	// Make sure the data are both time- and z-ordered
	std::sort(mData.begin(),mData.end(),DFDCPseudo_cmp);
}

/// 
/// DFDCPseudo_factory::makePseudo():
/// performs UV+X matching to create pseudopoints
///
void DFDCPseudo_factory::makePseudo(vector<const DFDCHit*>& x,
				    vector<const DFDCCathodeCluster*>& u,
				    vector<const DFDCCathodeCluster*>& v,
				    int layer,
				    vector<const DMCTrackHit*> &mctrackhits)
{
  vector<const DFDCHit*>::iterator xIt;
  vector<centroid_t>upeaks;
  vector<centroid_t>vpeaks;

  sort(x.begin(), x.end(), fdcxhit_cmp);

  //printf("---------u clusters --------\n");
  // Loop over all U and V clusters looking for peaks
  for (unsigned int i=0;i<u.size();i++){
    //printf("Cluster %d\n",i);
    if (DEBUG_HISTS) u_cl_size->Fill(u[i]->members.size());
    if (u[i]->members.size()>2){
      for (vector<const DFDCHit*>::const_iterator strip=u[i]->members.begin()+1;strip+1!=u[i]->members.end();strip++){  
	      //printf("  %d %f %f\n",(*strip)->element,(*strip)->pulse_height,(*strip)->t);
         if (FindCentroid(u[i]->members,strip,upeaks)==NOERROR){
            // Some values needed for cathode alignment
            unsigned int index=2*((*strip)->gLayer-1)+(1-(*strip)->plane/2);
            DMatrix3x1 XTemp,NTemp,NTempRaw,indexTemp; 
            XTemp(0) = fdccathodes[index][(*(strip-1))->element-1]->u;
            XTemp(1) = fdccathodes[index][(*(strip))->element-1]->u;
            XTemp(2) = fdccathodes[index][(*(strip+1))->element-1]->u;
            NTemp(0) = double ((*(strip-1))->pulse_height);
            NTemp(1) = double ((*(strip))->pulse_height);
            NTemp(2) = double ((*(strip+1))->pulse_height);
            NTempRaw(0) = double ((*(strip-1))->pulse_height_raw);
            NTempRaw(1) = double ((*(strip))->pulse_height_raw);
            NTempRaw(2) = double ((*(strip+1))->pulse_height_raw);
            indexTemp(0) = (*(strip-1))->element;
            indexTemp(1) = (*(strip))->element;
            indexTemp(2) = (*(strip+1))->element;
            upeaks[upeaks.size()-1].cluster=i;
            upeaks[upeaks.size()-1].X = XTemp;
            upeaks[upeaks.size()-1].N = NTemp;
            upeaks[upeaks.size()-1].NRaw = NTempRaw;
            upeaks[upeaks.size()-1].index = indexTemp;
         }
         else if (u[i]->members.size()==3){
            if (ThreeStripCluster(u[i]->members,strip,upeaks)==NOERROR){
               upeaks[upeaks.size()-1].cluster=i;
            }
         }
      }
    }
    else if (u[i]->members.size()==2){
       for (vector<const DFDCHit*>::const_iterator strip=u[i]->members.begin();strip+1!=u[i]->members.end();strip++){  
          if (TwoStripCluster(u[i]->members,strip,upeaks)==NOERROR){
             upeaks[upeaks.size()-1].cluster=i;
          }
       }
    }
  }  
  //  printf("---------v cluster --------\n");	
  for (unsigned int i=0;i<v.size();i++){
     //printf("Cluster %d\n",i);
     if (DEBUG_HISTS) v_cl_size->Fill(v[i]->members.size());
     if (v[i]->members.size()>2){
        for (vector<const DFDCHit*>::const_iterator strip=v[i]->members.begin()+1;strip+1!=v[i]->members.end();strip++){		
           //printf("  %d %f %f\n",(*strip)->element,(*strip)->pulse_height,(*strip)->t);
           if (FindCentroid(v[i]->members,strip,vpeaks)==NOERROR){
              // Some values needed for cathode alignment
              unsigned int index=2*((*strip)->gLayer-1)+(1-(*strip)->plane/2);
              DMatrix3x1 XTemp,NTemp,NTempRaw,indexTemp;
              XTemp(0) = fdccathodes[index][(*(strip-1))->element-1]->u;
              XTemp(1) = fdccathodes[index][(*(strip))->element-1]->u;
              XTemp(2) = fdccathodes[index][(*(strip+1))->element-1]->u;
              NTemp(0) = double ((*(strip-1))->pulse_height);
              NTemp(1) = double ((*(strip))->pulse_height);
              NTemp(2) = double ((*(strip+1))->pulse_height);
              NTempRaw(0) = double ((*(strip-1))->pulse_height_raw);
              NTempRaw(1) = double ((*(strip))->pulse_height_raw);
              NTempRaw(2) = double ((*(strip+1))->pulse_height_raw);
              indexTemp(0) = (*(strip-1))->element;
              indexTemp(1) = (*(strip))->element;
              indexTemp(2) = (*(strip+1))->element;
              vpeaks[vpeaks.size()-1].cluster=i;
              vpeaks[vpeaks.size()-1].X = XTemp;
              vpeaks[vpeaks.size()-1].N = NTemp;
              vpeaks[vpeaks.size()-1].NRaw = NTempRaw;
              vpeaks[vpeaks.size()-1].index = indexTemp;
           }
           else if (v[i]->members.size()==3){
              if (ThreeStripCluster(v[i]->members,strip,vpeaks)==NOERROR){
                 vpeaks[vpeaks.size()-1].cluster=i;
              }
           }
        }
     }
     else if (v[i]->members.size()==2){
        for (vector<const DFDCHit*>::const_iterator strip=v[i]->members.begin();strip+1!=v[i]->members.end();strip++){  
           if (TwoStripCluster(v[i]->members,strip,vpeaks)==NOERROR){
              vpeaks[vpeaks.size()-1].cluster=i;
           }
        }
     }
  }
  if (upeaks.size()*vpeaks.size()>0){
     // Rotation angles for strips
     unsigned int ilay=layer-1;
     unsigned int ind=2*ilay;
     float phi_u=fdccathodes[ind][0]->angle;
     float phi_v=fdccathodes[ind+1][0]->angle;

     //Loop over all u and v centroids looking for matches with wires
     for (unsigned int i=0;i<upeaks.size();i++){  
        for (unsigned int j=0;j<vpeaks.size();j++){
           // In the layer local coordinate system, wires are quantized 
           // in the x-direction and y is along the wire.
           double x_from_strips=DFDCGeometry::getXLocalStrips(upeaks[i].pos,phi_u,
                 vpeaks[j].pos,phi_v);
           double y_from_strips=DFDCGeometry::getYLocalStrips(upeaks[i].pos,phi_u,
                 vpeaks[j].pos,phi_v);
           int old_wire_num=-1;
           for(xIt=x.begin();xIt!=x.end();xIt++){
              if ((*xIt)->element<=WIRES_PER_PLANE && (*xIt)->element>0){
                 const DFDCWire *wire=fdcwires[layer-1][(*xIt)->element-1];
                 double x_from_wire=wire->u;

                 //printf("xs %f xw %f\n",x_from_strips,x_from_wire);

                 // Test radial value for checking whether or not the hit is within
                 // the fiducial region of the detector
                 double r2test=x_from_wire*x_from_wire+y_from_strips*y_from_strips;
                 double delta_x=x_from_wire-x_from_strips;

                 if (DEBUG_HISTS){
                    if (upeaks[i].numstrips == 3 && vpeaks[j].numstrips == 3) x_dist_3->Fill(delta_x);
                    if (upeaks[i].numstrips == 2 && vpeaks[j].numstrips == 2) x_dist_2->Fill(delta_x);
                    if (upeaks[i].numstrips == 2 && vpeaks[j].numstrips == 3) x_dist_23->Fill(delta_x);
                    else if (upeaks[i].numstrips == 3 && vpeaks[j].numstrips == 2) x_dist_23->Fill(delta_x);

                    if (upeaks[i].numstrips == 3 && vpeaks[j].numstrips == 10) x_dist_33->Fill(delta_x);
                    else if (upeaks[i].numstrips == 10 && vpeaks[j].numstrips == 3) x_dist_33->Fill(delta_x);
                 }

                 // Match between this wire and cathodes below, skip all other hits
                 if (old_wire_num==(*xIt)->element) continue;

                 if (fabs(delta_x)<DELTA_X_CUT && r2test<r2_out
                       && r2test>r2_in){
                    double dt1 = (*xIt)->t - upeaks[i].t;
                    double dt2 = (*xIt)->t - vpeaks[j].t;

                    //printf("dt1 %f dt2 %f\n",dt1,dt2);

                    if (DEBUG_HISTS){
                       //if (layer==1){
                       dtv_vs_dtu->Fill(dt1,dt2);
                       tv_vs_tu->Fill(upeaks[i].t, vpeaks[j].t);
                       u_wire_dt_vs_wire->Fill((*xIt)->element,(*xIt)->t-upeaks[i].t);
                       v_wire_dt_vs_wire->Fill((*xIt)->element,(*xIt)->t-vpeaks[j].t);


                       int uid=u[upeaks[i].cluster]->members[1]->element;
                       int vid=v[vpeaks[j].cluster]->members[1]->element;

                       const DFDCCathodeDigiHit *vdigihit;
                       v[vpeaks[j].cluster]->members[1]->GetSingle(vdigihit);
                       const DFDCCathodeDigiHit *udigihit;
                       u[upeaks[i].cluster]->members[1]->GetSingle(udigihit);
                       if (vdigihit!=NULL && udigihit!=NULL){
                          int dt=int(udigihit->pulse_time)-int(vdigihit->pulse_time);
                          // printf("%d %d\n",udigihit->pulse_time,vdigihit->pulse_time);
                          uv_dt_vs_u->Fill(uid,dt);
                          uv_dt_vs_v->Fill(vid,dt);
                          v_vs_u->Fill(uid,vid);
                          ut_vs_u->Fill(uid,udigihit->pulse_time);
                          vt_vs_v->Fill(vid,udigihit->pulse_time);
                       }
                       //  Hxy->Fill(x_from_strips,y_from_strips);
                       //}
                    }

                    if (DEBUG_HISTS){
                       d_uv->Fill(sqrt(dt1*dt1+dt2*dt2));
                    }

                    if (sqrt(dt1*dt1+dt2*dt2)>STRIP_ANODE_TIME_CUT) continue;

                    // Temporary cut until TDC timing is worked out
                    //if (fabs(vpeaks[j].t-upeaks[i].t)>STRIP_ANODE_TIME_CUT) continue;

                    if (DEBUG_HISTS){
                       // number of clusters of each type
                       u_cl_n->Fill(upeaks[i].numstrips);
                       v_cl_n->Fill(vpeaks[j].numstrips);
                    }

                    // Charge and energy loss
                    double q_cathodes=0.5*(upeaks[i].q+vpeaks[j].q);
                    double charge_to_energy=W_EFF/(GAS_GAIN*ELECTRON_CHARGE);
                    double dE=charge_to_energy*q_cathodes;
                    double q_from_pulse_height=5.0e-4*(upeaks[i].q_from_pulse_height
                          +vpeaks[j].q_from_pulse_height);
                    if (q_from_pulse_height<CHARGE_THRESHOLD) continue;
		    double dE_amp=charge_to_energy*q_from_pulse_height;

                    if (DEBUG_HISTS){
                       qv_vs_qu->Fill(upeaks[i].q,vpeaks[j].q);
                    }

                    // Match between this wire and cathodes, used to skip all other hits above
                    old_wire_num=(*xIt)->element;

                    int status=upeaks[i].numstrips+vpeaks[j].numstrips;
                    //double xres=WIRE_SPACING/2./sqrt(12.);

                    DFDCPseudo* newPseu = new DFDCPseudo;     
                    newPseu->phi_u=phi_u;
                    newPseu->phi_v=phi_v;
                    newPseu->u = upeaks[i].pos;
                    newPseu->v = vpeaks[j].pos;
                    newPseu->t_u = upeaks[i].t;
                    newPseu->t_v = vpeaks[j].t;
                    newPseu->cluster_u = upeaks[i];
                    newPseu->cluster_v = vpeaks[j];
                    newPseu->w      = x_from_wire+xshifts[ilay];
                    newPseu->dw     = 0.; // place holder
                    newPseu->w_c    = x_from_strips+xshifts[ilay];
                    newPseu->s      = y_from_strips+yshifts[ilay];
                    newPseu->ds = FDC_RES_PAR1/q_from_pulse_height+FDC_RES_PAR2;
                    //newPseu->ds=0.011/q_from_pulse_height+5e-3+2.14e-10*pow(q_from_pulse_height,6);
                    newPseu->wire   = wire;
                    newPseu->time   = (*xIt)->t;
                    //newPseu->time=0.5*(upeaks[i].t+vpeaks[j].t);
                    newPseu->status = status;
                    newPseu->itrack = (*xIt)->itrack;

                    newPseu->AddAssociatedObject(v[vpeaks[j].cluster]);
                    newPseu->AddAssociatedObject(u[upeaks[i].cluster]);

                    newPseu->dE = dE;
		    newPseu->dE_amp = dE_amp;
                    newPseu->q = q_from_pulse_height;

                    // It can occur (although rarely) that newPseu->wire is NULL
                    // which causes us to crash below. In these cases, we can't really
                    // make a psuedo point so we delete the current object
                    // and just go on to the next one.
                    if(newPseu->wire==NULL){
                       _DBG_<<"newPseu->wire=NULL! This shouldn't happen. Complain to staylor@jlab.org"<<endl;
                       delete newPseu;
                       continue;
                    }
                    double sinangle=newPseu->wire->udir(0);
                    double cosangle=newPseu->wire->udir(1); 
                    unsigned int pack_id=(layer-1)/6;
                    newPseu->s+=dY[pack_id]*cosangle+dX[pack_id]*sinangle; 

                    newPseu->xy.Set((newPseu->w)*cosangle+(newPseu->s)*sinangle,
                          -(newPseu->w)*sinangle+(newPseu->s)*cosangle);

                    double sigx2=HALF_CELL*HALF_CELL/3.;
                    double sigy2=MAX_DEFLECTION*MAX_DEFLECTION/3.;
                    newPseu->covxx=sigx2*cosangle*cosangle+sigy2*sinangle*sinangle;
                    newPseu->covyy=sigx2*sinangle*sinangle+sigy2*cosangle*cosangle;
                    newPseu->covxy=(sigy2-sigx2)*sinangle*cosangle;

                    // Try matching truth hit with this "real" hit.
                    const DMCTrackHit *mctrackhit = DTrackHitSelectorTHROWN::GetMCTrackHit(newPseu->wire, DRIFT_SPEED*newPseu->time, mctrackhits);
                    if(mctrackhit)newPseu->AddAssociatedObject(mctrackhit);

                    Insert(newPseu);

                    if (DEBUG_HISTS){
                       Hxy[ilay]->Fill(newPseu->w_c,newPseu->s);
                       if (ilay==6) dx_vs_dE->Fill(q_from_pulse_height,0.2588*delta_x);
                    }

                 } // match in x
              } else _DBG_ << "Bad wire " << (*xIt)->element <<endl;
           } // xIt loop
        } // vpeaks loop
     } // upeaks loop
  } // if we have peaks in both u and v views

}			

//
/// DFDCPseudo_factory::CalcMeanTime()
/// Calculate the mean and rms of the times of the hits passed in "H".
/// The contents of H should be pointers to a single cluster in a
/// cathode plane. 
//  1/2/2008 D.L.
void DFDCPseudo_factory::CalcMeanTime(const vector<const DFDCHit*>& H, double &t, double &t_rms)
{
   // Calculate mean
   t=0.0;
   for(unsigned int i=0; i<H.size(); i++)t+=H[i]->t;
   if(H.size()>0)t/=(double)H.size();

   // Calculate RMS
   t_rms=0.0;
   for(unsigned int i=0; i<H.size(); i++)t_rms+=pow((double)(H[i]->t-t),2.0);
   if(H.size()>0)t_rms = sqrt(t_rms/(double)H.size());
}

// Find the mean time and rms for a group of 3 hits with a maximum in the 
// center hit
void DFDCPseudo_factory::CalcMeanTime(vector<const DFDCHit *>::const_iterator peak, double &t, double &t_rms)
{
   // Calculate mean
   t=0.0;
   for (vector<const DFDCHit*>::const_iterator j=peak-1;j<=peak+1;j++){
      t+=(*j)->t;
   }
   t/=3.;

   // Calculate RMS
   t_rms=0.0;
   for (vector<const DFDCHit*>::const_iterator j=peak-1;j<=peak+1;j++){
      t_rms+=((*j)->t-t)*((*j)->t-t);
   }
   t_rms=sqrt(t_rms/3.);
}


//
/// DFDCPseudo_factory::FindCentroid()
///   Uses the Newton-Raphson method for solving the set of non-linear
/// equations describing the charge distribution over 3 strips for the peak 
/// position x0, the anode charge qa, and the "width" parameter k2.  
/// See Numerical Recipes in C p.379-383.
/// Updates list of centroids. 
///
jerror_t DFDCPseudo_factory::FindCentroid(const vector<const DFDCHit*>& H,
      vector<const DFDCHit *>::const_iterator peak,
      vector<centroid_t>&centroids){
   centroid_t temp;
   double err_diff1=0.,err_diff2=0.;

   // Fill in time info in temp  1/2/2008 D.L.
   //CalcMeanTime(H, temp.t, temp.t_rms);

   // Some code for checking for significance of fluctuations.
   // Currently disabled.
   //double dq1=(*(peak-1))->dq;
   //double dq2=(*peak)->dq;
   //double dq3=(*(peak+1))->dq;
   //err_diff1=sqrt(dq1*dq1+dq2*dq2);
   //err_diff2=sqrt(dq2*dq2+dq3*dq3);
   if ((*peak)->pulse_height<MIDDLE_STRIP_THRESHOLD) {
      return VALUE_OUT_OF_RANGE;
   }

   // Check for a peak in three adjacent strips
   if ((*peak)->pulse_height-(*(peak-1))->pulse_height > err_diff1
         && (*peak)->pulse_height-(*(peak+1))->pulse_height > err_diff2){
      // Define some matrices for use in the Newton-Raphson iteration
      DMatrix3x3 J;  //Jacobean matrix
      DMatrix3x1 F,N,X,par,newpar,f;
      int i=0;
      double sum=0.;

      // Initialize the matrices to some suitable starting values
      unsigned int index=2*((*peak)->gLayer-1)+(1-(*peak)->plane/2);
      par(K2)=1.;
      for (vector<const DFDCHit*>::const_iterator j=peak-1;j<=peak+1;j++){
         X(i)=fdccathodes[index][(*j)->element-1]->u;
         N(i)=double((*j)->pulse_height);
         sum+=N(i);
         i++;
      }
      par(X0)=X(1);
      par(QA)=2.*sum;
      newpar=par;

      // Newton-Raphson procedure
      double errf=0.,errx=0;
      for (int iter=1;iter<=ITER_MAX;iter++){
         errf=0.;
         errx=0.;

         // Compute Jacobian matrix: J_ij = dF_i/dx_j.
         for (i=0;i<3;i++){
            double dx=(par(X0)-X(i))*ONE_OVER_H;
            double argp=par(K2)*(dx+A_OVER_H);
            double argm=par(K2)*(dx-A_OVER_H);
            double tanh_p=tanh(argp);
            double tanh_m=tanh(argm);
            double tanh2_p=tanh_p*tanh_p;
            double tanh2_m=tanh_m*tanh_m;
            double q_over_4=0.25*par(QA);

            f(i)=tanh_p-tanh_m;
            J(i,QA)=-0.25*f(i);
            J(i,K2)=-q_over_4*(argp/par(K2)*(1.-tanh2_p)
                  -argm/par(K2)*(1.-tanh2_m));
            J(i,X0)=-q_over_4*par(K2)*(tanh2_m-tanh2_p); 
            F(i)=N(i)-q_over_4*f(i);
            double new_errf=fabs(F(i));
            if (new_errf>errf) errf=new_errf;
         }
         // Check for convergence
         if (errf<TOLF){	
            temp.pos=par(X0);
            temp.q_from_pulse_height=par(QA);
            temp.numstrips=3;  
            temp.t=(*peak)->t;
            temp.t_rms=0.;

            // Find estimate for anode charge
            sum=0;
            double sum_f=f(0)+f(1)+f(2);
            for (vector<const DFDCHit*>::const_iterator j=peak-1;j<=peak+1;j++){
               sum+=double((*j)->q);
            }
            temp.q=4.*sum/sum_f;

            //CalcMeanTime(peak,temp.t,temp.t_rms);
            centroids.push_back(temp);

            return NOERROR;
         }

         // Find the new set of parameters
         FindNewParmVec(N,X,F,J,par,newpar);

         //Check for convergence
         for (i=0;i<3;i++){
            double new_err=fabs(par(i)-newpar(i));
            if (new_err>errx) errx=new_err;
         }
         if (errx<TOLX){
            temp.pos=par(X0);
            temp.numstrips=3;
            temp.q_from_pulse_height=par(QA);
            temp.t=(*peak)->t;
            temp.t_rms=0.;

            // Find estimate for anode charge
            sum=0;
            double sum_f=f(0)+f(1)+f(2);
            for (vector<const DFDCHit*>::const_iterator j=peak-1;j<=peak+1;j++){
               sum+=double((*j)->q);
            }
            temp.q=4.*sum/sum_f;

            //CalcMeanTime(peak,temp.t,temp.t_rms);
            centroids.push_back(temp);

            return NOERROR;
         }
         par=newpar;
      } // iterations
   }
   return INFINITE_RECURSION; // error placeholder
}


///
/// DFDCPseudo_factory::FindNewParmVec()
///   Routine used by FindCentroid to backtrack along the direction
/// of the Newton step if the step is too large in order to avoid conditions
/// where the iteration procedure starts to diverge.
/// The procedure uses a scalar quantity f= 1/2 F.F for this purpose.
/// Algorithm described in Numerical Recipes in C, pp. 383-389.
///

jerror_t DFDCPseudo_factory::FindNewParmVec(const DMatrix3x1 &N,
      const DMatrix3x1 &X,
      const DMatrix3x1 &F,
      const DMatrix3x3 &J,
      const DMatrix3x1 &par,
      DMatrix3x1 &newpar){
   // Invert the J matrix
   DMatrix3x3 InvJ=J.Invert();

   // Find the full Newton step
   DMatrix3x1 fullstep=(-1.)*(InvJ*F);

   // find the rate of decrease for the Newton-Raphson step
   double slope=(-1.0)*F.Mag2(); //dot product

   // This should be a negative number...
   if (slope>=0){
      return VALUE_OUT_OF_RANGE;
   }

   double lambda=1.0;  // Start out with full Newton step
   double lambda_temp,lambda2=lambda;
   DMatrix3x1 newF;
   double f2=0.,newf;

   // Compute starting values for f=1/2 F.F 
   double f=-0.5*slope;

   for (;;){
      newpar=par+lambda*fullstep;

      // Compute the value of the vector F and f=1/2 F.F with the current step
      for (int i=0;i<3;i++){
         double dx=(newpar(X0)-X(i))*ONE_OVER_H;
         double argp=newpar(K2)*(dx+A_OVER_H);
         double argm=newpar(K2)*(dx-A_OVER_H);
         newF(i)=N(i)-0.25*newpar(QA)*(tanh(argp)-tanh(argm));
      }
      newf=0.5*newF.Mag2(); // dot product

      if (lambda<0.1) {  // make sure the step is not too small
         newpar=par;
         return NOERROR;
      } // Check if we have sufficient function decrease
      else if (newf<=f+ALPHA*lambda*slope){
         return NOERROR;
      }
      else{
         // g(lambda)=f(par+lambda*fullstep)
         if (lambda==1.0){//first attempt: quadratic approximation for g(lambda)
            lambda_temp=-0.5*slope/(newf-f-slope);
         }
         else{ // cubic approximation for g(lambda)
            double temp1=newf-f-lambda*slope;
            double temp2=f2-f-lambda2*slope;
            double one_over_lambda2_sq=1./(lambda2*lambda2);
            double one_over_lambda_sq=1./(lambda*lambda);
            double one_over_lambda_diff=1./(lambda-lambda2);
            double a=(temp1*one_over_lambda_sq-temp2*one_over_lambda2_sq)*one_over_lambda_diff;
            double b=(-lambda2*temp1*one_over_lambda_sq+lambda*temp2*one_over_lambda2_sq)
               *one_over_lambda_diff;
            if (a==0.0) lambda_temp=-0.5*slope/b;
            else{
               double disc=b*b-3.0*a*slope;
               if (disc<0.0) lambda_temp=0.5*lambda;
               else if (b<=0.0) lambda_temp=(-b+sqrt(disc))/(3.*a);
               else lambda_temp=-slope/(b+sqrt(disc));
            }
            // ensure that we are headed in the right direction...
            if (lambda_temp>0.5*lambda) lambda_temp=0.5*lambda;
         }
      }
      lambda2=lambda;
      f2=newf;
      // Make sure that new version of lambda is not too small
      lambda=(lambda_temp>0.1*lambda ? lambda_temp : 0.1*lambda);
   } 
}


//
/// DFDCPseudo_factory::TwoStripCluster()
/// Almost 10% of all clusters have only two strips
/// But FindCentroid does not work
/// Attempt to recover them with simple center-of-gravity
/// Updates list of centroids. 
///
jerror_t DFDCPseudo_factory::TwoStripCluster(const vector<const DFDCHit*>& H,
      vector<const DFDCHit *>::const_iterator peak,
      vector<centroid_t>&centroids){
   centroid_t temp;

   if ((*peak)->pulse_height<MIDDLE_STRIP_THRESHOLD && (*(peak+1))->pulse_height<MIDDLE_STRIP_THRESHOLD) {
      return VALUE_OUT_OF_RANGE;
   }

   unsigned int index1=2*((*peak)->gLayer-1)+(1-(*peak)->plane/2);
   unsigned int index2=2*((*(peak+1))->gLayer-1)+(1-(*(peak+1))->plane/2);

   // this should never happen
   if (index1 != index2) return VALUE_OUT_OF_RANGE;

   double pos1=fdccathodes[index1][(*peak)->element-1]->u;
   double pos2=fdccathodes[index2][(*(peak+1))->element-1]->u;

   double amp1=double((*peak)->pulse_height);
   double amp2=double((*(peak+1))->pulse_height);
   double sum=amp1+amp2;

   double t1=double((*peak)->t);
   double t2=double((*(peak+1))->t);

   // weighted sum
   temp.pos=(pos1*amp1+pos2*amp2)/sum;

   //correct for 'missing' signals on the other side
   //largest amp on the left: -
   //largest amp on the right: +
   double pos_corr = 0.05;
   (amp2 > amp1) ? temp.pos+=pos_corr : temp.pos-=pos_corr;

   (amp2 > amp1) ? temp.q_from_pulse_height=amp2 : temp.q_from_pulse_height=amp1;
   temp.q=sum;
   temp.numstrips=2;

   // time from greater amplitude
   (amp2 > amp1) ? temp.t=t2 : temp.t=t1;
   temp.t_rms=0.;

   //CalcMeanTime(peak,temp.t,temp.t_rms);
   centroids.push_back(temp);

   return NOERROR;
}

//
/// DFDCPseudo_factory::ThreeStripCluster()
/// But FindCentroid does not work for Clusters without peak
/// Attempt to recover them with simple center-of-gravity
/// Updates list of centroids. 
///
jerror_t DFDCPseudo_factory::ThreeStripCluster(const vector<const DFDCHit*>& H,
      vector<const DFDCHit *>::const_iterator peak,
      vector<centroid_t>&centroids){
   centroid_t temp;

   if ((*(peak-1))->pulse_height<MIDDLE_STRIP_THRESHOLD &&
         (*peak)->pulse_height<MIDDLE_STRIP_THRESHOLD &&
         (*(peak+1))->pulse_height<MIDDLE_STRIP_THRESHOLD) {
      return VALUE_OUT_OF_RANGE;
   }

   double sum=0;
   double wsum=0;
   double t=0;
   double o_amp=0;
   int i_corr=-2;
   double q_from_pulse_height = 0;
   for (vector<const DFDCHit*>::const_iterator j=peak-1;j<=peak+1;j++){
      unsigned int index=2*((*j)->gLayer-1)+(1-(*j)->plane/2);

      double pos=fdccathodes[index][(*j)->element-1]->u;
      double amp=double((*j)->pulse_height);

      sum+=amp;
      wsum+=amp*pos;

      // time, pulseheight and correction from largest amplitude
      if (amp > o_amp){
         t=double((*j)->t);
         o_amp = amp;
         q_from_pulse_height=amp;
         i_corr++;
      }
   }

   //correct for 'missing' signals on the other side
   //largest amp on the left: -
   //largest amp on the right: +
   //minimum in the centre: 0
   double pos_corr = 0.1;

   // weighted sum
   temp.pos=wsum/sum + i_corr*pos_corr;

   temp.q=sum;
   temp.q_from_pulse_height = q_from_pulse_height;
   temp.numstrips=10;
   temp.t=t;
   temp.t_rms=0.;

   //CalcMeanTime(peak,temp.t,temp.t_rms);
   centroids.push_back(temp);

   return NOERROR;
}

