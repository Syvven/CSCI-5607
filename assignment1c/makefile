CC = g++
CFLAGS = -lpthread -g -std=c++11
DEPS = inputs/RayTracer.h
OBJ = src/RayTracer.o src/ray_tracing_main.o

TXT = kiwer.txt

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raytracer1c: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run:
	./raytracer1c $(TXT)

comp-run: clean raytracer1c
	./raytracer1c $(TXT)
	
clean:
	rm -f gen src/*.o
