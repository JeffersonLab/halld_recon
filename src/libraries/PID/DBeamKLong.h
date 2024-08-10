// $Id$
//

#ifndef _DBeamKLong_
#define _DBeamKLong_

#include <PID/DKinematicData.h>

class DBeamKLong: public DKinematicData
{
	public:
		JOBJECT_PUBLIC(DBeamKLong);
		
		// int status;  // will need something here at some point to distinguish different algorithm results

		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "p(GeV)", "%f", momentum().Mag());
			AddString(items, "t(ns)", "%3.1f", time());
		}
};



#endif // _DBeamKLong_

