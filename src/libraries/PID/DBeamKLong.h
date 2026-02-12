// $Id$
//

#ifndef _DBeamKLong_
#define _DBeamKLong_

#include <PID/DKinematicData.h>

class DBeamKLong : public DKinematicData
{
	public:
		JOBJECT_PUBLIC(DBeamKLong);
		
		// int status;  // will need something here at some point to distinguish different algorithm results

		void Summarize(JObjectSummary& summary) const override {
			summary.add(momentum().Mag(), "p(GeV)", "%f");
			summary.add(time(), "t(ns)", "%3.1f");
		}

};



#endif // _DBeamKLong_

