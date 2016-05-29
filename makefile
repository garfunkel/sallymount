CC=clang -Wall -O2 -flto -pipe -march=native
CFLAGS=
LDFLAGS=
TARGET=sallymount
OBJECTS=sallymount.o usb.o cli.o mount.o umount.o
LIBS=-ludev -lmount

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LIBS) -o $(TARGET) $(OBJECTS)

%.o: %.c %.h
	$(CC) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

again: clean all
