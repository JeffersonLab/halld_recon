// Author: David Lawrence  June 25, 2004
//
//
// EventViewer.h
//
/// Example program for a Hall-D analyzer which uses DANA
///

#ifndef _EVENTVIEWER_H_
#define _EVENTVIEWER_H_

#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <GlueX.h>

#include <HDGEOMETRY/DMagneticFieldMap.h>
#include <HDGEOMETRY/DRootGeom.h>
#include <HDGEOMETRY/DGeometry.h>
#include <PID/DNeutralParticle.h>
#include <PID/DKinematicData.h>
#include <DCoordinateSystem.h>
#include <TRACKING/DReferenceTrajectory.h>
#include "DReferenceTrajectoryHDV.h"
#include <FMWPC/DFMWPCDigiHit.h>
#include <FMWPC/DFMWPCHit.h>
#include <FMWPC/DFMWPCCluster.h>
#include <FMWPC/DFMWPCMatchedTrack.h>

class DQuickFit;
class DTrackCandidate_factory;
class DCDCTrackHit;

#include "hdv_mainframe.h"

class hdv_fulllistframe;
class hdv_debugerframe;

#include <TPolyLine.h>
#include <TEllipse.h>
#include <TVector3.h>
#include <TMarker.h>
#include <TText.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2F.h>
#include <TCanvas.h>
#define MAX_HIT_MARKERS 2000
#define MAX_LINES 100
#define MAX_CIRCLES 100


class EventViewer:public JEventProcessor
{

public:

    enum RequiredClassesLogic {
        REQUIRED_CLASSES_LOGIC_OR,
        REQUIRED_CLASSES_LOGIC_AND
    };
    enum poly_type {
        kMarker =0,
        kLine   =1
    };
    struct DGraphicSet {
        vector<TVector3> points;
        Color_t color;
        poly_type type; // 0=markers, 1=lines
        double size;
        int marker_style;

        DGraphicSet(Color_t c, poly_type t, double s):color(c),type(t),size(s),marker_style(8){}
    };


    void Init() override; ///< Called once at program start
	void BeginRun(const std::shared_ptr<const JEvent>& locEvent) override;	///< Called everytime a new run number is detected.
	void Process(const std::shared_ptr<const JEvent>& locEvent) override;///< Called every event.

    const JEvent& GetCurrentEvent();
    void NextEvent();
    void SetRunContinuously(bool go);
    bool GetRunContinuously();


    const DMagneticFieldMap *Bfield = nullptr;

    vector<DGraphicSet> graphics;
    void FillGraphics(void);
    void UpdateTrackLabels(void);
    void UpdateBcalDisp(void);

    // Additional graphics that may be appropriate for only certain views
    vector<TObject*> graphics_xyA;
    vector<TObject*> graphics_xyB;
    vector<TObject*> graphics_xz;
    vector<TObject*> graphics_yz;
    vector<TObject*> graphics_tof_hits;

    void GetFactoryNames(vector<string> &facnames);
    unsigned int GetNrows(const string &factory, string tag);
    void GetDReferenceTrajectory(string dataname, string tag, unsigned int index, DReferenceTrajectoryHDV* &rt, vector<const DCDCTrackHit*> &cdchits);
    void GetAllWireHits(vector<pair<const DCoordinateSystem*,double> > &allhits);
    void FormatHistogram(TH2*, int);


 private:

    // Parameters

    int m_go = 0; // 1=continuously display events 0=wait for user
    bool m_skip_epics_events = true;

    // Members

    const JEvent *m_current_event = nullptr;
    std::mutex m_mutex;

    hdv_mainframe *hdvmf = nullptr;
    hdv_fulllistframe *fulllistmf=NULL;
    hdv_debugerframe *debugermf;
    DRootGeom *RootGeom;
    DGeometry *geom;
    string MATERIAL_MAP_MODEL;
    double RMAX_INTERIOR; // Used to allow user to extend drawing range of charged tracks
    double RMAX_EXTERIOR; // Used to allow user to extend drawing range of charged tracks
    double ZMAX;         // Used to allow user to extend drawing range of charged tracks

    uint32_t BCALVERBOSE;
    TCanvas *BCALHitCanvas;  
    TH2F *BCALHitMatrixU;
    TH2F *BCALHitMatrixD;
    TH2F *BCALParticles;
    vector <TText*> BCALPLables;
    TH2F *BCALPointZphiLayer[4];
    TH2F *BCALPointPhiTLayer[4];
    std::vector<TH2F*> BCALClusterZphiHistos;
    TLegend *LayerLegend;
    TLegend *ClusterLegend;

    map<string, double> photon_track_matching;
    double DELTA_R_FCAL;
  double GEMTRDz;

    void AddKinematicDataTrack(const DKinematicData* kd, int color, double size);
    void GetIntersectionWithCalorimeter(const DKinematicData* kd, DVector3 &pos, DetectorSystem_t &who);

};

extern EventViewer* gMYPROC;

#endif // _EVENTVIEWER_H_
