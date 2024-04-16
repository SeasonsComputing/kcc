include make.properties

SRC=$(KCC_SRC)/tools
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/xform.o
TARGET=$(BIN)/xform

default: compile

compile: $(TARGET)

$(OBJ)/xform.o: $(SRC)/xform.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
