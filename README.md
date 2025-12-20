# fasm2-elf64
Macro for fasm2 to produce x86-64 Linux ELF64 executables.
* Dynamic linking support,
* Call macros for `syscall`s and `stdcall`s.

## HelloWorld using `syscall`

```
include "elf64.inc"
	sys         write, 1, <"hello, world", 10>, @$
	sys         exit, 0
```

`@$` contains number of bytes in the last string argument, here it is `13` (length of `"hello, world", 10`).

Produces `148`-byte ELF64 executable. Can anyone beat this size?

## HelloWorld using `libc`

```
include "elf64.inc"
needed "libc.so.6"
	dyncallz    puts, "hello, world"
	dyncall     _exit, 0
```

`dyncallz` is like `dyncall` (call function from a dynamic library with parameters), but appends a zero byte to all string arguments.

Produces `464`-byte ELF64 executable. Once again: can anyone beat this size?
