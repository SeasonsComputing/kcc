include make.properties

SRC=$(KCC_SRC)/regex
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/Regex.o
TARGET=$(BIN)/libk_regex.so

default: compile

compile: $(TARGET)

$(OBJ)/Regex.o: $(SRC)/Regex.cpp
	g++ -c $(COMPILE_OPTIONS) -I $(SRC) $< -o $@

$(TARGET).1: $(OBJFILES)
	g++ $(LINK_OPTIONS) -shared -Wl \
	-o $(TARGET).1 \
	$(OBJFILES) \
	-lc -lk_core

$(TARGET): $(TARGET).1
	ln -f -s $(TARGET).1 $(TARGET)

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
	rm -f $(TARGET).1

