all: SerialDeconvolution OMPDeconvolution OMPInitDeconvolution
SerialDeconvolution: ProgramSerial.c
	gcc -Wall -O3 ProgramSerial.c -o SerialDeconvolution
OMPDeconvolution: ProgramOMP.c
	gcc -Wall -O3 -fopenmp ProgramOMP.c -o OMPDeconvolution -lm
OMPInitDeconvolution: ProgramOMP_init.c
	gcc -Wall -O3 -fopenmp ProgramOMP_init.c -o OMPInitDeconvolution -lm  

