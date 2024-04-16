include ../make.properties

SRC=$(KCC_TST)/platform
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/platform.o
TARGET= \
	$(BIN)/platform

all: $(TARGET)

$(OBJ)/platform.o: $(SRC)/platform.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
