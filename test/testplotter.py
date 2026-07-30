import numpy as np
import matplotlib.pyplot as pt

from pathlib import Path
import re
def GetFiles(directory, pattern,recursive = False):
    path = Path(directory)
    search = path.rglob if recursive else path.glob
    
    return [f for f in search(pattern) if f.is_file()]
 
def get_weight_indices(path: Path) -> tuple[int, int]:
    """Extracts (a, b) as integers from 'weights_a_b.dat' for proper numerical sorting."""
    match = re.search(r"weights_(\d+)_(\d+)", path.name)
    if match:
        return (int(match.group(1)), int(match.group(2)))
    return (0, 0)  # Fallback for unexpected filenames
 
def WeightPrint():
    files = GetFiles(".","weights_*.dat")
    # files.sort(key=get_weight_indices)
    files = sorted(files,key=get_weight_indices)

    fig,axs = pt.subplots(len(files),1)
    id = 0
    for file in files:
        first = True
        data = []
        expPos = []
        with open(file) as f:
            for line in f:
                if first:
                    first = False
                    for pos in line.rstrip().split(" "):
                        expPos.append(float(pos))
                else:
                    ldata = []
                    line = line.rstrip().split(" ")
                    for d in line:
                        ldata.append(float(d))
                    data.append(ldata)
        d = np.array(data)
        ne = d.shape[1]-1
        for n in range(ne):
            axs[id].scatter([expPos[n]],[1],marker="1")
        axs[id].set_prop_cycle(None)
        for n in range(ne):
            axs[id].plot(d[:,0], d[:,n+1]+0.001/(n+1))
        id += 1
    pt.draw()
    pt.pause(0.1)
    input("continue?")
WeightPrint()
    
