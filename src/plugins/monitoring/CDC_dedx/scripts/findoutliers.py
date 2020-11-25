import math

f = open("dedx_mean_at_1_5GeV.txt", "r")
for x in f:
  dedx = x.split()[1]
  runfile = x.split()[0]
  if abs(float(dedx)-2.02) > 0.02:
    print(runfile + " " + dedx)
