include ../make.properties

SRC=$(KCC_TST)/idhash
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/idhash.o
TARGET= \
	$(BIN)/idhash

default: compile

compile: $(TARGET)

$(OBJ)/idhash.o: $(SRC)/idhash.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
