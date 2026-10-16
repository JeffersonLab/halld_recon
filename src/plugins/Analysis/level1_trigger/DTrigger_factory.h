// $Id$
//
//    File: DTrigger_factory.h
// Created: Tue Jun  7 10:15:05 EDT 2011
// Creator: davidl (on Darwin eleanor.jlab.org 10.7.0 i386)
//

#ifndef _DTrigger_factory_
#define _DTrigger_factory_

#include <JANA/JFactoryT.h>
#include "DTrigger.h"

/// Implements a L1 trigger algorithm on simulated data in the form of
/// a flag in the DTrigger object. The flag will indicate whether the
/// level 1 trigger would have fired for the event based on the hit
/// objects. This can be used by analysis programs to decide whether
/// to process or ignore the event.
///
/// Much of this is based on the information GlueX-doc-1043. What is
/// currently implemented are two algorithms described on page 13.
/// both require BCAL + 4*FCAL >= 2.0GeV. Additional requirements are:
///
/// L1afired: BCAL > 200 MeV  &&  FCAL > 30 MeV
///
/// L1bfired: BCAL > 30 MeV  &&  FCAL > 30 MeV  &&  NSC > 0
///
/// The values of BCAL and FCAL and NSC used to make the decision
/// are kept in the DTrigger object at Ebcal, Efcal, and Nschits
/// respectively.

class DTrigger_factory:public JFactoryT<DTrigger>{
	public:
		DTrigger_factory(){};
		~DTrigger_factory(){};


	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		bool REQUIRE_START_COUNTER;
		double unattenuate_to_center;
};

#endif // _DTrigger_factory_

