<h1 align="center">CSCE 469/614 Group 10 Term Project - Mockingjay Cache Replacement Policy</h1>

<h4 align="center">Authors: Lohitaksh Allampalli, Ben Armstrong, Mathimalar Mathivanan</h1>

## Project Description

Below we discuss the goal of the term project and the primary sources we used to accomplish this goal

### Project Goal

This project aims to emulate a modern cache replacement algorithm. After conducting a literature review consisting of papers from top Computer Architecture confereneces such as ISCA, HPCA, and MICRO, we choose a paper to implement its proposed algorithm in the Zsim Simulator. The goal is to recreate the results expressed in the research paper, learning about the simulator and a bleeding edge cache replacement policy in the process. After implementation and simulation, we write a report on our findings, explaining any discrepencies from the source paper. The primary restriction is that the chosen paper and algorithm should not already have an open source Zsim implementation available to use.

We scanned a myriad of research papers, mostly consisting of Program Counter (PC) determined and Machine Learning (ML)/Reinforcment Learning (RL) cache replacement policies. We decided on implementing the fairly new (2022) Mockingjay replacement policy from HPCA as it claimed to mimic Belady's Clairvoyant algorithm, presenting close to optimal performance in the Last Level Cache (LLC). Further, this policy provided an open source implementation in ChampSim which we determined would simplify the development process.

### Resources

The following is the citation to the paper we referenced and emulated:

> I. Shah, A. Jain and C. Lin, "Effective Mimicry of Belady’s MIN Policy," 2022 IEEE International Symposium on High-Performance Computer Architecture (HPCA), Seoul, Korea, Republic of, 2022, pp. 558-572, doi: 10.1109/HPCA53966.2022.00048. keywords: {Prefetching;Computer architecture;Benchmark testing;Hardware;History;Marine vehicles},

This is the link to the public GitHub repository from one of the authors of the papers with their ChampSim implementation of Mockingjay: https://github.com/ishanashah/Mockingjay

## Project Structure

### File Organization & Hierarchy 

This project utilizes Zsim, an open source simulator, to emulate a processor and run the desired algoritms. Zsim has pre-built implementations of LRU, NRU, Random, LFU, and others. Adding a new replacement policy requires creating a new class whose parent is the virtual class, replPolicy. In this child class, we simply implement rank(), update(), and replaced() along with the constructor and destructor to override the virtual methods.

Further, the source code references config files which detail the processor type (e.g OOO) and structure (single core vs multicore), cache hierarchy (how many levels of cache) and structure (size and associativity of cache), replacement policy, and the benchmark that is being used (PARSEC, SPEC, etc). These config files are located in group10/zsim/ and we created configurations for multiple PARSEC benchmarks (testing multicore capabilities) and SPEC benchmarks (single core capabilities).

### Required Environnments 

A Python enviornment is required to be able to compile Zsim with any files added for algorithm implementation. The set up process for this environment is detailed below. Further, the benchmarks must be unzipped in the simulator after cloning the repository. This is also detailed below. Otherwise, all our results can be found an regenerated through the files provided in the simulator.

## Instructions to Compile

#### 1. Clone the repository into the desired location in your local system
#### 2. Unzip benchmarks files:
```
zip -F benchmarks.zip --out single-benchmark.zip && unzip single-benchmark.zip && mkdir benchmarks/parsec-2.1/inputs/streamcluster
```
#### 3. Environemnt setup
- To set up the Python environment for the first time, run the following commands. These should be run in the top directory (group10/):

```
python -m venv venv
source venv/bin/activate
pip install scons
```

- Everytime you want to build or run Zsim, you need to setup the environment variables first. These should also be run in group10/:

```
source venv/bin/activate
source setup_env
```

#### 4. Compile zsim

```
cd zsim
scons -j4
```

You need to compile the code each time you make a change.

###### For more information, check `zsim/README.md`

## Instructions to Run

We have made running the simulations an automatic process, enabling asynchronous execution of each desired benchmark for each desired policy. This is done via a bash script that iterates through the run commmand over the desired parameters.

<hr/>

Before running the simulations, make two optional changes:
- In group10/zsim/termProjectAutoRunScript.sh line 14, include all desired replacement policies in this format ``("<policy_name_1>" "policy_name_2" .... "policy_name_n")``
- In group10/zsim/termProjectRunScript line 9, include all desired replacement policies in this format ``repl_policy: <policy_name_1> policy_name_2 .... policy_name_n"``

We have setup the scripts to run through simulations of Mockingjay, LRU, LFU, NRU, Rand, and SRRIP. The config files already exist as well.

# - TODO: ^^^ DO THIS IN THE RUN SCRIPTS

<hr/>

To run, execute ``./termProjectAutoRunScript.sh`` in ``group10/zsim/``. 
- It is possible that the request is denied because of permissions. If this happens, run the following command: ``chmod 755 ./termProjectAutoRunScript.sh`` then re-execute.

One at a time, this script will run each benchmark for each policy and the outputs will be generated.

## Output File Format and Folder Path

The output files are under the ``./zsim/outputs/`` directory, and are organized into subdirectories by replacement policy.

## Instructions to Generate Plots and Results Information

Ensure that you are in the ``group10`` directory, and that you have ``pip`` installed and in your PATH variables, then run:
```
pip install -r requirements.txt
./zsimparser.py
```
requirements.txt contains dependencies necessary to run ``zsimparser.py`` and the installation of these dependencies and running of the script may be done in a python virtual environment.

Result plots will be generated in the ./Results directory as CC.png (clock cycle comparison), IPC.png (instructions per clock cycle comparison), and MPKI.png (misses per kilo-instruction comparison) alongside output.txt that holds the geometric mean IPC and MPKI for all benchmarks. 

If running a subset of these benchmarks, remove the unused benchmarks from line 13 and 14 of zsimparser.py, making sure to keep the necessary benchmarks space separated.
