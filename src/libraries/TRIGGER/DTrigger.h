#ifndef _DTrigger_
#define _DTrigger_

#include <JANA/JObject.h>

class DTrigger : public JObject
{
	public:
		JOBJECT_PUBLIC(DTrigger);

		//GETTERS
		uint32_t Get_L1TriggerBits(void) const{return dL1TriggerBits;}
		uint32_t Get_L1FrontPanelTriggerBits(void) const{return dL1FrontPanelTriggerBits;}
		bool Get_IsPhysicsEvent(void) const;

		float Get_GTP_BCALEnergy(void) const {return dGTP_BCALEnergy;};
		float Get_GTP_FCALEnergy(void) const {return dGTP_FCALEnergy;};

		//SETTERS
		void Set_L1TriggerBits(uint32_t locL1TriggerBits){dL1TriggerBits = locL1TriggerBits;}
		void Set_L1FrontPanelTriggerBits(uint32_t locL1FrontPanelTriggerBits){dL1FrontPanelTriggerBits = locL1FrontPanelTriggerBits;}

		void Set_GTP_BCALEnergy(float locGTP_BCALEnergy) { dGTP_BCALEnergy = locGTP_BCALEnergy; }
		void Set_GTP_FCALEnergy(float locGTP_FCALEnergy) { dGTP_FCALEnergy = locGTP_FCALEnergy; }
		void Set_GTP_SCHits(float locGTP_SCHits) { dGTP_SCHits = locGTP_SCHits; }

		void Summarize(JObjectSummary& summary) const override {
			summary.add(dL1TriggerBits, "dL1TriggerBits", "%ld");
			summary.add(dL1FrontPanelTriggerBits, "dL1FrontPanelTriggerBits", "%ld");
		}

	private:
		//NOTE: If is EPICS/SYNC/etc. event, both L1 values will be 0
		uint32_t dL1TriggerBits = 0;
		uint32_t dL1FrontPanelTriggerBits = 0;
		
		// Inputs to trigger logic
		float dGTP_BCALEnergy = 0.;
		float dGTP_FCALEnergy = 0.;
		float dGTP_SCHits = 0.;

};

inline bool DTrigger::Get_IsPhysicsEvent(void) const
{
	//Both L1 = 0: EPICS/SYNC/etc. //L1 = 8: PS
	return ((dL1FrontPanelTriggerBits == 0) && (dL1TriggerBits != 0) && (dL1TriggerBits != 8));
}

#endif // _DTrigger_
