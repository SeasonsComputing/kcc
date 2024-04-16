include ../make.properties

SRC=$(KCC_TST)/zip
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/zip.o
TARGET= \
	$(BIN)/zip

all: $(TARGET)

$(OBJ)/zip.o: $(SRC)/zip.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
