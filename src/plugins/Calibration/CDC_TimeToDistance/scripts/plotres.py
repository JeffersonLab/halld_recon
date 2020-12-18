
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df = pd.read_fwf('res.txt',colspecs=[(0,5),(15,22),(29,36)],header=None)

#print df.head()
print("Statistics:")
print(df.describe())

run = df[0]
mean = df[1]
sigma = df[2]

plt.plot(run,10000*mean, 'bo')


plt.title('Mean of single gaussian fitted to track fit residuals')
plt.xlabel('Run number')
plt.ylabel(r'Mean of residuals (um)')


plt.show()

plt.plot(run,10000*sigma, 'bo')


plt.title('Sigma of single gaussian fitted to track fit residuals')
plt.xlabel('Run number')
plt.ylabel('Sigma of track fit residuals (um)')

plt.show()


#plt.plot(x,y, 'bo', x, poly1d_fn(x), 'r-')

#coef = np.polyfit(x,y,1)

#fpoly1 = np.poly1d(coef)

#plt.plot(x,y,'bo')



#plt.savefig("resid_mean.png")



#plt.plot(x,y,'bo',fpoly1(x),'--k')
#plt.show()


