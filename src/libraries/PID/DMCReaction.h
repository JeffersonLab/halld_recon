// $Id$
//
//    File: DMCReaction.h
// Created: Sun Aug 28 18:41:08 EDT 2011
// Creator: davidl (on Darwin Amelia.local 9.8.0 i386)
//

#ifndef _DMCReaction_
#define _DMCReaction_

#include <JANA/JObject.h>

#include <PID/DKinematicData.h>

class DMCReaction:public JObject{
	public:
		DMCReaction(){}
		virtual ~DMCReaction(){}
		JOBJECT_PUBLIC(DMCReaction);
		
		int type;
		double weight;
		DKinematicData target;
		DKinematicData beam;
		
		void Summarize(JObjectSummary& summary) const override {
			summary.add(type, "type", "%2d");
			summary.add(weight, "weight", "%3.1f");
			summary.add(target.mass(), "mass target(GeV)", "%3.1f");
			summary.add(beam.energy(), "energy beam(GeV/c^2)", "%f");
		}

	protected:
	
	
	private:

};

#endif // _DMCReaction_

