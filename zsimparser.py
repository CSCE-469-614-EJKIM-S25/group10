# -*- coding: utf-8 -*-
import os
from pathlib import Path
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
matplotlib.use('Agg')

#IMPORTANT: This must be run on linux
directory = "./zsim/outputs"
subdirs = ["/LRU", "/LFU", "/NRU", "/Rand", "/SRRIP", "/Mockingjay"]

SPEC = "bzip2 gcc mcf hmmer sjeng libquantum xalan milc cactusADM leslie3d namd soplex calculix lbm".split(" ")
PARSEC = "blackscholes bodytrack canneal fluidanimate streamcluster swaptions x264".split(" ")
SPEC_IPCs = [{} for _ in range(len(subdirs))]
SPEC_MPKIs = [{} for _ in range(len(subdirs))]
SPEC_CCs =  [{} for _ in range(len(subdirs))]
PARSEC_IPCs = [{} for _ in range(len(subdirs))]
PARSEC_MPKIs = [{} for _ in range(len(subdirs))]
PARSEC_CCs =  [{} for _ in range(len(subdirs))]

for subdir in subdirs:
    for root, dirs, files in os.walk(directory + subdir):
        for file in files:
            # If the file has the specified extension, open it
            if file.endswith(".out"):
                file_path = Path(root) / file
                with open(file_path, 'r') as f:
                    #print(f"Opening file: {file_path}")
                    #logic for finding stuff in the file
                    totalCycles = 0
                    instructions = 0
                    misses = 0
                    lines = f.readlines()
                    inL3 = False
                    for line in lines:
                        trimmedLine = line.strip()
                        words = trimmedLine.split()
                        if words[0] == "cycles:" or words[0] == "cCycles:":
                            totalCycles = totalCycles + int(words[1])
                        elif words[0] == "instrs:":
                            instructions = instructions + int(words[1])
                        elif words[0] == "l3:":
                            inL3 = True
                        elif inL3 and words[0].startswith("mGET"):
                            misses = misses + int(words[1])
                    if instructions != 0 and totalCycles != 0: 
                        IPC = instructions/totalCycles
                        MPKI = misses*1000/instructions
                        spec = ""
                        parsec = ""
                        for spec_bm in SPEC:
                            if spec_bm in str(file_path):
                                spec = spec_bm
                        for parsec_bm in PARSEC:
                            if parsec_bm in str(file_path):
                                parsec = parsec_bm
                        if spec != "":
                            if subdir == "/LRU":
                                SPEC_IPCs[0][spec] = IPC
                                SPEC_MPKIs[0][spec] = MPKI
                                SPEC_CCs[0][spec] = totalCycles/1000000
                            if subdir == "/LFU":
                                SPEC_IPCs[1][spec] = IPC
                                SPEC_MPKIs[1][spec] = MPKI
                                SPEC_CCs[1][spec] = totalCycles/1000000
                            if subdir == "/NRU":
                                SPEC_IPCs[2][spec] = IPC
                                SPEC_MPKIs[2][spec] = MPKI
                                SPEC_CCs[2][spec] = totalCycles/1000000
                            if subdir == "/Rand":
                                SPEC_IPCs[3][spec] = IPC
                                SPEC_MPKIs[3][spec] = MPKI
                                SPEC_CCs[3][spec] = totalCycles/1000000
                            if subdir == "/SRRIP":
                                SPEC_IPCs[4][spec] = IPC
                                SPEC_MPKIs[4][spec] = MPKI
                                SPEC_CCs[4][spec] = totalCycles/1000000
                            if subdir == "/Mockingjay":
                                SPEC_IPCs[5][spec] = IPC
                                SPEC_MPKIs[5][spec] = MPKI
                                SPEC_CCs[5][spec] = totalCycles/1000000
                        elif parsec != "": #all of these are a factor of 10 smaller
                            if subdir == "/LRU":
                                PARSEC_IPCs[0][parsec] = IPC
                                PARSEC_MPKIs[0][parsec] = MPKI
                                PARSEC_CCs[0][parsec] = totalCycles/10000000
                            if subdir == "/LFU":
                                PARSEC_IPCs[1][parsec] = IPC
                                PARSEC_MPKIs[1][parsec] = MPKI
                                PARSEC_CCs[1][parsec] = totalCycles/10000000
                            if subdir == "/NRU":
                                PARSEC_IPCs[2][parsec] = IPC
                                PARSEC_MPKIs[2][parsec] = MPKI
                                PARSEC_CCs[2][parsec] = totalCycles/10000000
                            if subdir == "/Rand":
                                PARSEC_IPCs[3][parsec] = IPC
                                PARSEC_MPKIs[3][parsec] = MPKI
                                PARSEC_CCs[3][parsec] = totalCycles/10000000
                            if subdir == "/SRRIP":
                                PARSEC_IPCs[4][parsec] = IPC
                                PARSEC_MPKIs[4][parsec] = MPKI
                                PARSEC_CCs[4][parsec] = totalCycles/10000000
                            if subdir == "/Mockingjay":
                                PARSEC_IPCs[5][parsec] = IPC
                                PARSEC_MPKIs[5][parsec] = MPKI
                                PARSEC_CCs[5][parsec] = totalCycles/10000000
            
