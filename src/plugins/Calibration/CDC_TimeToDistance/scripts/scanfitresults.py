import math

f = open("res.txt", "r")
for x in f:
  resid = x.split()[3]
  runfile = x.split()[0]
  if abs(float(resid)) > 0.0008:
    print(runfile + " " + resid)
