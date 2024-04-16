include ../make.properties

SRC=$(KCC_TST)/textquery
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/textquery.o
TARGET= \
	$(BIN)/textquery

default: compile

compile: $(TARGET)

$(OBJ)/textquery.o: $(SRC)/textquery.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
