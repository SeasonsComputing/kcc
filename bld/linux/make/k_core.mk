include make.properties

SRC=$(KCC_SRC)/core
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/Core.o            \
	$(OBJ)/ComponentModule.o \
	$(OBJ)/Components.o      \
	$(OBJ)/Dictionary.o      \
	$(OBJ)/DOMReader.o       \
	$(OBJ)/DOMWriter.o       \
	$(OBJ)/Exception.o       \
	$(OBJ)/HTTP.o            \
	$(OBJ)/ISODate.o         \
	$(OBJ)/Log.o             \
	$(OBJ)/MD5.o             \
	$(OBJ)/Platform.o        \
	$(OBJ)/Properties.o      \
	$(OBJ)/Socket.o          \
	$(OBJ)/Strings.o         \
	$(OBJ)/Thread.o          \
	$(OBJ)/URL.o             \
	$(OBJ)/UUID.o
TARGET=$(BIN)/libk_core.so

default: compile

compile: $(TARGET)

$(OBJ)/ComponentModule.o: $(SRC)/ComponentModule.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Components.o: $(SRC)/Components.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Core.o: $(SRC)/Core.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Dictionary.o: $(SRC)/Dictionary.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/DOMReader.o: $(SRC)/DOMReader.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/DOMWriter.o: $(SRC)/DOMWriter.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Exception.o: $(SRC)/Exception.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/HTTP.o: $(SRC)/HTTP.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/ISODate.o: $(SRC)/ISODate.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Log.o: $(SRC)/Log.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/MD5.o: $(SRC)/MD5.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Platform.o: $(SRC)/Platform.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Properties.o: $(SRC)/Properties.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Socket.o: $(SRC)/Socket.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Strings.o: $(SRC)/Strings.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/Thread.o: $(SRC)/Thread.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/URL.o: $(SRC)/URL.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/UUID.o: $(SRC)/UUID.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(BIN)/libk_core.so.1: $(OBJFILES)
	g++ $(LINK_OPTIONS) -shared -Wl \
	-o $(TARGET).1 $(OBJFILES) \
	-lc -ldl -luuid -lpthread

$(TARGET): $(TARGET).1
	ln -f -s $(TARGET).1 $(TARGET)

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
	rm -f $(TARGET).1