width = 0.2  # Width of each bar
spacing = 1.5
gap = 1
colors = ['b', 'g', 'r', 'y', 'c', 'm'] 
names = ["LRU", "LFU", "NRU", "Rand", "SRRIP", "Mockingjay"]

def plot_grouped_bars(x, keys, values, label_offset):
    for i, val_set in enumerate(values):
        bars = ax.bar(x + i * width, val_set, width=width, label=f'{names[i]}' if label_offset == 0 else None, color=colors[i])
        # Add value labels above bars
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width() / 2, height + 0.2, f'{height:.2f}', 
                    ha='center', va='bottom', rotation=90, fontsize=9)
    return x + width  # Return adjusted x positions for labeling


keysCCS = list(SPEC_CCs[0].keys())  # Same for all dicts
valuesCCS = [list(d.values()) for d in SPEC_CCs]

keysCCP = list(PARSEC_CCs[0].keys())  # Same for all dicts
valuesCCP = [list(d.values()) for d in PARSEC_CCs]

xCCS = np.arange(len(keysCCS))*spacing
xCCP = np.arange(len(keysCCP))*spacing + len(keysCCS)*spacing + gap

fig, ax = plt.subplots(figsize=(15, 7))

# Plot first and second dataset
x1_labels = plot_grouped_bars(xCCS, keysCCS, valuesCCS, 0)
x2_labels = plot_grouped_bars(xCCP, keysCCP, valuesCCP, 1)

# Add x-axis labels at correct positions
ax.set_xticks(np.concatenate((x1_labels, x2_labels)))
ax.set_xticklabels(keysCCS + keysCCP, rotation=90)

# Add a vertical line to separate the groups
ax.axvline(x=len(keysCCS)*spacing + gap / 2 - width / 2, color='black', linestyle='--')

ax.text(len(keysCCS)*spacing / 2 - 0.5, ax.get_ylim()[0] - 300, "SPEC (in millions of CCs)", ha="center", fontsize=12, fontweight="bold")
ax.text(len(keysCCS)*spacing + gap + len(keysCCP)*spacing / 2 - 0.5, ax.get_ylim()[0] - 300, "PARSEC (in tens of millions of CCs)", ha="center", fontsize=12, fontweight="bold")
ax.set_ylim([0, 900])
# Labels and title
ax.set_ylabel("CCs")
ax.set_xlabel("Benchmark")
ax.legend()
ax.set_title("Clock Cycles by benchmark")

plt.tight_layout()
plt.savefig('Results/CC.png')





keysMPKIS = list(SPEC_MPKIs[0].keys())  # Same for all dicts
valuesMPKIS = [list(d.values()) for d in SPEC_MPKIs]

keysMPKIP = list(PARSEC_MPKIs[0].keys())  # Same for all dicts
valuesMPKIP = [list(d.values()) for d in PARSEC_MPKIs]

xMPKIS = np.arange(len(keysMPKIS))*spacing
xMPKIP = np.arange(len(keysMPKIP))*spacing + len(keysMPKIS)*spacing + gap

fig, ax = plt.subplots(figsize=(15, 7))

# Plot first and second dataset
x1_labels = plot_grouped_bars(xMPKIS, keysMPKIS, valuesMPKIS, 0)
x2_labels = plot_grouped_bars(xMPKIP, keysMPKIP, valuesMPKIP, 1)

# Add x-axis labels at correct positions
ax.set_xticks(np.concatenate((x1_labels, x2_labels)))
ax.set_xticklabels(keysMPKIS + keysMPKIP, rotation=90)

# Add a vertical line to separate the groups
ax.axvline(x=len(keysMPKIS)*spacing + gap / 2 - width / 2, color='black', linestyle='--')

ax.text(len(keysMPKIS)*spacing / 2 - 0.5, ax.get_ylim()[0] - 30, "SPEC", ha="center", fontsize=12, fontweight="bold")
ax.text(len(keysMPKIS)*spacing + gap + len(keysMPKIP)*spacing / 2 - 0.5, ax.get_ylim()[0] - 30, "PARSEC", ha="center", fontsize=12, fontweight="bold")
ax.set_ylim([0, 100])
# Labels and title
ax.set_ylabel("MPKI")
ax.set_xlabel("Benchmark")
ax.legend()
ax.set_title("MPKI by benchmark")

plt.tight_layout()
plt.savefig('Results/MPKI.png')




keysIPCS = list(SPEC_IPCs[0].keys())  # Same for all dicts
valuesIPCS = [list(d.values()) for d in SPEC_IPCs]

keysIPCP = list(PARSEC_IPCs[0].keys())  # Same for all dicts
valuesIPCP = [list(d.values()) for d in PARSEC_IPCs]

