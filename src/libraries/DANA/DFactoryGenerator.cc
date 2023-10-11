// $Id$
//
//    File: DFactoryGenerator.cc
// Created: Mon Jul  3 21:46:40 EDT 2006
// Creator: davidl (on Darwin Harriet.local 8.6.0 powerpc)
// Modified:	Oct 3, 2012, Yi Qiang: add CERE
//				Oct 11 2012, Yi Qiang: add RICH
// Modified:    June 22, 2015, jrsteven: move RICH -> DIRC and remove CERE

#include "DFactoryGenerator.h"

extern void BCAL_init(JFactorySet *factory_set);
extern void CDC_init(JFactorySet *factory_set);
extern void FDC_init(JFactorySet *factory_set);
extern void FCAL_init(JFactorySet *factory_set);
extern void CCAL_init(JFactorySet *factory_set);
extern void RF_init(JFactorySet *factory_set);
extern void START_COUNTER_init(JFactorySet *factory_set);
extern void TAGGER_init(JFactorySet *factory_set);
extern void TOF_init(JFactorySet *factory_set);
extern void TRACKING_init(JFactorySet *factory_set);
extern void HDDM_init(JFactorySet *factory_set);
extern void PID_init(JFactorySet *factory_set);
extern void ANALYSIS_init(JFactorySet *factory_set);
extern void DAQ_init(JFactorySet *factory_set);
extern void TTAB_init(JFactorySet *factory_set);
extern void CERE_init(JFactorySet *factory_set);
extern void DIRC_init(JFactorySet *factory_set);
extern void TRIGGER_init(JFactorySet *factory_set);
extern void PAIR_SPECTROMETER_init(JFactorySet *factory_set);
extern void TPOL_init(JFactorySet *factory_set);
extern void FMWPC_init(JFactorySet *factory_set);
extern void EVENTSTORE_init(JFactorySet *factory_set);
extern void TAC_init(JFactorySet *factory_set);
extern void TRD_init(JFactorySet *factory_set);

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
void DFactoryGenerator::GenerateFactories(JFactorySet *factory_set)
{
	BCAL_init(factory_set);
	CDC_init(factory_set);
	FDC_init(factory_set);
	FCAL_init(factory_set);
	ECAL_init(actory_set);
	CCAL_init(factory_set);
	RF_init(factory_set);
	START_COUNTER_init(factory_set);
	TAGGER_init(factory_set);
	TOF_init(factory_set);
	TRACKING_init(factory_set);
	HDDM_init(factory_set);
	PID_init(factory_set);
	ANALYSIS_init(factory_set);
	DAQ_init(factory_set);
	TTAB_init(factory_set);
	CERE_init(factory_set);
	DIRC_init(factory_set);
	TRIGGER_init(factory_set);
	PAIR_SPECTROMETER_init(factory_set);
	TPOL_init(factory_set);
	FMWPC_init(factory_set);
	EVENTSTORE_init(factory_set);
	TAC_init(factory_set);
	TRD_init(factory_set);	
}
