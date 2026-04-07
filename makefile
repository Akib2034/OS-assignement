CC = gcc
CFLAGS = -Wall -pthread -fsanitize=thread -g
OBJ = mssv.o
EXEC = mssv

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

mssv.o: mssv.c
	$(CC) $(CFLAGS) -c mssv.c

clean:
	rm -f $(EXEC) $(OBJ)
