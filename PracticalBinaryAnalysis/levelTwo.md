## Practical Binary Analysis Chapter 5 Level 2

Chapter 5 of Practical Binary Analysis by Dennis Andriesse contains several CTF challenges at the end of chapter 5.

Challenges begin at level 2 because the level 1 solution is walked through in chapter 5.

All challenges are solved on a VM provided by the book.

### Getting Started

The hint for this level is:

```bash
binary@binary-VirtualBox:~$ ./oracle 84b34c124b2ba5ca224af8e33b077e9e -h
Combine the parts
```

We are presented with a simple binary called `lvl2`. Examining the binary with the `file` utility shows it's a 64-bit ELF executable stripped of symbols. Nothing seems out of the ordinary. The `strings` utility does not reveal any flag-like strings.

```bash
binary@binary-VirtualBox:~/lvl2$ file lvl2
lvl2: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=457d7940f6a73d6505db1f022071ee7368b67ce9, stripped
```

`ltrace` shows the binary is calling some random functions as well as `time`, presumably to seed the random number generation. It then prints what appears to be a byte in hexadecimal representation.

```bash
binary@binary-VirtualBox:~/lvl2$ ltrace ./lvl2
__libc_start_main(0x400500, 1, 0x7fff6a0538d8, 0x400640 <unfinished ...>
time(0)                                                                                       = 1591202286
srand(0x5ed7d1ee, 0x7fff6a0538d8, 0x7fff6a0538e8, 0)                                          = 0
rand(0x7fd43d3d4620, 0x7fff6a0537bc, 0x7fd43d3d40a4, 0x7fd43d3d411c)                          = 0x2288d465
puts("36"36
)                                                                                    = 3
+++ exited (status 0) +++
```

An `strace` performed on the binary doesn't reveal much of interest.

By executing the binary a couple of times, we can see it is printing out a different byte on every execution:

```bash
binary@binary-VirtualBox:~/lvl2$ ./lvl2
74
binary@binary-VirtualBox:~/lvl2$ ./lvl2
d3
binary@binary-VirtualBox:~/lvl2$ ./lvl2
6c
```

Let's take a look at the disassembly to find out where these bytes are being grabbed:

```asm
binary@binary-VirtualBox:~/lvl2$ objdump -M intel -d lvl2
...
400524:	48 8b 3c c5 60 10 60 	mov    rdi,QWORD PTR [rax*8+0x601060]
40052b:	00 
40052c:	e8 6f ff ff ff       	call   4004a0 <puts@plt>
```

At `0x400524` we can see that a byte is being indexed from offset `0x601060` and loaded into `rdi` to be passed to the following `puts` call to print to screen. Let's load lvl2 into `gdb` to examine that memory address:

```bash
binary@binary-VirtualBox:~/lvl2$ gdb lvl2
GNU gdb (Ubuntu 7.11.1-0ubuntu1~16.5) 7.11.1
Copyright (C) 2016 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from lvl2...(no debugging symbols found)...done.
(gdb) b *0x400524
Breakpoint 1 at 0x400524
(gdb) run
Starting program: /home/binary/lvl2/lvl2 

Breakpoint 1, 0x0000000000400524 in ?? ()
(gdb) set disassembly-flavor intel
(gdb) x/i $rip
=> 0x400524:	mov    rdi,QWORD PTR [rax*8+0x601060]
(gdb) x/16s *0x601060
0x4006c4:	"03"
0x4006c7:	"4f"
0x4006ca:	"c4"
0x4006cd:	"f6"
0x4006d0:	"a5"
0x4006d3:	"36"
0x4006d6:	"f2"
0x4006d9:	"bf"
0x4006dc:	"74"
0x4006df:	"f8"
0x4006e2:	"d6"
0x4006e5:	"d3"
0x4006e8:	"81"
0x4006eb:	"6c"
0x4006ee:	"df"
0x4006f1:	"88"
```

We loaded `lvl2` into `gdb`, broke just before the call to `puts`, and examined the next 16 bytes at that memory location. It certainly looks like a flag. Combined, the bytes form the flag `034fc4f6a536f2bf74f8d6d3816cdf88`

Let's check our answer:

```bash
binary@binary-VirtualBox:~$ ./oracle 034fc4f6a536f2bf74f8d6d3816cdf88
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
| Level 2 completed, unlocked lvl3         |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
Run oracle with -h to show a hint
```

Level complete!
