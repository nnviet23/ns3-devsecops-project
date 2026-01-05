------------------------------------------------------------------------
VNUHCM - UNIVERSITY OF SCIENCE

FACULTY OF INFORMATION TECHNOLOGY
Date: January 05, 2026
------------------------------------------------------------------------
Department: Networks and Telecommunications (Information Security Track)

Course: CSC11004 - Advanced Computer Networking

Project: Research on TCP Congestion Control & DevSecOps Pipeline
------------------------------------------------------------------------
STUDENT INFORMATION
⦁ Student ID: 23127517
⦁ Full Name: Nguyễn Nam Việt
⦁ Email: nnviet23@clc.fitus.edu.vn
------------------------------------------------------------------------

TCP Performance Comparison: Cubic vs. BBR
This project evaluates the performance of TCP Cubic and TCP BBR in a high-latency WAN environment with a 5% packet loss rate using the NS-3 (Network Simulator 3).

I. System Setup
1. Prerequisites
Before installing the simulator, download and install the following tools on your host machine:
⦁ VirtualBox: https://www.virtualbox.org/wiki/Downloads 
⦁ MobaXterm: https://mobaxterm.mobatek.net/download.html 

2. Environment Configuration
Once inside your Linux VM (Ubuntu), install the necessary dependencies for NS-3 and data visualization:
# Update system and install build tools
	sudo apt update
	sudo apt install g++ python3 cmake ninja-build git libc6-dev -y

# Install critical libraries
⦁ libxml2: Required for FlowMonitor to export Throughput/Delay data to XML.
⦁ libsqlite3-dev: Enables database storage for simulation results.
⦁ python3-pip: Required to install plotting libraries.
	sudo apt install libsqlite3-dev libxml2 libxml2-dev python3-pip -y

# Install Python visualization libraries
	pip3 install matplotlib pandas

3. NS-3 Installation
Download and build NS-3 version 3.41:
⦁ Download Source: https://www.nsnam.org/releases/ns-3-41/download/
⦁ Extract the archive
	tar xjf ns-allinone-3.41.tar.bz2
	cd ns-allinone-3.41/ns-3.41

⦁ Configure the build (Optimized for speed, bypassing Tiger Lake CPU errors)
	./ns3 configure --build-profile=optimized --enable-examples --enable-tests -- -DNS3_NATIVE_OPTIMIZATIONS=OFF

⦁ Compile (This process may take 20-40 minutes)
	./ns3 build
II. Running the Simulation
1. Execute Simulation
Ensure your source file tcp-cubic-vs-bbr.cc is placed in the scratch/ directory.
⦁ Navigate to the NS-3 directory
	cd ~/ns-allinone-3.41/ns-3.41

⦁ Run the simulation script
	./ns3 run scratch/tcp-cubic-vs-bbr

2. Verify Results
The simulation generates an XML file containing flow statistics. You can verify the data using:
⦁ Check the generated XML for Flow IDs and basic stats
	cat results.xml | grep "flowId"

3. Data Visualization
Use the provided Python script to generate a comparison chart:
⦁ Generate the throughput comparison bar chart
	python3 plot_results.py

The output chart will be saved as simulation_chart.png in the current directory, showing the performance gap between TCP Cubic and TCP BBR under lossy network conditions.
