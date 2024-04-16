include ../make.properties

SRC=$(KCC_TST)/socket
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/server.o
TARGET= \
	$(BIN)/server

default: compile

compile: $(TARGET)

$(OBJ)/server.o: $(SRC)/server.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
