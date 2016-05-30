CFLAGS=-std=c99 -Wall -O2 -flto -march=native -pedantic-errors -fgnu89-inline
LDFLAGS=$(CFLAGS)
LDLIBS=-ludev -lmount
TARGET=sallymount
OBJECTS=sallymount.o usb.o cli.o mount.o umount.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $(TARGET) $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

again: clean all
