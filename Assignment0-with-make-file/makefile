CC = g++
CFLAGS = -g -std=c++11 
DEPS = SPH.h vec2.h utils.h Voronoi.h PerlinNoise.h
OBJ = SPH.o Voronoi.o PerlinNoise.o ppm_main.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gen: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run:
	./gen input_test.txt

com-run: gen 
	./gen input_test.txt

clean:
	rm -f gen *.o

