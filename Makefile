OBJ = ObjectManager.o
CC = clang++
CCFLAGS = -Wall
INCLUDE = ObjectManager.o ObjectManager.h Object.h  

TESTOBJ = main1 main2 main3 main4 main5 #was used for make compilation and clean

all: $(OBJ) main 

OBJ: ObjectManager.c ObjectManager.h
	$(CC) $(CCFLAGS) ObjectManager.c -o $(OBJ)

main: ObjectManager.o ObjectManager.h main.c
	$(CC) $(CCFLAGS) ObjectManager.o main.c -o main

#Was used for testing the additional test suites provided, left here in case of marking purposes
# main1: $(INCLUDE) main1.c
# 	$(CC) $(CCFLAGS) ObjectManager.o main1.c -o main1

# main2: $(INCLUDE) main2.c
# 	$(CC) $(CCFLAGS) ObjectManager.o main2.c -o main2

# main3: $(INCLUDE) main3.c
# 	$(CC) $(CCFLAGS) ObjectManager.o main3.c -o main3

# main4: $(INCLUDE) main4.c
# 	$(CC) $(CCFLAGS) ObjectManager.o main4.c -o main4

# main5: $(INCLUDE) main5.c
# 	$(CC) $(CCFLAGS) ObjectManager.o main5.c -o main5

clean:
	rm $(OBJ) main 