# mini help
# $@ is the name of the file to be made.
# $? is the names of the changed dependents.
# $< the name of the related file that caused the action.
# $* the prefix shared by target and dependent files.

all: app

CXXFLAGS += -std=c++17 -DC4DROID
#-Ofast -march=native -funroll-loops
LFLAGS += -lSDL2_image -lSDL2_net -ltiff -ljpeg -lpng -lz -lSDL2_ttf -lfreetype -lSDL2_mixer -lSDL2_test -lsmpeg2 -lvorbisfile -lvorbis -logg -lstdc++ -lSDL2 -lEGL -lGLESv1_CM -lGLESv2 -landroid -Wl,--no-undefined -shared

OUT = app
SIZE = -DSCR_W8=100 -DSCR_H8=100

app: app.o
	$(CXX) $(CXXFLAGS) -o $(OUT) app.o $(LFLAGS) $(LIBS)
app.o : SDL2_app.cpp SDL2_app.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SIZE) -c -o app.o $<

clean:
	rm *.o
	rm $(OUT)