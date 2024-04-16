default: compile

make.properties:
	cp make-env-vars.txt make.properties
	-echo "Remember to update make.properties"

include make.properties

all: build

build: clean compile

compile: make.properties
	make -k -f k_core.mk compile
	make -k -f k_textqueryclient.mk compile
	make -k -f k_mysql.mk compile
	make -k -f k_textstore.mk compile
	make -k -f k_bzip2.mk compile
	make -k -f k_zlib.mk compile
	make -k -f k_httpserver.mk compile
	make -k -f k_regex.mk compile
	make -k -f k_rodom.mk compile
	make -k -f k_pageresponse.mk compile
	make -k -f k_transform.mk compile
	make -k -f k_exist.mk compile
	make -k -f httptextquery.mk compile
	make -k -f codegen.mk compile

clean: make.properties
	make -k -f k_core.mk clean
	make -k -f k_textqueryclient.mk clean
	make -k -f k_mysql.mk clean
	make -k -f k_textstore.mk clean
	make -k -f k_bzip2.mk clean
	make -k -f k_zlib.mk clean
	make -k -f k_httpserver.mk clean
	make -k -f k_regex.mk clean
	make -k -f k_rodom.mk clean
	make -k -f k_pageresponse.mk clean
	make -k -f k_transform.mk clean
	make -k -f k_exist.mk clean
	make -k -f httptextquery.mk clean
	make -k -f codegen.mk clean
	make -k -C tst clean

cleanall:
	rm -f $(KCC_BIN)/*.so
	rm -f $(KCC_BIN)/*.so.*
	rm -f $(KCC_OBJ)/*.o
	rm -f $(KCC_TST_OBJ)/*.o
	# remove the excutables from KCC_BIN
	rm -f `find $(KCC_BIN) -type f -perm -1 -maxdepth 1 -print`
	# remove the excutables from KCC_TST_OBJ
	rm -f `find $(KCC_TST_OBJ) -type f -perm -1 -maxdepth 1 -print`

test: make.properties
	make -k -C tst
