CC+=-std=c99 -Wall -O2 -flto -march=native -pedantic-errors -fgnu89-inline
CFLAGS=`pkg-config --cflags libudev mount`
LDFLAGS=`pkg-config --libs libudev mount`
TARGET=sallymount
OBJECTS=sallymount.o usb.o cli.o mount.o umount.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

again: clean all
