# A simple Virtual Machine + Garbage Collector in C

Constructed as part of an exercise for the "Programming Languages 2" course in NTUA (2018-2019)  

### Description

The VM reads a sequence of bytecode instructions (no more than 65536 bytes of instructions).  
Every instructions begins with a 1-byte opcode.  
'vm_gc' implements a garbage collector for this VM.  