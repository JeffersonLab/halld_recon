// $Id$
//
//    File: JEventProcessor_TAGM_clusters.cc
// Created: Tue Jul  5 21:19:22 EDT 2016
// Creator: barnes (on Linux gluey.phys.uconn.edu 2.6.32-573.22.1.el6.x86_64 x86_64)
//

#include "JEventProcessor_TAGM_clusters.h"

#include <TH1.h>
#include <TH2.h>
#include <TDirectory.h>

#include <TAGGER/DTAGMGeometry.h>
#include <TAGGER/DTAGMHit.h>

using std::vector;
using std::set;

const int NCOLUMNS = DTAGMGeometry::kColumnCount;

static TH1D *h_occupancy_b;	// occupancy before merging
static TH1D *h_occupancy_a;	// occupancy after merging
static TH1D *h_occupancy_ind;	// occupancy of ind. channels
static TH1I *h_deltaT[10];	// delta T of neighbors before merging
static TH1I *h_mult_b;		// multiplicity before merging
static TH1I *h_mult_a;		// multiplicity after merging
static TH1I *h_E_b;		// energy before merging
static TH1I *h_E_a;		// energy after merging

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactoryT.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->Add(new JEventProcessor_TAGM_clusters());
}
} // "C"


//------------------
// JEventProcessor_TAGM_clusters (Constructor)
//------------------
JEventProcessor_TAGM_clusters::JEventProcessor_TAGM_clusters()
{
	SetTypeName("JEventProcessor_TAGM_clusters");
}

//------------------
// ~JEventProcessor_TAGM_clusters (Destructor)
//------------------
JEventProcessor_TAGM_clusters::~JEventProcessor_TAGM_clusters()
{

}

//------------------
// Init
//------------------
void JEventProcessor_TAGM_clusters::Init()
{
    // This is called once at program startup.
    auto app = GetApplication();
    lockService = app->GetService<JLockService>();

   TDirectory *mainDir = gDirectory;
   TDirectory *tagmDir = gDirectory->mkdir("TAGM_clusters");
   tagmDir->cd();
   // Before
   gDirectory->mkdir("Before")->cd();
   h_occupancy_b = new TH1D("occupancy_b","Histogram of occupancy",NCOLUMNS,1,NCOLUMNS+1);
   h_mult_b = new TH1I("mult_b","Multiplicity before",40,0.,40.);
   h_E_b = new TH1I("E_b","Energy before merging",110,8.2,9.3);
   for (uint32_t i = 0; i < 10; ++i)
   {
      h_deltaT[i] = new TH1I(Form("deltaT_%i",i+1),
                             Form("#Deltat of neighboring hits, group %i",i+1),
                             100,-10.,10.);
   }

   tagmDir->cd();

   // After
   gDirectory->mkdir("After")->cd();
   h_occupancy_a = new TH1D("occupancy_a","Histogram of occupancy",NCOLUMNS,1,NCOLUMNS+1);
   h_mult_a = new TH1I("mult_a","Multiplicity after",40,0.,40.);
   h_E_a = new TH1I("E_a","Energy after merging",110,8.2,9.3);

   tagmDir->cd();

   h_occupancy_ind = new TH1D("occupancy_ind","Histogram of occupancy",20,1,21);
   // back to main dir
   mainDir->cd();
}

//------------------
// BeginRun
//------------------
void JEventProcessor_TAGM_clusters::BeginRun(const std::shared_ptr<const JEvent>& event)
{
	// This is called whenever the run number changes
}

