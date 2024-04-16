include ../make.properties

SRC=$(KCC_TST)/sql
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/sql.o
TARGET= \
	$(BIN)/sql
	
default: compile

compile: $(TARGET)

$(OBJ)/sql.o: $(SRC)/sql.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET): $(OBJFILES)
	g++ $(LINK_OPTIONS) -o $(TARGET) $(OBJFILES) -lk_core

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
