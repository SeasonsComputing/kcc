include ../make.properties

SRC=$(KCC_TST)/dom
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/dom.o
TARGET= \
	$(BIN)/dom

default: compile

compile: $(TARGET)

$(OBJ)/dom.o: $(SRC)/dom.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