//------------------
// Process
//------------------
void JEventProcessor_TAGM_clusters::Process(const std::shared_ptr<const JEvent>& event)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// event->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// event->Get(mydataclasses);
	//
	// lockService->RootWriteLock();
	//  ... fill historgrams or trees ...
	// lockService->RootUnLock();

   vector<const DTAGMHit*>	tagm_hits;
   event->Get(tagm_hits, "Calib");
   vector<const DTAGMHit*>	tagm_merged_hits;
   event->Get(tagm_merged_hits);

   lockService->RootWriteLock();

   // Get occupancies and multiplicities before merging
   int mult_b = 0;
   for (uint32_t i = 0; i < tagm_hits.size(); ++i)
   {
      if (!tagm_hits[i]->has_fADC) continue;
      if (tagm_hits[i]->row > 0) continue;

      h_occupancy_b->Fill(tagm_hits[i]->column);
      h_E_b->Fill(tagm_hits[i]->E);
      mult_b++;
   }
   h_mult_b->Fill(mult_b);

   // Get occupancies and multiplicities after merging
   int mult_a = 0;
   for (uint32_t i = 0; i < tagm_merged_hits.size(); ++i)
   {
      if (!tagm_merged_hits[i]->has_fADC) continue;
      if (tagm_merged_hits[i]->row > 0) 
      {
         int col = tagm_merged_hits[i]->column;
         int row = tagm_merged_hits[i]->row;
         if (col == 9)
            h_occupancy_ind->Fill(row);
         else if (col == 27)
            h_occupancy_ind->Fill(5+row);
         else if (col == 81)
            h_occupancy_ind->Fill(10+row);
         else if (col == 99)
            h_occupancy_ind->Fill(15+row);
         continue;
      }

      h_occupancy_a->Fill(tagm_merged_hits[i]->column);
      h_E_a->Fill(tagm_merged_hits[i]->E);
      mult_a++;
   }
   h_mult_a->Fill(mult_a);

   // Check time differences between pre-merge neighbors
   set<uint32_t> locColUsedSoFar;
   for (uint32_t i = 0; i < tagm_hits.size(); ++i)
   {
      if (!tagm_hits[i]->has_fADC) continue;
      if (tagm_hits[i]->row > 0) continue;

      // check if column has been paired
      if (locColUsedSoFar.find(tagm_hits[i]->column) != locColUsedSoFar.end()) continue;

      for (uint32_t j = i+1; j < tagm_hits.size(); ++j)
      {
         if (!tagm_hits[j]->has_fADC) continue;
         if (tagm_hits[j]->row > 0) continue;

         int diff = tagm_hits[i]->column - tagm_hits[j]->column;
         double deltaT = tagm_hits[i]->t - tagm_hits[j]->t;
         if (fabs(diff) == 1)
         {
            if (tagm_hits[i]->column < 11)
               h_deltaT[0]->Fill(deltaT);
            else if (tagm_hits[i]->column < 21)
               h_deltaT[1]->Fill(deltaT);
            else if (tagm_hits[i]->column < 31)
               h_deltaT[2]->Fill(deltaT);
            else if (tagm_hits[i]->column < 41)
               h_deltaT[3]->Fill(deltaT);
            else if (tagm_hits[i]->column < 51)
               h_deltaT[4]->Fill(deltaT);
            else if (tagm_hits[i]->column < 61)
               h_deltaT[5]->Fill(deltaT);
            else if (tagm_hits[i]->column < 71)
               h_deltaT[6]->Fill(deltaT);
            else if (tagm_hits[i]->column < 81)
               h_deltaT[7]->Fill(deltaT);
            else if (tagm_hits[i]->column < 91)
               h_deltaT[8]->Fill(deltaT);
            else
               h_deltaT[9]->Fill(deltaT);
         }
         if (fabs(diff) == 1 && fabs(deltaT) <= 5)
         {
            locColUsedSoFar.insert(tagm_hits[j]->column);
            break;
         }
      }
   }

   lockService->RootUnLock();
}

//------------------
// EndRun
//------------------
void JEventProcessor_TAGM_clusters::EndRun()
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
}

//------------------
// Finish
//------------------
void JEventProcessor_TAGM_clusters::Finish()
{
	// Called before program exit after event processing is finished.
}

