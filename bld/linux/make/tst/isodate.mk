include ../make.properties

SRC=$(KCC_TST)/isodate
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/isodate.o
TARGET= \
	$(BIN)/isodate

default: compile

compile: $(TARGET)

$(OBJ)/isodate.o: $(SRC)/isodate.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
