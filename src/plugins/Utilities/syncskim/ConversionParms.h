
#include <TObject.h>

// This is used to hold the 2 parameters used
// to convert the timestamp to unix time. In
// the original implementation, these values
// were only printed to the screen and a script
// captured them via std.out. The plugin has
// now been upgraded for use in the offline
// monitoring as well. Thus, we save the values
// in the ROOT file too. 
//
// This is done using a TTree instead of a simple
// histogram so when the offmon job merges the
// root files for all files in the run, the 
// individual values are still available. The
// first and last event numbers are recorded
// here so we know what range of events each set
// of values corresponds to in case the merging
// of the trees does not preserve order.

class ConversionParms:public TObject
{
	public:
		UInt_t    run_number;
		ULong64_t first_event_number;
		ULong64_t last_event_number;
		ULong64_t tics_per_sec;
		ULong64_t unix_start_time;
		
		ClassDef(ConversionParms ,1)
};
