CC = g++
CFLAGS = -lpthread -g -std=c++11 -O3
DEPS = inputs/RayTracer.h
OBJ = src/RayTracer.o src/ray_tracing_main.o

TXT = kiwer.txt

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raytracer1d: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run:
	./raytracer1d $(TXT)

comp-run: clean raytracer1d
	./raytracer1d $(TXT)
	
clean:
	rm -f gen src/*.o
