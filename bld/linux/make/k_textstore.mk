include make.properties

SRC=$(KCC_SRC)/store
INC=-I $(KCC_SRC)/store -I $(KCC_SRC)/store/clucene.0.9.10 -I $(KCC_SRC)/store/clucene.0.9.10/CLucene
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
CL_COMPILE_OPTIONS=-D_ASCII
OBJFILES= \
	$(OBJ)/TextStore.o
TARGET=$(BIN)/libk_textstore.so

default: compile

compile: $(TARGET)

$(OBJ)/TextStore.o: $(SRC)/TextStore.cpp
	g++ -c $(COMPILE_OPTIONS) $(CL_COMPILE_OPTIONS) $(INC) $< -o $@

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
