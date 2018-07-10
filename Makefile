CC = g++
CFLAGS = -Wall
mainfile: $(OBJ)
	$(CC) $(CFLAGS) mainfile.cpp -o $@ $^

