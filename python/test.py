import pyfade
import numpy as np
import matplotlib.pyplot as pt
def generateData(Nx=100,minNy=1,maxNy=10):

    data = []

    # x = np.random.uniform(-2,5,(Nx,))
    x = np.linspace(-2,2,Nx)
    x.sort()
    
    sig = 0.1
    sep = 0.3
    peak = np.pi/3
    for i in range(Nx):
        xx = x[i]
        Ny = np.random.randint(minNy,maxNy+1)
        if xx < -1:
            hull = peak * (xx + 2)
            yy = hull + np.random.normal(0,sig,(Ny,))
        elif xx < 1:
            hull = peak
            yy = hull + np.random.normal(0,0.02 + sig*xx**2,(Ny,))
        else:
            hull = -peak/2 * (xx - 3)
            r = np.random.uniform(0,1,(Ny,))
            dup = r > 0.5
            ddown = r <= 0.5
            yy = hull + (dup * np.random.normal(sep*(xx-1),sig,(Ny,)) + ddown * np.random.normal(-sep*(xx-1),sig,(Ny,)))             
        yy = np.maximum(0,yy) 
        yy.sort()
        data.append([xx,yy])
    return data

def WiTest(data):
    styles = {}
    styles[1] = "-"
    styles[3] = "--"
    styles[5] = ":"
    for Nd in [1,5]:
        lineoffset = 0.1*(Nd-1)*0
        nodeoffset = 0.1*(Nd-1)
        np.random.seed(1)
        pt.gca().set_prop_cycle(None)
        a = pyfade.FADE(Nd,5,1,1)
        a.Domain = [[-3,3]]
        a.Train(data)

        a.PredictAt([1])

        ### check Wi
        x = np.linspace(-3,3,500)
        dep = [[],[]]
        for i in range(a.Nd):
           dep[0].append(a.DepPos[i])
           dep[1].append(1+nodeoffset)

        Wis = np.zeros((a.Ne, len(x)))
        for i in range(len(x)):
            xx = x[i]
            a.ComputeWeights(xx)
            for e in range(a.Ne):
                Wis[e,i] = a.Wi[e]
        for e in range(a.Ne):
            pt.plot(x,Wis[e,:]+lineoffset,styles[Nd])
            pt.scatter([a.ExpPos[e]],[1+nodeoffset],marker = "1")
        pt.scatter(dep[0],dep[1],marker="v")
    pt.show()

def Plot(data,fade,steps):
    pt.gcf().clear()
    Nsample = len(data)
    axs = pt.gcf().subplots(1,Nsample)
    pt.suptitle(f"{steps}-Convergence, Score = {fade.Score(data):.2f}")
    for i in range(len(data)):
        axs[i].hist(data[i][1],bins=20,density=True)
        axs[i].set_title(f"x={data[i][0]:.2f}")

        x = np.linspace(min(data[i][1]),max(data[i][1]),100)
        # x = np.linspace(-0.1,1.3,100
        y = fade.PredictAt(data[i][0],x)
        axs[i].plot(x,y)
    pt.draw()
    pt.pause(0.2)
def ConvergeTest(data):
    a = pyfade.FADE(2,7,1,3)
    a.Randomise()
    a.Randomise()
    Plot(data,a,0)
    input("evolve?")
    
    for s in range(50):
        a.EMUpdate(data)
        if ((s+1)%1 == 0):
            Plot(data,a,s+1)
        # input("evolve?")
    pt.draw()
    pt.pause(0.01)
    input("exit?")
     
np.random.seed(2)
 
  
data = generateData(10,180,270)
ConvergeTest(data)
# WiTest(data)
