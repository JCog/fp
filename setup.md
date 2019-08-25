# Building

This tool requires two programs to compile and build.  
The first being Glank's MIPS N64 Toolchain: https://github.com/glankk/n64  
and Armips: https://github.com/Kingcom/armips  
  
There are instructions on how to download and build both programs on their respective pages, but I'll also detail out 
how to do this on Ubuntu for Windows (WSL).
  
First download and install Ubuntu for Windows. You may need to enable "Linux Subsystem for Windows" in Windows Features when doing this.  
When you have a terminal up and running, first we'll get Glanks toolchain.  
## N64 Toolchain  

cd into the directory of your choice (or just use home) and clone the n64 repo:  
```git clone https://github.com/glankk/n64.git```  
  
If you cd into the n64 repo youll find a script which will install all prerequisite programs for you. However, I've had cases
where the script doesnt find all packages. So I reccomend doing them manually:    
```sudo apt-get install wget```  
```sudo apt-get install tar```  
```sudo apt-get install make```  
```sudo apt-get install diffutils```  
```sudo apt-get install texinfo```  
```sudo apt-get install gcc```  
```sudo apt-get install g++```  
```sudo apt-get install liblua5.3-dev```  
```sudo apt-get install libjansson-dev```  
```sudo apt-get install libusb-1.0-0-dev```  
If you like you can still run the script after to make sure everything is installed:  
```sudo ./install-deps```  
  
Now that all dependencies are installed we can begin configuring and building the toolchain. If you haven't already, cd into the repo and configure the package for install:  
```./configure --prefix=/opt/n64 --enable-vc```  
The prefix is where the toolchain will be installed. The VC flag is also enabled, though as of right now this project will not work on VC.  
  
When it is done configuring you can start make. Glanks instructions reccomend to use the -j flag, however DO NOT use this on WSL. For some reason it is known to cause issues.  
This does mean it will take longer, depending on your computer.  It took a couple of hours for me last I did it:  
```sudo make install-toolchain```  
  
When make is done you can install the included programs:  
```sudo make && make install```  
This won't take as long as the the toolchain.  
  
Lastly install the remaining headers and libraries with:  
```sudo make install-sys```
  
## Armips  

Next we'll install Armips. This program is used for the initial rom editing and dma'ing the payload code created by the toolchain to memory.  
On WSL cd to home or your directory of choice and clone the armips source:  
```git clone --recursive https://github.com/Kingcom/armips.git```  
  
Next youll need to install cmake:
```sudo apt-get install cmake```  
Then, I chose to install Armips in the opt directory alongside the toolchain:  
```cd /opt```  
```sudo mkdir armips```  
```cd armips```  
Then run cmake to build Armips  
```sudo cmake -DCMAKE_BUILD_TYPE=Release ~/path/to/armips/repo```  
```sudo make```  
  
## Add to PATH 
Now that the toolchain and Armips are installed, we need to add them to PATH so that you can run it from anywhere.  
cd to your home directory and open the .profile file:  
```sudo nano .profile```  
then use the arrow keys to scroll to the bottom and add the line:  
```PATH=$PATH:/opt/n64/bin:/opt/armips``` 
ctr+x and enter to exit.  

Then you need to "source" the file so that the changes you made take effect:  
```sudo source .profile```  
At this point you shuold be able to call ```armips``` and have it tell you its usage parameters. If it tells you its not a known command, something went wrong.  
You can also test one of the progams that came with glanks toolchain by calling ```gru```.  

## Running fp  
With all programs installed, we are now ready to actually build the practice rom.  
First clone this project to your location of choice. I personally keep it on the windows side because that is where I do all my editing and emulator testing. I just use WSL to build.  
```cd /mnt/e/<path to fp destination>```  
replace the 'e' with 'c' if you'd like to use your C drive:  
```git clone https://github.com/fig02/fp.git```  

Then back in windows, create a folder in the fp project called "rom".  Place a totally legally obtained Mario Story (J) ROM in this folder and rename it to "base-j.z64".  
Back in WSL, run the following:  
```cd /mnt/e/<path to fp>```  
```make clean all crc```  
This will create an obj and bin folder in the project and output a "fp.z64" file in the rom folder.  
  
A breif explanation of what the make commands do:  
```make```: builds all of the c code in the src folder and compiles it to a .bin file. This gets injected to the rom.  
```clean```: erases the bin and obj folders from the previous build.  
```all```: builds the obj and binary files to be appeneded to the rom. Also runs armips to patch the rom with the asm to dma the payload to ram at run time.     
```crc```: uses glanks lua tool called gru to update the rom crc. Since armips will dynamically allocate space for the binary depending on its size, the dma instructions change on each build so the crc needs to update accordingly.  

