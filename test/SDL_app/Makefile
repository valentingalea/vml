# mini help
# $@ is the name of the file to be made.
# $? is the names of the changed dependents.
# $< the name of the related file that caused the action.
# $* the prefix shared by target and dependent files.

all: app

APP = #-DAPP_PLANET
SIZE = -DSCR_W8=240 -DSCR_H8=240

CXXFLAGS += -std=c++17 -DC4DROID -fsingle-precision-constant
#CXXFLAGS += -Ofast -march=native -funroll-loops

LIBS += -lSDL
OUT = app

app: app.o
	$(CXX) $(CXXFLAGS) -o $(OUT) app.o $(LIBS)
app.o : SDL_app.cpp SDL_app.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(APP) $(SIZE) -c -o app.o $<

clean:
	rm *.o
	rm $(OUT)