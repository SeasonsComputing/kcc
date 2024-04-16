include ../make.properties

SRC=$(KCC_TST)/page
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/page.o
TARGET= \
	$(BIN)/page

default: compile

compile: $(TARGET)

$(OBJ)/page.o: $(SRC)/page.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
