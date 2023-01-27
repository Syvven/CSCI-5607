CC = g++
CFLAGS = -g -std=c++11 
DEPS = utils.h vec3.h
OBJ = ray_casting_main.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cast: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run:
	./cast input.txt

cast-run: cast 
	./cast input.txt
	

clean:
	rm -f gen *.o