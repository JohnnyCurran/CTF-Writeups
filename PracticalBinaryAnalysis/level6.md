## Practical Binary Analysis Chapter 5 Level 5

Chapter 5 of Practical Binary Analysis by Dennis Andriesse contains several CTF challenges at the end of chapter 5.

All challenges are solved on a VM provided by the book.

### Getting Started


```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 0fa355cbec64a05f7a5d050e836b1a1f -h
Find out what I expect, then trace me for a hint
```

Seems `strace` and `ltrace` will be important in solving this challenge.

### Analysis

Let's start with a simple `strings` call on the binary and see what we can find

<pre>
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ strings lvl6
/lib64/ld-linux-x86-64.so.2
libc.so.6
&#95;&#95;printf&#95;chk
&#95;&#95;stack&#95;chk&#95;fail
putchar
&#95;&#95;sprintf&#95;chk
<b>
strcmp
</b>
&#95;&#95;libc&#95;start&#95;main
<b>
setenv
</b>
&#95;&#95;gmon&#95;start&#95;&#95;
GLIBC&#95;2.3.4
GLIBC&#95;2.4
GLIBC&#95;2.2.5
UH-
AWAVA
AUATL
[]A\A]A^A
<b>
DEBUG: argv[1] = %s
get_data_addr
0x%jx
DATA_ADDR
</b>
; * 3$"
GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.9) 5.4.0 20160609
.shstrtab
.interp
.note.ABI-tag
.note.gnu.build-id
.......
</pre>

The strings in bold reveal some information: It appears the argument passed to the function is being checked in some way based on the `DEBUG: argv[1] = %s` string as well as the call to `strcmp` revealed in the above output. There's a `%jx` format specifier presumably to print out the data address referred to by the `DATA_ADDR` strings. Additionally, there appears there will be a call to setenv somewhere in the program execution. Maybe that call will contain the flag, like in level 4.

Program execution prints out all primes below 100. It does not appear to change when given an arbirtrary argument(s):

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ./lvl6
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ./lvl6 foo
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ./lvl6 foo bar baz
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97
```

Let's trace the program execution with `ltrace` and `strace` with no arguments and see what we can learn.

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ltrace ./lvl6
__libc_start_main(0x4005f0, 1, 0x7ffe344b3a18, 0x400890 <unfinished ...>
__printf_chk(1, 0x400947, 2, 100)                                                             = 2
__printf_chk(1, 0x400947, 3, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 5, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 7, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 11, 0x7ffffffe)                                                     = 3
__printf_chk(1, 0x400947, 13, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 17, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 19, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 23, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 29, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 31, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 37, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 41, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 43, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 47, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 53, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 59, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 61, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 67, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 71, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 73, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 79, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 83, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 89, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 97, 0x7ffffffd)                                                     = 3
putchar(10, 3, 0, 0x7ffffffd2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
)                                                                 = 10
+++ exited (status 0) +++
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ./lvl6
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97

binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ strace ./lvl6
execve("./lvl6", ["./lvl6"], [/* 60 vars */]) = 0
brk(NULL)                               = 0xe4e000
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
open("/home/binary/code/chapter5/tls/x86_64/libc.so.6", O_RDONLY|O_CLOEXEC) = -1 ENOENT (No such file or directory)
stat("/home/binary/code/chapter5/tls/x86_64", 0x7ffc154ae9a0) = -1 ENOENT (No such file or directory)
open("/home/binary/code/chapter5/tls/libc.so.6", O_RDONLY|O_CLOEXEC) = -1 ENOENT (No such file or directory)
stat("/home/binary/code/chapter5/tls", 0x7ffc154ae9a0) = -1 ENOENT (No such file or directory)
open("/home/binary/code/chapter5/x86_64/libc.so.6", O_RDONLY|O_CLOEXEC) = -1 ENOENT (No such file or directory)
stat("/home/binary/code/chapter5/x86_64", 0x7ffc154ae9a0) = -1 ENOENT (No such file or directory)
open("/home/binary/code/chapter5/libc.so.6", O_RDONLY|O_CLOEXEC) = -1 ENOENT (No such file or directory)
stat("/home/binary/code/chapter5", {st_mode=S_IFDIR|0775, st_size=4096, ...}) = 0
open("/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=98537, ...}) = 0
mmap(NULL, 98537, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f9359e45000
close(3)                                = 0
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
open("/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0P\t\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1868984, ...}) = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f9359e44000
mmap(NULL, 3971488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f935986f000
mprotect(0x7f9359a2f000, 2097152, PROT_NONE) = 0
mmap(0x7f9359c2f000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1c0000) = 0x7f9359c2f000
mmap(0x7f9359c35000, 14752, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f9359c35000
close(3)                                = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f9359e43000
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f9359e42000
arch_prctl(ARCH_SET_FS, 0x7f9359e43700) = 0
mprotect(0x7f9359c2f000, 16384, PROT_READ) = 0
mprotect(0x600000, 4096, PROT_READ)     = 0
mprotect(0x7f9359e5e000, 4096, PROT_READ) = 0
munmap(0x7f9359e45000, 98537)           = 0
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(136, 0), ...}) = 0
brk(NULL)                               = 0xe4e000
brk(0xe6f000)                           = 0xe6f000
write(1, "2 3 5 7 11 13 17 19 23 29 31 37 "..., 722 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
) = 72
exit_group(0)                           = ?
+++ exited with 0 +++
```

