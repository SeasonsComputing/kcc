include ../make.properties

SRC=$(KCC_TST)/httpd
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/httpd.o
TARGET= \
	$(BIN)/httpd

default: compile

compile: $(TARGET)

$(OBJ)/httpd.o: $(SRC)/httpd.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
