CC=gcc
OPT=-Wall -g
TARGET=sallymount
OBJECTS=sallymount.o usb.o cli.o mount.o umount.o
LIBS=-ludev -lm -lmount

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OPT) $(LIBS) -o $(TARGET) $(OBJECTS)

%.o: %.c %.h
	$(CC) $(OPT) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

again: clean all
