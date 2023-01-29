CC = g++
CFLAGS = -g -std=c++11
DEPS = RayTracer.h
OBJ = RayTracer.o ray_tracing_main.o

TXT = input.txt

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cast: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run:
	./cast $(TXT)

cast-run: clean cast 
	./cast $(TXT)
	
clean:
	rm -f gen *.o