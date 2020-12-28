########################################################################
TARGET	= find_max_amp

SRCS	= $(TARGET).c parse_cmdline.c read_sac.c peak_trough.c

OBJS	= $(SRCS:%.c=%.o)

LIBS	= -lm

CC	= cc
COPT	= -g
CFLAGS	= $(COPT) $(IQLIB2) -m32

all:	$(TARGET) 

$(TARGET):	$(OBJS) $(QLIB2)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:	
	rm -rf *.o
