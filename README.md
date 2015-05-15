# On-Off-Keying-module


// This is a guide, or moreso, a list or resources giving instructions on how to install NS-3 and run a simulation

LINKS:

Overall NS-3 tutorial: https://www.nsnam.org/docs/release/3.21/tutorial/singlehtml/index.html
Installation guide: https://www.nsnam.org/wiki/Installation
                -Follow the instructions for installation depending on which flavor of Linux

Once an example script is made and compiles, to run this script it must be in the scratch directory.
                                          -To get to this directory run the cmd: cd repos/ns-3-allinone/ns-3-dev/scratch
                                          
NS-3 file documentation: https://www.nsnam.org/doxygen/index.html

Description:
Currently this module mimcs that of a point to point connection but it will soon be akin to that of an OOK modulation scheme through an optical light channel. The main module files are in the model folder and the helper is there to make seting objects of this type in a script easier and require less overhead.


//installing the actual module

Prerequisites:
-Having the NS3 environment installed and running in your linux environment
-ability to open .zip files

Installation:
1) download .zip file in this repository: https://github.com/RyanAckerman/On-Off-Keying-module
2) open .zip file and you should see a simple folder named On-Off-Keying-module-master
3) enter that folder and you should see folders that mimic the ones in the repository
4) now, find your NS3 files(if you have followed the tutorial above for installing you should be able to figure out where it is)
5) In those files there is a folder titled "src", go into that folder
6) you should now see all the modules that NS3 builds and are available, simply make a new folder in "src" titled "On-Off-Keying-module"
7) Look back to where you unziped the downloaded repository files and copy and paste those folders and files into this newly made folder
8) Now, start up the terminal and go to the ns-3-dev directory
9) While in this directory build all the NS3 modules (if unsure use the installation guide to help, I use ubuntu and my command is ./waf build)
10) It is optional to enable tests and examples but it should build regardless
11) It may take a while but it should build, and now you can use the on-off-keying-module in NS3 simulations
