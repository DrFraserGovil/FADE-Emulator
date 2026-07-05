import matplotlib.pyplot as pt
import numpy as np
from sklearn.mixture import GaussianMixture

def show():
    pt.pause(0.1)
    pt.draw()
    input("Enter to continue")
def save(filename):
    pt.pause(0.1)
    pt.draw()
    pt.savefig(filename,bbox_inches='tight')

def kriger(x, y_obs, predrange):
    n = len(x)
    k = np.zeros((n, n))
    order = 3
    phi = np.zeros((order + 1, n)) 
    sig = 3
    err = 0.2
    scale = 3
    
    # Build Covariance Matrix and Trend Matrix
    for i in range(n):
        xi = x[i]
        for j in range(i, n):
            xj = x[j]
            kij = np.exp(- (xi - xj)**2 / (2 * sig))
            k[i, j] = kij
            k[j, i] = kij
        k[i, i] += err
        
        for p in range(order + 1):
            phi[p, i] = xi**p

    kinv = np.linalg.inv(k)
    
    c = phi @ kinv @ phi.T
    cinv = np.linalg.inv(c)
    
    beta_weights = cinv @ phi @ kinv  # Shape: (order+1, n)
    beta = beta_weights @ y_obs
    
    y_pred = np.zeros((len(predrange),))
    E = np.zeros(y_pred.shape)
    for i in range(len(predrange)):
        # Kernel vector between training data and prediction point
        ki = np.exp(- (x - predrange[i])**2 / (2 * sig))
        
        # Polynomial features for the prediction point
        phi_i = np.array([predrange[i]**p for p in range(order + 1)])
        
        y_pred[i] = ki.T @ kinv @ (y_obs - phi.T @ beta) + phi_i @ beta
       
        polyG = (phi_i - phi @ kinv @ ki)
    
        E[i] = scale * (1.0 - ki.T @ kinv @ ki + polyG.T @ cinv @ polyG) + err

    return y_pred,E
def divergentPlots():
    pt.figure(figsize=(10,5),dpi=100)

    x = np.linspace(0,10,100)
    y = lambda x:  x - 0.08*x**2
    y2 = lambda x:  x + 0.02*x**2 - 0.01*x**3
    # pt.plot(x,y(x))
    # pt.plot(x,y2(x))

    N = 3000
    xp = np.random.uniform(x[0],x[-1],(N,))
    xp = np.sort(xp)
    frac =0.00

    for frac in [0,0.1,0.5]:
        pt.cla()
        pop = np.random.random((N,)) < frac
        
        yp = (pop==0)*np.random.normal(y(xp),0.1)+ (pop==1)*np.random.normal(y2(xp),0.2)

        ####### initial plot
        pt.scatter(xp,yp,2)
        pt.grid()
        pt.xlabel("$x$")
        pt.ylabel("$y$")
        fprint = int(10*frac)
        save(f"data-only-f{fprint}.pdf")
        # yp = y(x) +  
        X = np.linspace(xp[0],xp[-1],100)
        Y,E = kriger(xp,yp,X)
        pt.plot(X,Y)
        pt.fill_between(X,Y-E,Y+E,alpha=0.2)
        save(f"fit-f{fprint}.pdf")
def fitGauss(y, n_components):
    # Ensure y is a 2D column vector of shape (N, 1) as required by scikit-learn
    y_samples = np.asarray(y).reshape(-1, 1)
    
    # Fit a 1D Gaussian Mixture Model
    # 'spherical' is perfect for 1D because variance is just a single scalar per component
    gmm = GaussianMixture(n_components=n_components)
    gmm.fit(y_samples)
    
    # Extract flat 1D parameters
    weights = gmm.weights_
    means = gmm.means_.flatten()
    variances = gmm.covariances_.flatten()
    return weights, means, np.sqrt(variances)
def simpleTiling():


    x = np.linspace(0,10,100)
    y = lambda x:  x - 0.08*x**2
    y2 = lambda x:  x + 0.02*x**2 - 0.01*x**3

    N = 400
    xp = np.random.uniform(x[0],x[-1],(N,))
    xp = np.sort(xp)
    frac = 0.4
    pop = np.random.random((N,)) < frac
    yp = (pop==0)*np.random.normal(y(xp),0.1)+ (pop==1)*np.random.normal(y2(xp),0.2)
   
    pop2 = np.random.random((N,)) < frac
    yp2 = (pop2==0)*np.random.normal(y(xp),0.1)+ (pop2==1)*np.random.normal(y2(xp),0.2)

    for tiling in [1,4,9,24,44]:
        de = 0.01
        fig = pt.figure(figsize=(10,5),dpi=100)
        gs = fig.add_gridspec(2, tiling+1)
        axs = []
        for i in range(tiling+1):
            axs.append(fig.add_subplot(gs[0,i]))
            axs[i].set_yticks([])
        bigax = fig.add_subplot(gs[1,:])
        width = (x[-1] - x[0])/(tiling+1)
        r = x[0]
        overall = 0
        id = 0
        while r < x[-1]:
            subset = np.logical_and(xp < r+width,xp >=r)
            obj = bigax.scatter(xp[subset],yp[subset],alpha=0.1)
            col = obj.get_facecolor()[0]
            if len(yp[subset] > 3):
                yyp = np.sort(yp[subset])
                yyp2 = np.sort(yp2[subset])
                for order in range(2,4):
                    mix = fitGauss(yyp,order)

                    base = np.linspace(yyp[0],yyp[-1],100)
                    fit = np.zeros(base.shape)
                    for i in range(order):
                        d = (base - mix[1][i])/mix[2][i]
                        fit += mix[0][i] /(np.sqrt(2*np.pi) * mix[2][i])* np.exp(-0.5 * d**2)
                    a = np.exp(-(np.abs(order - 2))/3)
                    a = max(0.2,a)
                    axs[id].plot(base,fit,color=col,alpha=a)

                    if (order == 2):
                        score = 0
                        for yv in yyp2:
                            v = 0
                            for p in range(order):
                                w = mix[0][p]
                                mu = mix[1][p]
                                sig = mix[2][p]
                                d = (yv - mu)/sig
                                v += w/(np.sqrt(2*np.pi) * sig) * np.exp(-0.5 * d**2)
                            score += np.log(de*v)
                        axs[id].set_title(f"{score:.1f}")
                        overall += score
             
            id += 1
                

            r += width
        bigax.set_xlabel(f"Fit score = {overall:.1f}")
        save(f"partition-{tiling+1}-data.pdf")

simpleTiling()

# divergentPlots()




