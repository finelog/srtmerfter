CC    = gcc
bin   = $(PWD)/srtmerfter
runid = $(shell id -u)

default: srtmerfter
	$(CC) -o srtmerfter srtmerfter.c
debug: srtmerfter
	$(CC) -g -o srtmerfter srtmerfter.c

.PHONY: install clean
install: 
ifeq ($(runid),0)
	cp -u $(bin) /usr/local/bin/
else
	@echo 'please run make install as a root user'
endif
	@echo 'All things done'
clean:
	rm srtmerfter
