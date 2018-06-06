TARGET = gpio_key_v2
SRC = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRC))
LIBS = -lpthread
.PHONY: all
all: $(TARGET)
$(TARGET) : $(OBJS)
	#test -d ../../bin || mkdir ../../bin
	$(CC) $^ $(LIBS) -o $@
$(OBJS):%.o:%.c
	$(CC) -c $< -o $@ 
clean:
	$(RM) *.o $(TARGET)