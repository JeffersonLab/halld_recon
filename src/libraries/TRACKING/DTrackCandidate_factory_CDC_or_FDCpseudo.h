// $Id$
//
//    File: DTrackCandidate_factory_CDC_or_FDCpseudo.h
// Created: Thu Apr 16 09:14:49 EDT 2009
// Creator: davidl (on Darwin harriet.jlab.org 9.6.0 i386)
//

#ifndef _DTrackCandidate_factory_CDC_or_FDCpseudo_
#define _DTrackCandidate_factory_CDC_or_FDCpseudo_

#include <JANA/JFactoryT.h>
#include "DTrackCandidate.h"

/// \htmlonly
/// <A href="index.html#legend">
///	<IMG src="ND.png" width="100">
///	</A>
/// \endhtmlonly

/// This class is not used as part of the baseline reconstruction. It is 
/// mainly for debugging purposes.
///
/// This will essentially make a single list of DTrackCandidate objects by
/// merging the CDC and FDCPseudo candidates. There is a weak attempt to filter
/// clones of candidates found in both lists, but no merging is done, one of
/// the clones is simply dropped.
///
/// Note that this merges candidates from the DTrackCandidate_factory_FDCpseudo  
/// class which is also not the default FDC track finder (see DTrackCandidate_factory_FDCCathodes
/// for that).

class DTrackCandidate_factory_CDC_or_FDCpseudo:public JFactoryT<DTrackCandidate>{
	public:
		DTrackCandidate_factory_CDC_or_FDCpseudo(){
			SetTag("CDC_or_FDCpseudo");
		};
		~DTrackCandidate_factory_CDC_or_FDCpseudo(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;
		
		int DEBUG_LEVEL;
};

#endif // _DTrackCandidate_factory_CDC_or_FDCpseudo_

