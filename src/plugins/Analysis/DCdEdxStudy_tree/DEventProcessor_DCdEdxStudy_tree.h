#ifndef _DEventProcessor_DCdEdxStudy_tree_
#define _DEventProcessor_DCdEdxStudy_tree_

#include <JANA/JEventProcessor.h>

#include <TFile.h>
#include <TTree.h>
#include <DVector3.h>
#include <particleType.h>

#include <TRACKING/DMCThrown.h>
#include <TRACKING/DTrackTimeBased.h>
#include <PID/DParticleID.h>
#include <DCdEdxInformation.h>

class DEventProcessor_DCdEdxStudy_tree:public JEventProcessor{
	public:
		DEventProcessor_DCdEdxStudy_tree(){};
		~DEventProcessor_DCdEdxStudy_tree(){};
		SetTypeName("DEventProcessor_DCdEdxStudy_tree");

	private:
		void Init() override;
		void BeginRun(const std::shared_ptr<const JEvent>& event) override;
		void Process(const std::shared_ptr<const JEvent>& event) override;
		void EndRun() override;
		void Finish() override;

		DCdEdxInformation *dDCdEdxInformation;
		TTree* dPluginTree_DCdEdxInformation;

};

#endif // _DEventProcessor_DCdEdxStudy_tree_

