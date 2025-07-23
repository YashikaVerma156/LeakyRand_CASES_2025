1. Install Docker in your Ubuntu machine </br>
2. Setup container using the following commands </br>
a) Download Docker image (leakyrand_image.tar) from Zenodo link 10.5281/zenodo.16237834  </br>
b) Import .tar file as docker image: </br>
docker import leakyrand_image.tar cases:leakyrand  </br>
c) Check the added image: </br> 
sudo docker images  </br>
d) RUn the container  </br> 
sudo docker run -it cases:leakyrand /bin/bash </br>
3. Download code inside container  </br>
git clone 
