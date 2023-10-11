// $Id$
//
//    File: DDetectorMatches_factory_Combo.h
// Created: Tue Aug  9 14:29:24 EST 2011
// Creator: pmatt (on Linux ifarml6 2.6.18-128.el5 x86_64)
//

#ifndef _DDetectorMatches_factory_Combo_
#define _DDetectorMatches_factory_Combo_

#include <iostream>
#include <iomanip>

#include <JANA/JFactoryT.h>
#include <PID/DDetectorMatches_factory.h>

#include <PID/DDetectorMatches.h>
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DParticleID.h>
#include <TOF/DTOFPoint.h>
#include <BCAL/DBCALShower.h>
#include <FCAL/DFCALShower.h>

using namespace std;

class DDetectorMatches_factory_Combo : public JFactoryT<DDetectorMatches>
{
	public:
		DDetectorMatches_factory_Combo(){
			SetTag("Combo");
		}
		~DDetectorMatches_factory_Combo(){};

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		pair<double, double> Calc_EnergyRatio(const DTrackTimeBased* locTrackTimeBased, const DTrackTimeBased* locOriginalTrackTimeBased) const;
		double Calc_PVariance(const DTrackTimeBased* locTrack) const;

		DDetectorMatches_factory* dDetectorMatchesFactory;
};

#endif // _DDetectorMatches_factory_Combo_

