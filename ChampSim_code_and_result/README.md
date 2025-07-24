## Setup
1. Install Docker in your Ubuntu machine </br>
2. Setup container using the following commands </br>
a) Download Docker image (leakyrand_image.tar) from Zenodo link https://doi.org/10.5281/zenodo.16237834 </br>
b) Import .tar file as docker image: </br>
docker import leakyrand_image.tar cases:leakyrand  </br>
c) Check the added image: </br> 
sudo docker images  </br>
d) Run the container  </br> 
sudo docker run -it cases:leakyrand /bin/bash </br>
3. Download code inside container  </br>
cd home/yashikav/Desktop/Mirage_project  </br>
git clone https://github.com/YashikaVerma156/LeakyRand_CASES_2025.git  </br>
4. To overcome the path dependencies in the ChampSim code, run the below command </br>
cp -r LeakyRand_CASES_2025/ChampSim_code_and_result/*  . </br>

## Running simulations and generating respective plots
1. Refer to directories, for that particular figure in the paper, for example, fig11/ contains scripts to run simulations and generate plot for figure 11 in the paper. </br>
2. To run the simulation and generate plot, run ./script.sh in each fig*/  </br>
3. Refer to fig*/README.txt for instructions.
