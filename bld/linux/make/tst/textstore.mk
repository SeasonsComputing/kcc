include ../make.properties

SRC=$(KCC_TST)/textstore
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/textstore.o
TARGET= \
	$(BIN)/textstore

default: compile

compile: $(TARGET)

$(OBJ)/textstore.o: $(SRC)/textstore.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
