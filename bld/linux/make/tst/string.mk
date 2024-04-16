include ../make.properties

SRC=$(KCC_TST)/string
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/string.o
TARGET= \
	$(BIN)/string

default: compile

compile: $(TARGET)

$(OBJ)/string.o: $(SRC)/string.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
