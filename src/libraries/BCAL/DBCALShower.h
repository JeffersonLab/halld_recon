#ifndef _DBCALShower_
#define _DBCALShower_

#include <JANA/JObject.h>
#include <math.h>
#include <DMatrix.h>
#include <TMatrixFSym.h>

#include "DANA/DObjectID.h"

class DBCALShower: public JObject{
	public:
		JOBJECT_PUBLIC(DBCALShower);

 DBCALShower():ExyztCovariance(5) {} ///< Constructor initializes matrix to 5x5

    oid_t id = reinterpret_cast<oid_t>(this);
    float E;
    float E_raw;
    float E_preshower;
    float E_L2;
    float E_L3;
    float E_L4;
    float x;
    float y;
    float z;
    float t;
    float sigLong;
    float sigTrans;
    float sigTheta;
//    float sigTime;
    float rmsTime;
    int N_cell;
    int Q;
	TMatrixFSym ExyztCovariance;

    float const EErr() const { return sqrt(ExyztCovariance(0,0)); }
    float const xErr() const { return sqrt(ExyztCovariance(1,1)); }
    float const yErr() const { return sqrt(ExyztCovariance(2,2)); }
    float const zErr() const { return sqrt(ExyztCovariance(3,3)); }
	float const tErr() const { return sqrt(ExyztCovariance(4,4)); }
	float const XYcorr() const {
		if (xErr()>0 && yErr()>0) return ExyztCovariance(1,2)/xErr()/yErr();
		else return 0;
	}
	float const XZcorr() const {
		if (xErr()>0 && zErr()>0) return ExyztCovariance(1,3)/xErr()/zErr();
		else return 0;
	}
	float const YZcorr() const {
		if (yErr()>0 && zErr()>0) return ExyztCovariance(2,3)/yErr()/zErr();
		else return 0;
	}
	float const EXcorr() const {
		if (EErr()>0 && xErr()>0) return ExyztCovariance(0,1)/EErr()/xErr();
		else return 0;
	}
	float const EYcorr() const {
		if (EErr()>0 && yErr()>0) return ExyztCovariance(0,2)/EErr()/yErr();
		else return 0;
	}
	float const EZcorr() const {
		if (EErr()>0 && zErr()>0) return ExyztCovariance(0,3)/EErr()/zErr();
		else return 0;
	}
	float const XTcorr() const {
		if (xErr()>0 && tErr()>0) return ExyztCovariance(1,4)/xErr()/tErr();
		else return 0;
	}
	float const YTcorr() const {
		if (yErr()>0 && tErr()>0) return ExyztCovariance(2,4)/yErr()/tErr();
		else return 0;
	}
	float const ZTcorr() const {
		if (zErr()>0 && tErr()>0) return ExyztCovariance(3,4)/zErr()/tErr();
		else return 0;
	}
	float const ETcorr() const {
		if (EErr()>0 && tErr()>0) return ExyztCovariance(0,4)/EErr()/tErr();
		else return 0;
	}

	void Summarize(JObjectSummary& summary) const override {
			summary.add(E, "E", "%5.3f");
			summary.add(x, "x", "%5.2f");
			summary.add(y, "y", "%5.2f");
			summary.add(z, "z", "%5.1f");
			summary.add(t, "t", "%5.1f");
			summary.add(sqrt(x*x+y*y), "r", "%5.1f");
			summary.add(atan2(y,x), "phi", "%5.3f");
			summary.add(E_preshower, "E_preshower", "%5.3f");
			summary.add(E_L2, "E_L2", "%5.3f");
			summary.add(E_L3, "E_L3", "%5.3f");
			summary.add(E_L4, "E_L4", "%5.3f");
			summary.add(N_cell, "N_cell", "%d");
			summary.add(Q, "Q", "%d");
			summary.add(EErr(), "dE", "%5.3f");
			summary.add(xErr(), "dx", "%5.3f");
			summary.add(yErr(), "dy", "%5.3f");
			summary.add(zErr(), "dz", "%5.2f");
			summary.add(tErr(), "dt", "%5.3f");
			summary.add(EXcorr(), "EXcorr", "%5.3f");
			summary.add(EYcorr(), "EYcorr", "%5.3f");
			summary.add(EZcorr(), "EZcorr", "%5.3f");
			summary.add(ETcorr(), "ETcorr", "%5.3f");
			summary.add(XYcorr(), "XYcorr", "%5.3f");
			summary.add(XZcorr(), "XZcorr", "%5.3f");
			summary.add(XTcorr(), "XTcorr", "%5.3f");
			summary.add(YZcorr(), "YZcorr", "%5.3f");
			summary.add(YTcorr(), "YTcorr", "%5.3f");
			summary.add(ZTcorr(), "ZTcorr", "%5.3f");
			summary.add(sigLong, "sigLong", "%5.3f");
			summary.add(sigTrans, "sigTrans", "%5.3f");
			summary.add(sigTheta, "sigTheta", "%5.3f");
//            summary.add(sigTime, "sigTime", "%5.3f");
			summary.add(rmsTime, "rmsTime", "%5.3f");
	}
};

#endif // _DBCALShower_

