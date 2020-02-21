#ifndef _DTPOLHit_
#define _DTPOLHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DTPOLHit:public jana::JObject{
public:
    JOBJECT_PUBLIC(DTPOLHit);

    int sector;    // sector number 1-32
    double phi;
    int ring;      // ring number 1-24
    double theta;
    unsigned int nsamples;
    unsigned int w_samp1;
    unsigned int w_min;
    unsigned int w_max;
    unsigned int integral; 
    double pulse_peak;     // Energy loss in keV
    double dE_proxy;
    double t_proxy;
    double dE;
    double t;

    void toStrings(vector<pair<string,string> > &items)const{
        AddString(items, "sector", "%d", sector);
        AddString(items, "phi", "%3.3f", phi);
        AddString(items, "ring", "%d", ring);
        AddString(items, "theta", "%3.3f", theta);
        AddString(items, "nsamples", "%d", nsamples);
        AddString(items, "w_samp1", "%d", w_samp1);
        AddString(items, "w_min", "%d", w_min);
        AddString(items, "w_max", "%d", w_max);
	AddString(items, "integral", "%d", integral);
	AddString(items, "pulse_peak", "%3.3f", pulse_peak);
	AddString(items, "dE_proxy", "%3.3f", dE_proxy);
        AddString(items, "t_proxy", "%3.3f", t_proxy);
        AddString(items, "dE", "%3.3f", dE);
        AddString(items, "t", "%3.3f", t);
    }
};

#endif // _DTPOLHit_
