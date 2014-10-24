GCC_OPTIONS=-Wall -pedantic -Iinclude -g -Wno-deprecated
GL_OPTIONS=-framework OpenGL -framework GLUT 
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

all: terrain

InitShader.o: InitShader.cpp
	g++ -c InitShader.cpp $(GCC_OPTIONS)

mesh.o: mesh.cpp mesh.h
	g++ -c mesh.cpp $(GCC_OPTIONS)

terrain.o: terrain.cpp
	g++ -c terrain.cpp $(GCC_OPTIONS)

terrain: InitShader.o mesh.o terrain.o
	g++ $^ $(OPTIONS) -o $@  

clean:
	rm -rf *.o *~ *.dSYM terrain
