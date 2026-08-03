import numpy as np
import matplotlib.pyplot as pt


def gauss(x,mu,sigma):
    d = (x-mu)/sigma
    return 1.0/np.sqrt(2*np.pi)/sigma * np.exp(-0.5*d*d)


def GaussMerge():
    x = np.linspace(-3,3,1000)

    fig,axs = pt.subplots(1,3,figsize=(10,7))
    

    y1 = gauss(x,0,1)
    y2 = gauss(x,0,0.1)
    y3 = gauss(x,0,0.65)
    axs[0].plot(x,y1)
    axs[0].set_title("Expert 1")
    axs[1].set_title("Midpoint")
    axs[1].plot(x,0.5*y1 + 0.5*y2,label="PDF Interpolation")
    axs[1].plot(x,y3,label="Parameter Interpolation")
    axs[1].legend(loc="upper right")
    axs[2].set_title("Expert 2")
    axs[2].plot(x,y2)


    pt.savefig("merge.pdf")
    # pt.draw()
    # pt.pause(0.1)
    #
    # input("?")


GaussMerge()
