include ../make.properties

SRC=$(KCC_TST)/timer
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/timer.o
TARGET= \
	$(BIN)/timer

all: $(TARGET)

$(OBJ)/timer.o: $(SRC)/timer.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
