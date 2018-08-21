# mini help
# $@ is the name of the file to be made.
# $? is the names of the changed dependents.
# $< the name of the related file that caused the action.
# $* the prefix shared by target and dependent files.

all: app

CXXFLAGS += -std=c++17 -DC4DROID
#-Ofast -march=native -funroll-loops
LFLAGS += -lSDL_image -lSDL_net -ltiff -ljpeg -lpng -lz -lSDL_ttf -lfreetype -lSDL_mixer -lsmpeg -lvorbisfile -lvorbis -logg -lstdc++ -lSDL 

OUT = app
SIZE = -DSCR_W8=100 -DSCR_H8=100

app: app.o
	$(CXX) $(CXXFLAGS) -o $(OUT) app.o $(LFLAGS) $(LIBS)
app.o : SDL_app.cpp SDL_app.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SIZE) -c -o app.o $<

clean:
	rm *.o
	rm $(OUT)