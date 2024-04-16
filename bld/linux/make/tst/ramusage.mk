include ../make.properties

SRC=$(KCC_TST)/platform
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/ramusage.o
TARGET= \
	$(BIN)/ramusage

all: $(TARGET)

$(OBJ)/ramusage.o: $(SRC)/ramusage.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
