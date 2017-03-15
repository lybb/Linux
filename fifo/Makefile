read=fifo_read
write=fifo_write
cc=gcc

readSrc=read.c
writSrc=write.c

.PHONY:all
all:$(read) $(write)

$(read):$(readSrc)
	$(cc) -o $@ $^
$(write):$(writSrc)
	$(cc) -o $@ $^

.PHONY:clean
clean:
	rm -f $(read) $(write) myfifo
