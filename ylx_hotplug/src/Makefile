OBJS:=ylx_hotplug.o
EXEC:=ylx_hotplug
# LIBS:=-ludev


all: $(OBJS) 
	$(CC) -o $(EXEC) $(OBJS)  $(LIBS)

%.o:%.c 
	@$(CC) -c -o $@ $< $(CFLAGS) -fPIC

clean:
	rm -f $(EXEC) *.o