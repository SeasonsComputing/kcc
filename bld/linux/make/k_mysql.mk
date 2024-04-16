include make.properties

SRC=$(KCC_SRC)/store
OBJ=$(KCC_OBJ)
BIN=$(KCC_BIN)
OBJFILES= \
	$(OBJ)/MySql.o
TARGET=$(BIN)/libk_mysql.so

default: compile

compile: $(TARGET)

$(OBJ)/MySql.o: $(SRC)/MySql.cpp
	g++ -c $(COMPILE_OPTIONS) -I $(MYSQLPP_INC) $< -o $@

$(TARGET).1: $(OBJFILES)
	g++ $(LINK_OPTIONS) -shared -Wl \
	-o $(TARGET).1 \
	$(OBJFILES) \
	-L /usr/lib64 -L $(MYSQLPP_LIB) -L $(MYSQLPP_LIB64) \
	-lc -lmysqlclient -lz -lk_core

$(TARGET): $(TARGET).1
	ln -f -s $(TARGET).1 $(TARGET)

clean:
	rm -f $(OBJFILES)
	rm -f $(TARGET)
	rm -f $(TARGET).1
