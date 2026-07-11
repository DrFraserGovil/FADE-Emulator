import numpy as np
import matplotlib.pyplot as pt

def OneDFunction():

    N = 1000
    xTrain = np.random.uniform(-2,3,(N,))
    xTrain.sort()

    domain1 = xTrain <  - 1
    domain2 = np.logical_and(xTrain < 1, xTrain >= -1)
    domain3 = xTrain >= 1 

    peak = np.pi/3
    sig = 0.1
    sep = 0.3
    y1 = peak * (xTrain +2) * domain1
    y1 += np.random.normal(0,0.1,(N,)) * domain1
    y2 = peak * domain2
    y2 += np.random.normal(0,0.02 + sig*xTrain**2) * domain2
    
    y3 = -peak/2 * (xTrain -3) * domain3 
    
    r = np.random.uniform(0,1,(N,))
    dd3p = np.logical_and(domain3,r>0.5)
    dd3m = np.logical_and(domain3,r<=0.5)
    print(dd3p.shape,dd3m.shape) 

    y3 += (np.random.normal(sep*(xTrain-1),sig)*dd3p + np.random.normal(-sep*(xTrain-1),sig)*dd3m)

    sample = np.maximum(0,y1 + y2 + y3)
    r = np.ones((N,)) 
    X = np.array([xTrain,r,sample,r]).T
    np.savetxt("single_train.dat",X, delimiter= " ",fmt='%.7f') 

    ### generate queries
    q = 100
    xQ = np.linspace(-2,3,q) + np.random.normal(0,0.1,(q,))
    xQ.sort()
    np.savetxt("single_query.dat",xQ,delimiter=" ",fmt='%.7f')
     
    ### generate bounded queries
    force = np.ones((3,q))
    force[0,:] = xQ
    force[1,:] = 0
    force[2,:] = peak + np.random.normal(0,sig,(q,))
    np.savetxt("single_query_bounded.dat",force.T,delimiter=" ",fmt='%.7f')
OneDFunction() 
