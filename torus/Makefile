all: torus

CC=g++
CPPFLAGS=-Wall -g --std=gnu++11
LDFLAGS=-lglfw -lGL -lGLEW -lpng

torus: torus.o shader.o controls.o png_texture.o save_image.o object.o


clean:
	rm -f *.o

clobber: clean
	rm -f torus
