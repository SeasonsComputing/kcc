include ../make.properties

SRC=$(KCC_TST)/xform
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/xform.o
TARGET= \
	$(BIN)/xform

all: $(TARGET)

$(OBJ)/xform.o: $(SRC)/xform.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
