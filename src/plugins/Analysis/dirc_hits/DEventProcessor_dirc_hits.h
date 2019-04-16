// -----------------------------------------
// DEventProcessor_dirc_hits.h
// created on: 07.04.2017
// initial athor: r.dzhygadlo at gsi.de
// -----------------------------------------

#ifndef DEVENTPROCESSOR_dirc_hits_H_
#define DEVENTPROCESSOR_dirc_hits_H_

#include <JANA/JEventProcessor.h>
#include <JANA/JApplication.h>
#include <DANA/DApplication.h>
#include <HDGEOMETRY/DGeometry.h>

#include <ANALYSIS/DEventWriterROOT.h>
#include <HDDM/DEventWriterREST.h>
#include <ANALYSIS/DHistogramActions.h>

#include "DFactoryGenerator_dirc_hits.h"
#include "DIRC/DDIRCGeometry.h"
#include "DrcHit.h"
#include "DrcEvent.h"

#include <TVector3.h>
#include <TROOT.h>
#include <TClonesArray.h>
#include "ANALYSIS/DAnalysisUtilities.h"

using namespace jana;
using namespace std;


class DEventProcessor_dirc_hits: public JEventProcessor {

    public:
        DEventProcessor_dirc_hits();
        ~DEventProcessor_dirc_hits();


        const char* className(void) {
            return "DEventProcessor_dirc_hits";
        }

        const DParticleID* dParticleID;
        const DDIRCGeometry* dDIRCGeometry;


        TClonesArray *fcEvent;
        DrcEvent *fEvent;
        DrcHit *fHit;
        TTree *fTree;

        pthread_mutex_t mutex;


    private:
        jerror_t init(void);
        jerror_t brun(jana::JEventLoop *loop, int32_t runnumber);
        jerror_t evnt(JEventLoop *loop, uint64_t eventnumber);
        jerror_t erun(void);
        jerror_t fini(void); // called after last event

        const DAnalysisUtilities* dAnalysisUtilities;


};

#endif /* DEVENTPROCESSOR_dirc_hits_H_ */

