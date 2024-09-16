CC        = gcc
CFLAGS    = -Wall -O3 -lm #-lblas -llapack
OPTFLAGS  = -fopenmp 
LDFLAGS   = 
TARGET    = ../run/OMPInitDeconvolution

.SUFFIXES:
.SUFFIXES: .o .c

.c.o:
	$(CC) -c $(CFLAGS) $(OPTFLAGS) $< -o $@

OBJS =\
	input.o  \
	func.o   \
	program.o
	
HEADERS =\
	input.h\
	func.h \


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADERS)

clean:
	rm -f *.o $(TARGET) *~ *.log


OMPInitDeconvolutionOld: ProgramOMP_init.c
	gcc -Wall -O3 -fopenmp ProgramOMP_init.c -o ../run/OMPInitDeconvolutionOld -lm  



#test:
#	$(CC) -lm -lblas -llapack -o a.out test.c input.c 
		

#2DKBmirrors: program.c
#	gcc -O3 Program.c -o ../2DKBmirrors
