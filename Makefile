CPPFLAGS = -I/usr/X11R6/include -DX11 -g
LDLIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -lopencv_core -lopencv_highgui -lopencv_imgproc -lXmu -lXi -lXext -lX11 -lpthread -lm
TARGETS = cvtest
OBJECTS = main.o gg.o matrix.o

$(TARGETS): $(OBJECTS)
	$(CXX) $(CFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

main.o:	gg.h matrix.h
clean:
	-rm -f $(TARGETS) *.o *~
