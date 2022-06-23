# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
#GCCDIR = ~/gnu/local/decstation-ultrix/bin/
GCCDIR = /usr/local/nachos/decstation-ultrix/bin/
LDFLAGS = -T script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = /lib/cpp
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -mips1

all: halt shell matmult sort cat cp echo fibo25 loop ro lp matmult2 sort2 vs

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

shell.o: shell.c utils.h
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o
	$(LD) $(LDFLAGS) start.o shell.o -o shell.coff
	../bin/coff2noff shell.coff shell

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

sort2.o: sort2.c
	$(CC) $(CFLAGS) -c sort2.c
sort2: sort2.o start.o
	$(LD) $(LDFLAGS) start.o sort2.o -o sort2.coff
	../bin/coff2noff sort2.coff sort2

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

matmult2.o: matmult2.c
	$(CC) $(CFLAGS) -c matmult2.c
matmult2: matmult2.o start.o
	$(LD) $(LDFLAGS) start.o matmult2.o -o matmult2.coff
	../bin/coff2noff matmult2.coff matmult2

cat.o: cat.c utils.h
	$(CC) $(CFLAGS) -c cat.c
cat: cat.o start.o
	$(LD) $(LDFLAGS) start.o cat.o -o cat.coff
	../bin/coff2noff cat.coff cat

cp.o: cp.c utils.h
	$(CC) $(CFLAGS) -c cp.c
cp: cp.o start.o
	$(LD) $(LDFLAGS) start.o cp.o -o cp.coff
	../bin/coff2noff cp.coff cp

echo.o: echo.c utils.h
	$(CC) $(CFLAGS) -c echo.c
echo: echo.o start.o
	$(LD) $(LDFLAGS) start.o echo.o -o echo.coff
	../bin/coff2noff echo.coff echo

fibo25.o: fibo25.c utils.h
	$(CC) $(CFLAGS) -c fibo25.c
fibo25: fibo25.o start.o
	$(LD) $(LDFLAGS) start.o fibo25.o -o fibo25.coff
	../bin/coff2noff fibo25.coff fibo25

loop.o: loop.c
	$(CC) $(CFLAGS) -c loop.c
loop: loop.o start.o
	$(LD) $(LDFLAGS) start.o loop.o -o loop.coff
	../bin/coff2noff loop.coff loop

ro.o: ro.c
	$(CC) $(CFLAGS) -c ro.c
ro: ro.o start.o
	$(LD) $(LDFLAGS) start.o ro.o -o ro.coff
	../bin/coff2noff ro.coff ro

lp.o: lp.c utils.h
	$(CC) $(CFLAGS) -c lp.c
lp: lp.o start.o
	$(LD) $(LDFLAGS) start.o lp.o -o lp.coff
	../bin/coff2noff lp.coff lp

vs.o: vs.c utils.h
	$(CC) $(CFLAGS) -c vs.c
vs: vs.o start.o
	$(LD) $(LDFLAGS) start.o vs.o -o vs.coff
	../bin/coff2noff vs.coff vs


# Estas reglas sirven para compilar programas simples,
# que consistan en un único fuente.
# Las reglas anteriores para construir los ejecutables
# halt, shell, sort y matmult se podrían suprimir

%: %.c 
	make start.o 
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) start.o $*.o -o $*.coff
	../bin/coff2noff $*.coff $@
	

