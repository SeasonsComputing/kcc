include ../make.properties

SRC=$(KCC_TST)/thread
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/thread.o
TARGET= \
	$(BIN)/thread

all: $(TARGET)

$(OBJ)/thread.o: $(SRC)/thread.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
