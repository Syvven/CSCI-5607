CC = g++
CFLAGS = -lpthread -g -std=c++11
DEPS = inputs/RayTracer.h
OBJ = src/RayTracer.o src/ray_tracing_main.o

TXT = kiwer.txt

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

raytracer1b: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run:
	./raytracer1b $(TXT)

comp-run: clean raytracer1b
	./raytracer1b $(TXT)
	
clean:
	rm -f gen src/*.o

test:
	./raytracer1b inputs/base.txt 
	./raytracer1b inputs/cylindertest.txt 
	./raytracer1b inputs/grasskiwi.txt 
	./raytracer1b inputs/kiwer.txt 
	./raytracer1b inputs/shadow.txt 
	./raytracer1b inputs/simplekiwi.txt 
	./raytracer1b inputs/tests1.txt 
	./raytracer1b inputs/varyn1.txt 