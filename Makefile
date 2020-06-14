SOURCES=$(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJECTS=$(patsubst src/%,build/%,${SOURCES:.cpp=.o})
DIRS=$(filter-out build/,$(sort $(dir ${OBJECTS})))

CXX=g++
LD_FLAGS=
CXX_FLAGS=-std=c++17 -I./include

NAME=chasm
BINARY=out/${NAME}

ifdef DEBUG
CXX_FLAGS += -g -DDEBUG
endif
ifndef DEBUG
CXX_FLAGS += -O2
endif

all: dirs ${BINARY}

${BINARY}: ${OBJECTS}
	${CXX} $^ ${LD_FLAGS} -o $@

build/%.o: src/%.cpp
	${CXX} $< ${CXX_FLAGS} -c -o $@

.PHONY: dirs
dirs:
	mkdir -p ${DIRS}
	mkdir -p out/

.PHONY: clean
clean:
	-rm -r build/
	-rm -r out/

.PHONY: install
install:
	cp -r ${BINARY} /usr/local/bin
	mkdir -p /usr/local/share/${NAME}
	cp -r data/* /usr/local/share/${NAME}
	cp app.desktop /usr/local/share/applications/${NAME}.desktop
	cp icon.png /usr/local/share/icons/${NAME}.png

.PHONY: uninstall
uninstall:
	rm /usr/local/bin/${NAME}
	rm -r /usr/local/share/${NAME}
	rm /usr/local/share/applications/${NAME}.desktop
	rm /usr/local/share/icons/${NAME}.png
