include make.properties

SRC=$(KCC_SRC)/rodom
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/DOM.o \
	$(OBJ)/RODOM.o
TARGET=$(BIN)/libk_rodom.so

default: compile

compile: $(TARGET)

$(OBJ)/RODOM.o: $(SRC)/RODOM.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/DOM.o: $(SRC)/DOM.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

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
