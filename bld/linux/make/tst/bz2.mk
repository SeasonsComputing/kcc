include ../make.properties

SRC=$(KCC_TST)/bz2
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/bz2.o
TARGET= \
	$(BIN)/bz2

all: $(TARGET)

$(OBJ)/bz2.o: $(SRC)/bz2.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
