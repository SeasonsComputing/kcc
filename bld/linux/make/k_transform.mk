include make.properties

INC=$(KCC_INC)/xml
SRC=$(KCC_SRC)/xml
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/XMLTransform.o
TARGET=$(BIN)/libk_transform.so

default: compile

compile: $(TARGET)

$(OBJ)/XMLTransform.o: $(SRC)/XMLTransform.cpp $(INC)/IXMLTransform.h
	g++ -c $(COMPILE_OPTIONS) -I $(LIBXML_INC) -I $(LIBXSLT_INC) $< -o $@

$(TARGET).1: $(OBJFILES)
	g++ $(LINK_OPTIONS) -shared -Wl,-soname,$(TARGET).1 \
	-o $(TARGET).1 \
	$(OBJFILES) \
	-lc -lxml2 -lxslt -lexslt -lz -lk_core

$(TARGET): $(TARGET).1
	ln -f -s $(TARGET).1 $(TARGET)

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
	rm -f $(TARGET).1

