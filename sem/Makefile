bin=test
cc=gcc
src=comm.c test.c

$(bin):$(src)
	$(cc) -o $@ $^

.PHONY:clean
clean:
	rm -f $(bin)
