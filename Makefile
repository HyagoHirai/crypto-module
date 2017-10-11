obj-m += cryptoMOD.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

userProg: userProg.o
	g++ -o testMakef userProg.o
