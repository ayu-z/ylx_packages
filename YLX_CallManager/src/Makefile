OBJS:=ModemDetect.o main.o cJSON.o
EXEC:=netlink
LIBS:=-L /home/yang/iyunlink/ylxwrt23.05/staging_dir/target-mipsel_24kc_musl/usr/lib -lusb-1.0 -libjson-c -I/home/yang/iyunlink/ylxwrt23.05/staging_dir/target-mipsel_24kc_musl/usr/include
# CC=/home/yang/iyunlink/openwrt_x86/staging_dir/toolchain-x86_64_gcc-12.3.0_musl/bin/x86_64-openwrt-linux-gcc
CC=/home/yang/iyunlink/ylxwrt23.05/staging_dir/toolchain-mipsel_24kc_gcc-12.3.0_musl/bin/mipsel-openwrt-linux-gcc
# CC=gcc
all: $(OBJS) 
	$(CC) -o $(EXEC) $(OBJS)  $(LIBS) -Wall 

%.o:%.c 
	@$(CC) -c -o $@ $< $(CFLAGS) -fPIC

clean:
	rm -f $(EXEC) *.o