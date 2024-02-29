// $Id$
//
//    File: DFactoryGenerator.cc
// Created: Mon Jul  3 21:46:40 EDT 2006
// Creator: davidl (on Darwin Harriet.local 8.6.0 powerpc)
// Modified:	Oct 3, 2012, Yi Qiang: add CERE
//				Oct 11 2012, Yi Qiang: add RICH
// Modified:    June 22, 2015, jrsteven: move RICH -> DIRC and remove CERE

#include "DFactoryGenerator.h"

extern jerror_t BCAL_init(JEventLoop *loop);
extern jerror_t CDC_init(JEventLoop *loop);
extern jerror_t FDC_init(JEventLoop *loop);
extern jerror_t FCAL_init(JEventLoop *loop);
extern jerror_t ECAL_init(JEventLoop *loop);
extern jerror_t CCAL_init(JEventLoop *loop);
extern jerror_t RF_init(JEventLoop *loop);
extern jerror_t START_COUNTER_init(JEventLoop *loop);
extern jerror_t TAGGER_init(JEventLoop *loop);
extern jerror_t TOF_init(JEventLoop *loop);
extern jerror_t TRACKING_init(JEventLoop *loop);
extern jerror_t HDDM_init(JEventLoop *loop);
extern jerror_t PID_init(JEventLoop *loop);
extern jerror_t ANALYSIS_init(JEventLoop *loop);
extern jerror_t DAQ_init(JEventLoop *loop);
extern jerror_t TTAB_init(JEventLoop *loop);
extern jerror_t CERE_init(JEventLoop *loop);
extern jerror_t DIRC_init(JEventLoop *loop);
extern jerror_t TRIGGER_init(JEventLoop *loop);
extern jerror_t PAIR_SPECTROMETER_init(JEventLoop *loop);
extern jerror_t TPOL_init(JEventLoop *loop);
extern jerror_t FMWPC_init(JEventLoop *loop);
extern jerror_t EVENTSTORE_init(JEventLoop *loop);
extern jerror_t TAC_init(JEventLoop *loop);
extern jerror_t TRD_init(JEventLoop *loop);

//---------------------------------
// DFactoryGenerator    (Constructor)
//---------------------------------
DFactoryGenerator::DFactoryGenerator()
{

}

//---------------------------------
// ~DFactoryGenerator    (Destructor)
//---------------------------------
DFactoryGenerator::~DFactoryGenerator()
{

}

//---------------------------------
// GenerateFactories
//---------------------------------
jerror_t DFactoryGenerator::GenerateFactories(JEventLoop *loop)
{
	BCAL_init(loop);
	CDC_init(loop);
	FDC_init(loop);
	FCAL_init(loop);
	ECAL_init(loop);
	CCAL_init(loop);
	RF_init(loop);
	START_COUNTER_init(loop);
	TAGGER_init(loop);
	TOF_init(loop);
	TRACKING_init(loop);
	HDDM_init(loop);
	PID_init(loop);
	ANALYSIS_init(loop);
	DAQ_init(loop);
	TTAB_init(loop);
	CERE_init(loop);
	DIRC_init(loop);
	TRIGGER_init(loop);
	PAIR_SPECTROMETER_init(loop);
	TPOL_init(loop);
	FMWPC_init(loop);
	EVENTSTORE_init(loop);
	TAC_init(loop);
	TRD_init(loop);	

	return NOERROR;
}
