#ifndef _DBCALTruthShower_
#define _DBCALTruthShower_

#include <JANA/JObject.h>

class DBCALTruthShower: public JObject {
	public:
		JOBJECT_PUBLIC(DBCALTruthShower);

		int track; ///< This is the unique number that GEANT has assigned the particle
		int itrack; ///< This is the index within the MCThrown structure of this track
		int ptype; ///< This is the particle ID number
		int primary;
		float phi;
		float r;
		float z;
		float t;
		float E;
		float px;
		float py;
		float pz;

		void Summarize(JObjectSummary& summary) const override {
			summary.add(ptype, "ptype", "%d");
			summary.add(track, "track", "%d");
			summary.add(itrack, "itrack", "%d");
			summary.add(primary, "primary", "%d");
			summary.add(phi, "phi", "%1.3f");
			summary.add(r, "r", "%4.3f");
			summary.add(z, "z", "%4.1f");
			summary.add(t, "t", "%4.3f");
			summary.add(sqrt(px*px + py*py + pz*pz), "p", "%4.3f");
			summary.add(E, "E", "%4.3f");
		}
};

#endif // _DBCALTruthShower_

