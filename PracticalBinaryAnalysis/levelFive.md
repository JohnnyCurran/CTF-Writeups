## Practical Binary Analysis Chapter 5 Level 5

Chapter 5 of Practical Binary Analysis by Dennis Andriesse contains several CTF challenges at the end of chapter 5.

All challenges are solved on a VM provided by the book.

### Getting Started

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 656cf8aecb76113a4dece1688c61d0e7 -h
   Secrets hidden in code unused
The method of redirection is the key
   Static rather than dynamically
```

As the hint states, static analysis will be valuable in this analysis because certain code sections are unused.

### Analysis

Let's see if there's anything unusual with the `file` command and check for interesting `strings`.

<pre>
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ file lvl5
lvl5: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=1c4f1d4d245a8e252b77c38c9c1ba936f70d8245, stripped

binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ strings lvl5
/lib64/ld-linux-x86-64.so.2
libc.so.6
__printf_chk
puts
__stack_chk_fail
__libc_start_main
__gmon_start__
GLIBC_2.3.4
GLIBC_2.4
GLIBC_2.2.5
UH-H
D$(1
Eev4A6ufH
3peH
gqfH
T$ H
L$(dH3
AWAVA
AUATL
<b>key = 0x%08x
decrypted flag = %s
nothing to see here</b>
...
</pre>

The final three strings in the above snippet are worth looking into further. A quick `grep` on the `xxd` output of the binary for each string revealed that they are located at memory locations `0x400774`, `0x400782`, and `0x400797`, respectively.

The program will only output `nothing to see here` when executed:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ ./lvl5
nothing to see here
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ ./lvl5 foobar
nothing to see here
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ ./lvl5 foo bar
nothing to see here
```

Let's check the disassembly and see what we can learn from the program instructions

```asm
40052f: 49 c7 c0 60 07 40 00    mov    r8,0x400760
400536: 48 c7 c1 f0 06 40 00    mov    rcx,0x4006f0
40053d:	48 c7 c7 00 05 40 00	mov    rdi,0x400500
400544:	e8 87 ff ff ff		call   4004d0 <__libc_start_main@plt>
```

Here we see the address of main being loaded into the `rdi` register. The code in this main method is simple:

```asm
400500:	48 83 ec 08          	sub    rsp,0x8
400504:	bf 97 07 40 00       	mov    edi,0x400797
400509:	e8 a2 ff ff ff       	call   4004b0 <puts@plt>
40050e:	b8 01 00 00 00       	mov    eax,0x1
400513:	48 83 c4 08          	add    rsp,0x8
400517:	c3                   	ret 
```

It loads the string **nothing to see here**, prints it, and returns. How can we alter this program execution to do what we want?

The strings we are interested in, "key = %s", and "decrypted string = %s" we know are located at locations `0x400774` and `0x400782`. Let's find these instructions in our program:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ objdump -d lvl5 -M intel | grep 400774
  400621:   be 74 07 40 00	    mov    esi,0x400774
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ objdump -d lvl5 -M intel | grep 400782
  4006b0: be 82 07 40 00	    mov    esi,0x400782
```

Analyzing the instructions at those memory addresses and following them upwards, we eventually reach `0x400620` which looks like a standard function prologue:

```asm
400620: 53		    push   rbx
400621: be 74 07 40 00	    mov    esi,0x400774
400626:	bf 01 00 00 00	    mov    edi,0x1
40062b:	48 83 ec 30	    sub    rsp,0x30
```

Using gdb, let's alter program execution to jump to `0x400620` instead of the original `0x400500`:

<pre>
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ gdb lvl5
.........
Reading symbols from lvl5...(no debugging symbols found)...done.
(gdb) b * 0x400544
Breakpoint 1 at 0x400544
(gdb) run
Starting program: /home/binary/crackmes/chapter5/lvlFive/lvl5 

Breakpoint 1, 0x0000000000400544 in ?? ()
(gdb) set $rdi=0x400620
(gdb) c
Continuing.
<b>
key = 0x00400500
decrypted flag = ea36cbE64A35fb5d60e06bb1f
</b>
[Inferior 1 (process 14443) exited normally]
(gdb) 
</pre>
There are some control characters which do not show up well in the markdown, but they are a solid indicator our flag is not exactly what we are looking for. We can see the `key` was set to `0x00400500`. Notably, this is the same address we altered to redirect execution. Maybe if we patch the binary to contain the address we want, it will be loaded as the correct key. 

Using xxd, we look for the instruction bytes at `0x40053d`:

<pre>
00000530: c7c0 6007 4000 48c7 c1f0 0640 00<b>48 c7c7</b>  ..`` ` ``.@.H....@.H..
00000540: <b>0005 40</b>00 e887 ffff fff4 660f 1f44 0000  ..@.......f..D..
</pre>

We want to alter the bytes at `0x540` to be the address of our new main function, located at `0x400620`. In little-endian, this will become `20 06 40`

Altering the program using vim with xxd gives us this new `.text` disassembly with the proper start address loaded:

```asm
40052f: 49 c7 c0 60 07 40 00    mov    r8,0x400760
400536: 48 c7 c1 f0 06 40 00    mov    rcx,0x4006f0
40053d:	48 c7 c7 20 06 40 00	mov    rdi,0x400620
400544:	e8 87 ff ff ff		call   4004d0 <__libc_start_main@plt>
```

Which, when executed, gives us the proper flag:


```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFive$ ./patched
key = 0x00400620
decrypted flag = 0fa355cbec64a05f7a5d050e836b1a1f
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 0fa355cbec64a05f7a5d050e836b1a1f
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
| Level 5 completed, unlocked lvl6         |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
Run oracle with -h to show a hint
```
