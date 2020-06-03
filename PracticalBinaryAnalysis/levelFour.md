## Practical Binary Analysis Chapter 5 Level 3

Chapter 5 of Practical Binary Analysis by Dennis Andriesse contains several CTF challenges at the end of chapter 5.

All challenges are solved on a VM provided by the book.

### Getting Started

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 3a5c381e40d2fffd95ba4452a0fb4a40 -h
Watch closely while I run
```

This hint lets us know this will probably involve some dynamic analysis, not strictly static.

As usual, we run through `file`, `strings`, `ltrace`, and `strace` before executing the binary itself.


```bash

binary@binary-VirtualBox:~/crackmes/chapter5/lvlFour$ file lvl4
lvl4: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=f8785d89a1f11e7b413c08c6176ad1ed7b95ca08, stripped

binary@binary-VirtualBox:~/crackmes/chapter5/lvlFour$ ltrace ./lvl4
__libc_start_main(0x4004a0, 1, 0x7ffdfbc73f08, 0x400650 <unfinished ...>
setenv("FLAG", "656cf8aecb76113a4dece1688c61d0e7"..., 1)                                      = 0
+++ exited (status 0) +++
```

Right away in the `ltrace` output we notice an environment variable "FLAG" being set. When we feed that string to the oracle, we get:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 656cf8aecb76113a4dece1688c61d0e7
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
| Level 4 completed, unlocked lvl5         |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
Run oracle with -h to show a hint
```

While we have found the flag, there must be more to the challenge than that. How is the flag being generated?

Running the binary does not seem to produce any output, arguments or otherwise.

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFour$ ./lvl4
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFour$ ./lvl4 foobar
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFour$ 
```

Running `strings` on the binary reveals one interesting string:

<pre>
binary@binary-VirtualBox:~/crackmes/chapter5/lvlFour$ strings lvl4
/lib64/ld-linux-x86-64.so.2
libc.so.6
__stack_chk_fail
__libc_start_main
setenv
__gmon_start__
GLIBC_2.4
GLIBC_2.2.5
D$(1
P@UH
TTQ\\]TH
L$(dH3
UH-@
AWAVA
AUATL
[]A\A]A^A\_
<b>XaDht-+1432=/as4?0129mklqt!@cnz^</b>
FLAG
</pre>

The string beginning with `XaDht` looks like it could be potentially manipulated into becoming the flag which is set in the `setenv` call. Let's take a look at the disassembly:

<pre>
00000000004004a0 <.text>:
.........
  400500:	0f b6 14 04          	movzx  edx,BYTE PTR [rsp+rax\*1]
  400504:	32 90 d8 06 40 00    	xor    dl,BYTE PTR [rax+0x4006d8]
  40050a:	88 14 04             	mov    BYTE PTR [rsp+rax\*1],dl
  40050d:	48 83 c0 01          	add    rax,0x1
  400511:	48 83 f8 20          	cmp    rax,0x20
  400515:	75 e9                	jne    400500 <__libc_start_main@plt+0x80>
</pre>

In this section of the disassembly, we can see a byte is being loaded into `edx`, XOR'd with some byte being loaded from offset `0x4006d8`, and loaded back onto the stack. It appears `rax` contains a loop counter which runs 20 times -- If the loop has not run 20 times, it grabs the next byte and XORs it once again. Let's take a look at the program execution in `gdb` to get a better idea of what's going on

```asm
(gdb) x/s 0x4006d8
0x4006d8:	"XaDht-+1432=/as4?0129mklqt!@cnz^"
```

Now we know for sure that the string is being used to `xor` with the value on the stack. Before the loop has begun, the value in the `$rsp` register contains:

```asm
(gdb) x/s $rsp
0x7fffffffdeb0:	"nTr\v\022\025JTWQ\005\v\036P@U\vTTQ\\\\]TI\027\027q\a^\037i"
```

Once we execute the loop several times, we begin to see the FLAG environment variable we identified take shape:

<pre>
Breakpoint 1, 0x0000000000400500 in ?? ()
(gdb) c
Continuing.

Breakpoint 1, 0x0000000000400500 in ?? ()
(gdb) c
Continuing.

Breakpoint 1, 0x0000000000400500 in ?? ()
(gdb) c
Continuing.

Breakpoint 1, 0x0000000000400500 in ?? ()
(gdb) x/s $rsp
0x7fffffffdeb0:	"<b>656cf8ae</b>WQ\005\v\036P@U\vTTQ\\\\]TI\027\027q\a^\037i"
</pre>

The `setenv` call is made at `0x400524`. We can see the final result of the `xor` loop by setting a breakpoint at that address and examining the `$rsp` register.

Location of setenv call:
```asm
  400517:	ba 01 00 00 00       	mov    edx,0x1
  40051c:	48 89 e6             	mov    rsi,rsp
  40051f:	bf 00 07 40 00       	mov    edi,0x400700
  400524:	e8 37 ff ff ff       	call   400460 <setenv@plt>
```

Continuing execution until just before `setenv` call and examining value of `rsp`:
<pre>
(gdb) b *0x400524
Breakpoint 3 at 0x400524
(gdb) c
Continuing.

Breakpoint 3, 0x0000000000400524 in ?? ()
(gdb) x/s $rsp
0x7fffffffdeb0:	<b>"656cf8aecb76113a4dece1688c61d0e7"</b>
</pre>

In the end, this program turned out to be a simple `xor` encryption loop.
