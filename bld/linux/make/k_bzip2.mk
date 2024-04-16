include make.properties

SRC=$(KCC_SRC)/zip
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/BZip2.o
TARGET= \
	$(BIN)/libk_bzip2.so

default: compile

compile: $(TARGET)

$(OBJ)/BZip2.o: $(SRC)/BZip2.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET).1: $(OBJFILES)
	g++ $(LINK_OPTIONS) -shared -Wl \
	-o $(TARGET).1 $(OBJFILES) \
	-lbz2 -lc -lk_core

$(TARGET): $(TARGET).1
	ln -f -s $(TARGET).1 $(TARGET)

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
	rm -f $(TARGET).1

