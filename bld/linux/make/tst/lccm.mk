include ../make.properties

SRC=$(KCC_TST)/lccm
OBJ=$(KCC_TST_OBJ)
BIN=$(KCC_BIN)
OBJFILES1= \
	$(OBJ)/lccm.o
OBJFILES2= \
	$(OBJ)/SimpleComponentImpl.o
OBJFILES3= \
	$(OBJ)/MyComponentImpl.o \
	$(OBJ)/MyComponentImplEx.o \
	$(OBJ)/MyComponentFactoryImpl.o
TARGET1= \
	$(BIN)/lccm
TARGET2= \
	$(BIN)/libk_simplecomponent.so
TARGET3= \
	$(BIN)/libk_mycomponent.so

default: compile

compile: $(TARGET1)

$(OBJ)/lccm.o: $(SRC)/lccm.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/SimpleComponentImpl.o: $(SRC)/simplecomponent/SimpleComponentImpl.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/MyComponentFactoryImpl.o: $(SRC)/mycomponent/MyComponentFactoryImpl.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/MyComponentImpl.o: $(SRC)/mycomponent/MyComponentImpl.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(OBJ)/MyComponentImplEx.o: $(SRC)/mycomponent/MyComponentImplEx.cpp
	g++ -c $(COMPILE_OPTIONS) $< -o $@

$(TARGET2).1: $(OBJFILES2)
	gcc $(LINK_OPTIONS) -shared -Wl,-soname,libk_simplecomponent.so.1 \
	-o $(TARGET2).1 \
	$(OBJFILES2) \
	-lc -lk_core

$(TARGET2): $(TARGET2).1
	ln -f -s $(TARGET2).1 $(TARGET2)

$(TARGET3).1: $(OBJFILES3)
	gcc $(LINK_OPTIONS) -shared -Wl,-soname,libk_mycomponent.so.1 \
	-o $(TARGET3).1 \
	$(OBJFILES3) \
	-lc -lk_core

$(TARGET3): $(TARGET3).1 
	ln -f -s $(TARGET3).1 $(TARGET3) 

$(TARGET1): $(OBJFILES1) $(TARGET2) $(TARGET3)
	g++ $(LINK_OPTIONS) -o $(TARGET1) $(OBJFILES1) -lk_core

clean:
	rm -f $(OBJFILES1)
	rm -f $(OBJFILES2)
	rm -f $(OBJFILES3)
	rm -f $(TARGET3)
	rm -f $(TARGET3).1
	rm -f $(TARGET2)
	rm -f $(TARGET2).1
	rm -f $(TARGET1)
