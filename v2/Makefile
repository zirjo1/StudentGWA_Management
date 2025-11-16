CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I.

OBJ = main.o studentfunc.o
OUT = student_app

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

main.o: main.c student.h
	$(CC) $(CFLAGS) -c main.c

studentfunc.o: studentfunc.c student.h
	$(CC) $(CFLAGS) -c studentfunc.c

clean:
	rm -f $(OBJ) $(OUT)
