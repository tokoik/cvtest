CXXFLAGS	= -I/usr/X11R6/include -I/opt/local/include -DX11 -Wall -g
LDLIBS = -L/usr/X11R6/lib -lglut -lGLU -lGL -L/opt/local/lib -lopencv_core -lopencv_highgui -lm
OBJECTS	= $(patsubst %.cpp,%.o,$(wildcard *.cpp))
TARGET = cvtest

.PHONY: clean depend

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-$(RM) $(TARGET) *.o *~ .*~ core

depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > $(TARGET).dep

-include $(wildcard *.dep)
