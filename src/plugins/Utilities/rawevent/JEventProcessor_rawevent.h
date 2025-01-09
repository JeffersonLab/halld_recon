// $Id$
//
//    File: JEventProcessor_rawevent.h
// Created: Fri Jun 24 12:05:19 EDT 2011
// Creator: wolin (on Linux stan.jlab.org 2.6.18-194.11.1.el5 x86_64)
//

#ifndef _JEventProcessor_rawevent_
#define _JEventProcessor_rawevent_


// temporary root stuff
#include <TH1.h>


#include <vector>
#include <map>


#include <JANA/JEventProcessor.h>

#ifdef HAVE_EVIO
#include <evioFileChannel.hxx>
#include <evioUtil.hxx>
#endif //HAVE_EVIO

#include <BCAL/DBCALHit.h>
#include <BCAL/DBCALTDCHit.h>
#include <CDC/DCDCHit.h>
#include <FCAL/DFCALHit.h>
#include <FDC/DFDCHit.h>
#include <START_COUNTER/DSCHit.h>
#include <TOF/DTOFHit.h>
#include <TAGGER/DTAGMHit.h>
#include <TAGGER/DTAGHHit.h>
#include <PAIR_SPECTROMETER/DPSHit.h>
#include <PAIR_SPECTROMETER/DPSCHit.h>

using namespace std;
#ifdef HAVE_EVIO
using namespace evio;


// holds crate/slot/channel
typedef struct {
  int crate;
  int slot;
  int channel;
} cscVal;
typedef const cscVal &cscRef;

#endif //HAVE_EVIO


//----------------------------------------------------------------------------


class JEventProcessor_rawevent : public JEventProcessor {

	public:
		JEventProcessor_rawevent();
		~JEventProcessor_rawevent();


	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;


#ifdef HAVE_EVIO

                // these routines read and fill the translation tables
                static void readTranslationTable(void);
                static void StartElement(void *userData, const char *xmlname, const char **atts);
                static void EndElement(void *userData, const char *xmlname);


                // these routines access the translation tables
                cscRef DTOFHitTranslationADC(const DTOFHit* hit) const;
                cscRef DTOFHitTranslationTDC(const DTOFHit* hit) const;

                cscRef DBCALHitTranslationADC(const DBCALHit* hit) const;
                //cscRef DBCALHitTranslationTDC(const DBCALHit* hit) const;
                cscRef DBCALHitTranslationTDC(const DBCALTDCHit* hit) const;

                cscRef DFCALHitTranslationADC(const DFCALHit* hit) const;

                cscRef DFDCAnodeHitTranslation(const DFDCHit* hit) const;
                cscRef DFDCCathodeHitTranslation(const DFDCHit* hit) const;

                cscRef DCDCHitTranslationADC(const DCDCHit* hit) const;

                cscRef DSTHitTranslationADC(const DSCHit* hit) const;
                cscRef DSTHitTranslationTDC(const DSCHit* hit) const;

                cscRef DTAGMHitTranslationADC(const DTAGMHit* hit) const;
                cscRef DTAGMHitTranslationTDC(const DTAGMHit* hit) const;

                cscRef DTAGHHitTranslationADC(const DTAGHHit* hit) const;
                cscRef DTAGHHitTranslationTDC(const DTAGHHit* hit) const;

                cscRef DPSHitTranslationADC(const DPSHit* hit) const;

                cscRef DPSCHitTranslationADC(const DPSCHit* hit) const;
                cscRef DPSCHitTranslationTDC(const DPSCHit* hit) const;
#endif //HAVE_EVIO
};

#endif // _JEventProcessor_rawevent_


//----------------------------------------------------------------------------
