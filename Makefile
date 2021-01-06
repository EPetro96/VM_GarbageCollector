CFLAGS = -O2 -ffast-math

all: vm vm_gc vm_gc_other

vm: vm.c
	gcc $(CFLAGS) $^ -o $@

vm_gc: vm_gc.c
	gcc $(CFLAGS) $^ -o $@

vm_gc_other: vm_gc_other.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm -f vm vm_gc vm_gc_other