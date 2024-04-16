include ../make.properties

SRC=$(KCC_TST)/socket
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/client.o
TARGET= \
	$(BIN)/client

default: compile

compile: $(TARGET)

$(OBJ)/client.o: $(SRC)/client.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
