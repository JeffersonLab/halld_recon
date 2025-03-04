#ifndef _Df250EmulatorAlgorithm_v3_
#define _Df250EmulatorAlgorithm_v3_
#include <JANA/JEvent.h>

#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;

#include <DAQ/Df250EmulatorAlgorithm.h>

/////////////////////////////////////////////////////////////////
// This implements the base class for the f250 firmware emulation
// EmulateFirmware needs to be virtually overwritten by the user
////////////////////////////////////////////////////////////////

class Df250EmulatorAlgorithm_v3:public Df250EmulatorAlgorithm{
    public:

        Df250EmulatorAlgorithm_v3(JApplication* app);
        ~Df250EmulatorAlgorithm_v3(){};

        //Only the emulation routines need to be overwritten
        void EmulateFirmware(const Df250WindowRawData* rawData,
                             std::vector<Df250PulseData*> &pdatt_objs);

        void EmulateFirmware(const Df250WindowRawData* wrd,
                             std::vector<Df250PulseTime*> &pt_objs,
                             std::vector<Df250PulsePedestal*> &pp_objs,
                             std::vector<Df250PulseIntegral*> &pi_objs) {
            throw JException("Invalid data format being called for Df250EmulatorAlgorithm_v3!");
        }


    protected:
        Df250EmulatorAlgorithm_v3(){};
        // Enables forced use of default values
        int FORCE_DEFAULT;
        int USE_CRATE_DEFAULTS;
        // Default values for the essential parameters
        uint32_t NSA_DEF; 
        uint32_t NSB_DEF; 
        uint16_t THR_DEF;
        uint32_t NPED_DEF;
        uint32_t MAXPED_DEF;
        uint16_t NSAT_DEF;
        // Set verbosity
        int VERBOSE;

};

#endif // _Df250EmulatorAlgorithm_v3_
