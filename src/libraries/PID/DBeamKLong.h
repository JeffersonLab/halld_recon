// $Id$
//

#ifndef _DBeamKLong_
#define _DBeamKLong_

#include <PID/DKinematicData.h>
#include <PID/DBeamPhoton.h>

class DBeamKLong: public DBeamPhoton
{
	public:
		JOBJECT_PUBLIC(DBeamKLong);

		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "p(GeV)", "%f", momentum().Mag());
			AddString(items, "t(ns)", "%3.1f", time());
		}
};



#endif // _DBeamKLong_

