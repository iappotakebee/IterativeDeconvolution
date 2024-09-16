#env OMP_NUM_THREADS=4 ./OMPDeconvolution | tee a.log
#env OMP_NUM_THREADS=4 ./OMPDeconvolution 
#env OMP_NUM_THREADS=6 ./OMPInitDeconvolution | tee ./files/tmp/a.log
env OMP_NUM_THREADS=6 OMP_STACKSIZE="10G" ./OMPInitDeconvolution
