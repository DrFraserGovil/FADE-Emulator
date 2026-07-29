import numpy as np
import sys
from numpy.typing import NDArray

def ale(x,y):
    if x > y:
        return x + np.log1p(np.exp(y- x))
    else:
        return y + np.log1p(np.exp(x - y))

class Cache:
    def __init__(self):
        self.Wi :NDArray | None = None
        self.Mu :NDArray | None = None
        self.Vs :NDArray | None = None
        self.Weights :NDArray | None = None


class FADE:


    def __init__(self,Nd,Ne,dimension,order):
        self.Nd = Nd
        self.Ne = Ne
        self.Dim = dimension
        self.P = order
        self.Domain = [[0,1]]*self.Dim 
    def Randomise(self):

        self.DepPos = []
        self.ExpPos = []
        for i in range(self.Ne):
            seed = np.zeros((self.Dim,))
            for j in range(self.Dim):
                seed[j] = np.random.uniform(self.Domain[j][0], self.Domain[j][1])
            self.ExpPos.append(seed)
        unif = np.linspace(-2,2,self.Nd)
         
        self.logWeights = [np.zeros((self.P-1,))]*self.Ne
        self.Mus = [np.zeros((self.P,))] * self.Ne
        self.logSig = [np.zeros((self.P,))] * self.Ne
        self.logExpScale = np.zeros((self.Ne,))
        self.Vs = self.logSig.copy()
        self.Weights = self.Mus.copy()
        self.ExpScale = self.logExpScale.copy()
        for i in range(self.Ne):
            self.Mus[i] = np.random.uniform(0,1, (self.P,))
            self.logSig[i] = np.random.uniform(-2,0, (self.P,))
            self.logWeights[i] = np.random.uniform(-1,1,(self.P-1,))
            self.logExpScale[i] = np.random.uniform(-3,1)
        self.Tki = np.zeros((self.Nd, self.Ne))
        self.Wi = np.zeros((self.Ne,))
        self.wki = np.zeros((self.Nd,self.Ne))
        phiDim = int(self.Dim * (self.Dim + 1)/2)
        self.Phi = []
        self.Gk = []
        for i in range(self.Nd):
            seed = np.zeros((self.Dim,))
            for j in range(self.Dim):
                seed[j] = np.random.uniform(self.Domain[j][0], self.Domain[j][1])
            self.DepPos.append([unif[i]])
            self.Phi.append(np.atleast_1d(np.random.uniform(-1,1,(phiDim,))))
            self.Gk.append(np.eye(self.Dim))
        self.Transform()
        self.WeightExperts()
    def Transform(self):
        for i in range(self.Ne):
            self.Weights[i] = np.exp( np.concatenate([[1],self.logWeights[i]]))
            self.Weights[i] /= np.sum(self.Weights[i])
            self.Vs[i] = np.exp(2*self.logSig[i])
            minScale = 0.01
            maxScale = 2
            self.ExpScale[i] = 0.01 + (maxScale - minScale)/(1 + np.exp(-self.logExpScale[i]))  
        for j in range(self.Nd):
            id = 0
            for x in range(self.Dim):
                self.Gk[j][x,x] = np.exp(self.Phi[j][id])
                id += 1
                for y in range(x+1,self.Dim):
                    self.Gk[j][x,y] = self.Phi[j][id]
                    self.Gk[j][y,x] = self.Phi[j][id]
                    id += 1
    def T(self,k, pos):
        if self.Nd == 1:
            return 1
        else:
            sum = 1e-100
            keepK = 0
            for kk in range(self.Nd):
                K =  self.LogKernel(kk,pos,self.DepPos[kk],0.1)
                if (kk == 0):
                    sum = K 
                else:
                    sum = ale(sum,K)


                if kk == k:
                    keepK = K
            return np.exp(keepK-sum)

    def WeightExperts(self):
        for k in range(self.Nd):
            for i in range(self.Ne):
                self.Tki[k,i] = self.T(k,self.ExpPos[i])

    def Train(self,data):
        self.Randomise()
        Nt = 0
        for i in range(len(data)):
            data[i][1] = np.atleast_1d(data[i][1])
            data[i][0] = np.atleast_1d(data[i][0])
            Nt += len(data[i][1])
            if len(data[i][0]) != self.Dim:
                raise ValueError(f"Dimension mismatch: was told to expect x-dim = {self.Dim}, but input was {data[i][0]}") 
        print(f"Training with {Nt} data points spread across {len(data)} unique points in emulation space")
         

        self.Transform()
        self.WeightExperts()
    def LogKernel(self, k, pos1, pos2,scale=1.0):
        r = pos1 - pos2
         
        d =  np.dot(r, self.Gk[k] @ r)/scale
        return -0.5*d*d

    def ComputeWeights(self,pos):
        for k in range(self.Nd):
            sum = 1e-100
            for i in range(self.Ne):
                term = np.log(self.Tki[k,i]+1e-100) + self.LogKernel(k,pos,self.ExpPos[i],self.ExpScale[i])
                self.wki[k,i] = term
                if (i == 0):
                    sum = term
                else:
                    sum = ale(sum,term)
            self.wki[k,:] =np.exp(self.wki[k,:] - sum)

        for i in range(self.Ne):
            sum = 1e-100
            for k in range(self.Nd):
                sum += self.wki[k,i] * self.T(k,pos)
            self.Wi[i] = sum

        W = np.zeros((self.P,))
        Mu = np.zeros((self.P,))
        Vs = np.zeros((self.P,))
        for i in range(self.Ne):
            W += self.Wi[i] * self.Weights[i]
            Mu += self.Wi[i] * self.Mus[i]
            Vs += self.Wi[i] * self.Vs[i]
        return W,Mu,Vs
        

    def PredictAt(self, position,points):
        pos = np.atleast_1d(position)
        if pos.size != self.Dim:
            raise ValueError("Inconsistent dimensions in prediction query point")
        W,Mu,Vs = self.ComputeWeights(pos) 
        output = np.zeros(points.size)

        print(self.DepPos)
        print(self.ExpPos)
        print(self.Wi)
        for j in range(len(points)):
            s = 0
            for p in range(self.P):
                d = (points[j] - Mu[p])
                s += W[p] * np.exp(-0.5 * d*d/Vs[p]) * 1.0/np.sqrt(2*np.pi * Vs[p])
            output[j] = s
        return output

    def EMUpdate(self,data):
        self.EPhase(data)
        self.MPhase_Mus(data)
        for i in range(15):
            self.MPhase_Weights(data)
        self.MPhase_Vars(data)
        self.WeightExperts()
    def EPhase(self,data):
        self.CacheValues(data)
        self.ET = []
        log2pi = np.log(2*np.pi)
        for t in range(len(data)):
            Nt = len(data[t][1])
            et = np.zeros((self.P,Nt))
            
            for a in range(Nt):
                s = 0
                for p in range(self.P):
                    d = self.Cache[t].Mu[p] - data[t][1][a]
                    cont = np.log(self.Cache[t].Weights[p]) - 0.5*log2pi - 0.5 * np.log(self.Cache[t].Vs[p]) - 0.5*d*d/self.Cache[t].Vs[p] 
                    if p == 0:
                        s = cont
                    else:
                        s = ale(s,cont)
                    et[p][a] = cont
                for p in range(self.P):
                    et[p][a] = np.exp(et[p][a] -s)


            self.ET.append(et)

    def CacheValues(self,data):
        self.Cache = []
        for t in range(len(data)):
            nc = Cache()
            W,Mus,Vs = self.ComputeWeights(data[t][0])
            nc.Weights = W.copy()
            nc.Mu = Mus.copy()
            nc.Vs = Vs.copy()
            nc.Wi = self.Wi.copy()
            self.Cache.append(nc)
    def MPhase_Mus(self,data):

        mu = np.zeros((self.Ne,))
        b = np.zeros(mu.shape)
        M = np.zeros((self.Ne,self.Ne))
        for p in range(self.P):
            for i in range(self.Ne):
                mu[i] = self.Mus[i][p]
            b*= 0
            M*= 0 
            for t in range(len(data)):

                for a in range(len(data[t][1])):
                    for i in range(self.Ne):
                        b[i] += data[t][1][a] * self.Cache[t].Wi[i] * self.ET[t][p,a] / self.Cache[t].Vs[p]
                        for j in range(i,self.Ne):
                            term = self.Cache[t].Wi[i] * self.Cache[t].Wi[j] * self.ET[t][p,a]/self.Cache[t].Vs[p]
                            M[i,j] += term
                            M[j,i] = M[i,j]
            try:
                newMu = np.linalg.solve(M,b)
                for i in range(self.Ne):
                    self.Mus[i][p] = newMu[i]
            except:
                #nothing
                print("oops")
       
        for t in range(len(self.Cache)):
            self.Cache[t].Mu*= 0
            for i in range(self.Ne):
                self.Cache[t].Mu += self.Cache[t].Wi[i] * self.Mus[i]
    def MPhase_Weights(self,data):

        for e in range(self.Ne):
            c = np.zeros((self.P,))
            for p in range(self.P):
                for t in range(len(data)):
                    for a in range(len(data[t][1])):
                        c[p] += self.ET[t][p,a] * self.Cache[t].Wi[e] * self.Weights[e][p] / self.Cache[t].Weights[p]
            c/= np.sum(c)
            for p in range(self.P):
                self.Weights[e][p] = c[p]
    
        for t in range(len(self.Cache)):
            self.Cache[t].Weights*= 0
            for i in range(self.Ne):
                self.Cache[t].Weights += self.Cache[t].Wi[i] * self.Weights[i]
    def MPhase_Vars(self,data):
        for p in range(self.P):
            for i in range(self.Ne):
                for it in range(5):
                    grad = 0
                    curve = 0
                    for t in range(len(self.Cache)):
                        for a in range(len(data[t][1])):
                            d = data[t][1][a] - self.Cache[t].Mu[p]
                            pref = self.ET[t][p,a] * self.Cache[t].Wi[i]
                            vp = self.Cache[t].Vs[p]
                            b1 = (d*d/(2*vp*vp) - 1.0/(2*vp))
                            # b2 = (1.0/(2*vp*vp) - d*d/(vp*vp*vp))
                            b2 = -1.0 / (2.0 * vp * vp)
                            grad += pref * b1
                            curve += pref * self.Cache[t].Wi[i] * b2
                    param = self.Vs[i][p]
                    gamma = 1e-9
                    v0 = 0.05
                    prior_grad = (gamma/param) * (v0/param - 1.0)
                    prior_curve = -gamma /(param * param)
                    grad += prior_grad
                    curve += prior_curve

                    newV = max(1e-6,self.Vs[i][p] - grad/curve)
                    # newV =self.Vs[i][p] - grad/curve

                  
                    self.Vs[i][p] = newV
                    for t in range(len(self.Cache)):
                        self.Cache[t].Vs[p] = np.sum([self.Cache[t].Wi[e] * self.Vs[e][p] for e in range(self.Ne)])

    def Score(self, data):

        s = 0
        for [x,y] in data:
            self.ComputeWeights(x)

            W = np.zeros((self.P,))
            Mu = np.zeros((self.P,))
            Vs = np.zeros((self.P,))
            for i in range(self.Ne):
                W += self.Wi[i] * self.Weights[i]
                Mu += self.Wi[i] * self.Mus[i]
                Vs += self.Wi[i] * self.Vs[i]
            for idx in range(len(y)):
                logprob = 0
                for p in range(self.P):
                    d = y[idx] - Mu[p]
                    if (Vs[p] < 0):
                        raise RuntimeError("oops")
                    modeP = np.log(W[p]) - 0.5 * np.log(Vs[p]) - 0.5/Vs[p] * d * d
                    if p == 0:
                        logprob = modeP
                    else:
                        logprob = ale(logprob,modeP)
                s += logprob
        return s
                
