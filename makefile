CC = g++
GCC_OPTIONS=-Wall -pedantic -I../include  -Wno-deprecated-declarations
GL_OPTIONS=-framework OpenGL -framework GLUT
COPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)
DEPS = parser.h init.h classes.h
OBJ = parser.o init.o initShader.o assn6.o

all: prog

%.o: %.cpp $(DEPS)
	$(CC) $(GCC_OPTIONS) -c -o $@ $<

prog: $(OBJ)
	$(CC) $(GL_OPTIONS) -o $@ $^

clean:
	rm *.o
	rm prog