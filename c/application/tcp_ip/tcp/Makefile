SOURCE=$(wildcard *.c)  
PROGS=$(patsubst %.c,%,$(SOURCE))  
temp = $(wildcard *~)  
all:$(PROGS)  
  
clean:  
	rm $(temp) $(PROGS)  
