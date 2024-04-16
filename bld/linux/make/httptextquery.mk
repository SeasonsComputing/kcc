include make.properties

SRC=$(KCC_SRC)/store
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/httptextquery.o
TARGET=$(BIN)/httptextquery

default: compile

compile: $(TARGET)

$(OBJ)/httptextquery.o: $(SRC)/httptextquery.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
