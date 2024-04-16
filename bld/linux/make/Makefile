default: compile

make.properties:
	cp make-env-vars.txt make.properties
	-echo "Remember to update make.properties"

include make.properties

all: deploy

compile:
	make -k -f KCC.mk

deploy:
	make -k -f KCC.mk

build:
	make -k -f KCC.mk clean
	make -k -f KCC.mk

test:
	make -k -f KCC.mk test

clean:
	make -k -f KCC.mk clean
