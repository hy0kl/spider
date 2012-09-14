LIBDIR = \
		-llua \
		-lcrypto \
		-lsqlite3 \
		-lm

GCC = gcc
GCCFLAGS = -g -finline-functions -Wall -Winline -pipe

TARGET = spider

OBJS1  = spider.o util.o

all : $(TARGET)
	@echo "Start compile all"
	rm -f *.o

$(TARGET) : $(OBJS1)
	@echo "start compile server"
	$(GCC) -g -o $@ $^ $(LIBDIR)

%.o : %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET)

.PHONY: cleanobj
cleanobj:
	rm -rf $(OBJS1)
