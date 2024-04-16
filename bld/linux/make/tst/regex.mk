include ../make.properties

SRC=$(KCC_TST)/regex
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/regex.o
TARGET= \
	$(BIN)/regex

default: compile

compile: $(TARGET)

$(OBJ)/regex.o: $(SRC)/regex.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