xIPCS = np.arange(len(keysIPCS))*spacing
xIPCP = np.arange(len(keysIPCP))*spacing + len(keysIPCS)*spacing + gap

fig, ax = plt.subplots(figsize=(15, 7))

# Plot first and second dataset
x1_labels = plot_grouped_bars(xIPCS, keysIPCS, valuesIPCS, 0)
x2_labels = plot_grouped_bars(xIPCP, keysIPCP, valuesIPCP, 1)

# Add x-axis labels at correct positions
ax.set_xticks(np.concatenate((x1_labels, x2_labels)))
ax.set_xticklabels(keysIPCS + keysIPCP, rotation=90)

# Add a vertical line to separate the groups
ax.axvline(x=len(keysIPCS)*spacing + gap / 2 - width / 2, color='black', linestyle='--')

ax.text(len(keysIPCS)*spacing / 2 - 0.5, ax.get_ylim()[0] - 1, "SPEC", ha="center", fontsize=12, fontweight="bold")
ax.text(len(keysIPCS)*spacing + gap + len(keysIPCP)*spacing / 2 - 0.5, ax.get_ylim()[0] - 1, "PARSEC", ha="center", fontsize=12, fontweight="bold")
ax.set_ylim([0, 3])
# Labels and title
ax.set_ylabel("IPC")
ax.set_xlabel("Benchmark")
ax.legend()
ax.set_title("IPC by benchmark")

plt.tight_layout()
plt.savefig('Results/IPC.png')

#Geometric means - comparison across the board for different kinds of benchmarks

with open("Results/output.txt", 'w') as file:
    print("Mean IPC by replacement policy: ", file=file)
    print(file=file)
    LRU_results = []
    for index in range(len(PARSEC_IPCs)):
        temp_valSPEC = 1.0
        temp_valPAR = 1.0
        temp_valBOTH = 1.0
        dictionary = SPEC_IPCs[index]
        for i, key in enumerate(dictionary):
            temp_valSPEC = temp_valSPEC*dictionary[key]
            temp_valBOTH = temp_valBOTH*dictionary[key]
        
        dictionary2 = PARSEC_IPCs[index]
        for i, key in enumerate(dictionary2):
            temp_valPAR = temp_valPAR*dictionary2[key]
            temp_valBOTH = temp_valBOTH*dictionary2[key]
        if names[index] == 'LRU':
            LRU_results.append((temp_valSPEC ** (1.0/len(dictionary))))
            LRU_results.append((temp_valPAR ** (1.0/len(dictionary))))
            LRU_results.append((temp_valBOTH ** (1.0/len(dictionary))))
        print(f'{names[index]}', file=file)
        print(f'SPEC: {(temp_valSPEC ** (1.0/len(dictionary)))}, {((temp_valSPEC ** (1.0/len(dictionary)))/LRU_results[0] - 1)* 100}% better than LRU', file=file)
        print(f'PARSEC: {(temp_valPAR ** (1.0/len(dictionary)))}, {((temp_valPAR ** (1.0/len(dictionary)))/LRU_results[1] - 1)* 100}% better than LRU', file=file)
        print(f'BOTH: {(temp_valBOTH ** (1.0/len(dictionary)))}, {((temp_valBOTH ** (1.0/len(dictionary)))/LRU_results[2] - 1)* 100}% better than LRU', file=file)
        print(file=file)

    
    print("Mean MPKI by replacement policy: ", file=file)
    LRU_results = []
    for index in range(len(PARSEC_MPKIs)):
        temp_valSPEC = 1.0
        temp_valPAR = 1.0
        temp_valBOTH = 1.0
        dictionary = SPEC_MPKIs[index]
        for i, key in enumerate(dictionary):
            temp_valSPEC = temp_valSPEC*dictionary[key]
            temp_valBOTH = temp_valBOTH*dictionary[key]
        
        dictionary2 = PARSEC_MPKIs[index]
        for i, key in enumerate(dictionary2):
            temp_valPAR = temp_valPAR*dictionary2[key]
            temp_valBOTH = temp_valBOTH*dictionary2[key]

        if names[index] == 'LRU':
            LRU_results.append((temp_valSPEC ** (1.0/len(dictionary))))
            LRU_results.append((temp_valPAR ** (1.0/len(dictionary))))
            LRU_results.append((temp_valBOTH ** (1.0/len(dictionary))))

        print(file=file)
        print(f'{names[index]}', file=file)
        print(f'SPEC: {(temp_valSPEC ** (1.0/len(dictionary)))}, {((temp_valSPEC ** (1.0/len(dictionary)))/LRU_results[0] - 1)* -100}% better than LRU', file=file)
        print(f'PARSEC: {(temp_valPAR ** (1.0/len(dictionary)))}, {((temp_valPAR ** (1.0/len(dictionary)))/LRU_results[1] - 1)* -100}% better than LRU', file=file)
        print(f'BOTH: {(temp_valBOTH ** (1.0/len(dictionary)))}, {((temp_valBOTH ** (1.0/len(dictionary)))/LRU_results[2] - 1)* -100}% better than LRU', file=file)
