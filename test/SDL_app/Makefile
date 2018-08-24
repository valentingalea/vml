# mini help
# $@ is the name of the file to be made.
# $? is the names of the changed dependents.
# $< the name of the related file that caused the action.
# $* the prefix shared by target and dependent files.

all: app

CXXFLAGS += -std=c++17 -DC4DROID -fsingle-precision-constant
#-Ofast -march=native -funroll-loops

LIBS += -lSDL

OUT = app

app: app.o
	$(CXX) $(CXXFLAGS) -o $(OUT) app.o $(LIBS)
app.o : SDL_app.cpp SDL_app.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o app.o $<

clean:
	rm *.o
	rm $(OUT)