There is not a lot to be gleaned from these two traces. Notably, we do not have any calls to `setenv` or `strcmp` - presumably because no argument was passed.

Since this program seems to expect an argument based on the `argv[1]` string, let's trace execution with an argument given:

<pre>
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ltrace ./lvl6 foobar
__libc_start_main(0x4005f0, 2, 0x7fff68222678, 0x400890 <unfinished ...>
<b>
strcmp("foobar", "get_data_addr")                                                             = -1
</b>
__printf_chk(1, 0x400947, 2, 100)                                                             = 2
__printf_chk(1, 0x400947, 3, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 5, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 7, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 11, 0x7ffffffe)                                                     = 3
__printf_chk(1, 0x400947, 13, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 17, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 19, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 23, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 29, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 31, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 37, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 41, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 43, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 47, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 53, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 59, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 61, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 67, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 71, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 73, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 79, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 83, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 89, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 97, 0x7ffffffd)                                                     = 3
putchar(10, 3, 0, 0x7ffffffd2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
)                                                                 = 10
+++ exited (status 0) +++
</pre>

Here we can see that our argument `foobar` is being compared to `get_data_addr`. Let's run the program with this argument with and without `ltrace` and see what we get:


<pre>
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ./lvl6 get_data_addr
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSix$ ltrace ./lvl6 get_data_addr
__libc_start_main(0x4005f0, 2, 0x7ffcd2c3e648, 0x400890 <unfinished ...>
strcmp("get_data_addr", "get_data_addr")                                                      = 0
__sprintf_chk(0x7ffcd2c3e140, 1, 1024, 0x400937)                                              = 8
<b>
setenv("DATA_ADDR", "0x4006c1", 1)                                                            = 0
</b>
__printf_chk(1, 0x400947, 2, 100)                                                             = 2
__printf_chk(1, 0x400947, 3, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 5, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 7, 0x7ffffffe)                                                      = 2
__printf_chk(1, 0x400947, 11, 0x7ffffffe)                                                     = 3
__printf_chk(1, 0x400947, 13, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 17, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 19, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 23, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 29, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 31, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 37, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 41, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 43, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 47, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 53, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 59, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 61, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 67, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 71, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 73, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 79, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 83, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 89, 0x7ffffffd)                                                     = 3
__printf_chk(1, 0x400947, 97, 0x7ffffffd)                                                     = 3
putchar(10, 3, 0, 0x7ffffffd2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71 73 79 83 89 97 
)                                                                 = 10
+++ exited (status 0) +++
</pre>

With the correct argument passed the program is now setting an environment variable named `DATA_ADDR` with the value `0x4006c1`. Looking at the disassembly at that address, we have:

<pre>
  4006b8:	8b 44 24 24          	mov    eax,DWORD PTR [rsp+0x24]
  4006bc:	83 f8 00             	cmp    eax,0x0
  4006bf:	74 10                	je     4006d1 <__sprintf_chk@plt+0x101>
  <b>
  4006c1:	2e 29 c6             	cs sub esi,eax
  </b>
  4006c4:	4a 0f 03 a6 ee 2a 30 	rex.WX lsl rsp,WORD PTR [rsi+0x7f302aee]
  4006cb:	7f 
  4006cc:	ec                   	in     al,dx
  4006cd:	c8 c3 ff 42          	enter  0xffc3,0x42
  4006d1:	48 8d ac 24 90 01 00 	lea    rbp,[rsp+0x190]
</pre>

The name `DATA_ADDR` is a hint the data we're looking for is hidden somewhere around this address. Is it possible the 16 bytes we need are the opcodes for the instructions starting at `0x4006c1`?

Combining the hex from the opcodes from `0x4006c1` to `0x4006cd`, we get the byte string `2e29c64a0f03a6ee2a307fecc8c3ff42`

Feeding it to the oracle, we obtain:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 2e29c64a0f03a6ee2a307fecc8c3ff42
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
| Level 6 completed, unlocked lvl7         |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
Run oracle with -h to show a hint
```

Time to move on to level 7
