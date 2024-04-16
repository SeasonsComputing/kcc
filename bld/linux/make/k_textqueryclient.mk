include make.properties

SRC=$(KCC_SRC)/store
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/TextQueryClient.o
TARGET=$(BIN)/libk_textqueryclient.so

default: compile

compile: $(TARGET)

$(OBJ)/TextQueryClient.o: $(SRC)/TextQueryClient.cpp
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

