CFLAGS = -g -O2 -c
LDFLAGS = 

TARGET = input

%.o:%.c
	gcc $(CFLAGS) $< 

$(TARGET): $(TARGET).o
	gcc $< -o $@ $(LDFLAGS)
