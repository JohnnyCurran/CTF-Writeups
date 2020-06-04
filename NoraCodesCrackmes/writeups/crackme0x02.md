## Crackme0x02

All crackmes are sourced from [this repo](https://github.com/leotindall/crackmes)

Analysis of binary performed on a VirtualBox Ubuntu 18.04 VM

### Getting Started ###

After compilation, we have a 64-bit ELF binary with symbols included. Similarly to crackme0x01, we begin by attempting to execute it. This binary also expects exactly one argument.

```bash
johnny:crackmes$ file crackme02
crackme02: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 3.2.0, BuildID[sha1]=aa8efed44085f874c65cfed91eb37d04db829586, not stripped

johnny:crackmes$ ./crackme02
Need exactly one argument.
```

Running `strings` on the binary reveals a strings list extremely similar to CrackMe0x01. It even includes the same `password1` string. Unfortunately that does not solve the binary:

```bash
johnny:crackmes$ ./crackme02 password1
No, password1 is not correct.
```

ltrace and strace don't reveal anything out of the ordinary. Let's go to the disassembly.

### Disassembly ###

We'll begin by examining the `.text` section and the `main` function within (init sections are omitted for brevity).

```asm
johnny:crackmes$ objdump -M intel -D crackme02 --section=.text

crackme02:     file format elf64-x86-64


Disassembly of section .text:
...
000000000000068a <main>:
68a:	55		        push   rbp
68b:	48 89 e5	        mov    rbp,rsp
68e:	48 83 ec 20	        sub    rsp,0x20
692:	89 7d ec	        mov    DWORD PTR [rbp-0x14],edi
695:	48 89 75 e0	        mov    QWORD PTR [rbp-0x20],rsi
699:	83 7d ec 02	        cmp    DWORD PTR [rbp-0x14],0x2
69d:	74 16		        je     6b5 <main+0x2b>
69f:	48 8d 3d 5e 01 00 00	lea    rdi,[rip+0x15e]        # 804 <_IO_stdin_used+0x4>
6a6:	e8 a5 fe ff ff	        call   550 <puts@plt>
6ab:	b8 ff ff ff ff	        mov    eax,0xffffffff
6b0:	e9 c7 00 00 00	        jmp    77c <main+0xf2>
6b5:	48 8d 05 63 01 00 00    lea    rax,[rip+0x163]        # 81f <_IO_stdin_used+0x1f>
6bc:	48 89 45 f8	        mov    QWORD PTR [rbp-0x8],rax
6c0:	c7 45 f4 00 00 00 00    mov    DWORD PTR [rbp-0xc],0x0
6c7:	eb 5d		        jmp    726 <main+0x9c>
```

We see the standard function prologue
```
68a:    55		push   rbp
68b:    48 89 e5        mov    rbp,rsp
68e:    48 83 ec 20     sub    rsp,0x20
```

The compiler assigns some local variables and then checks to see that there is exactly one argument provided (cmp with `0x02` because first argument supplied is always the name of the program, in this case "crackme02"):

```asm
692:    89 7d ec            mov    DWORD PTR [rbp-0x14],edi
695:    48 89 75 e0         mov    QWORD PTR [rbp-0x20],rsi
699:    83 7d ec 02         cmp    DWORD PTR [rbp-0x14],0x2
```

In the event no argument is supplied, the program loads the address at memory location `0x804` into `rdi` and calls `puts`, writing the string to the screen. It then jumps to `0x77c`, terminating execution.

Just as we'd expect, the string at `0x804` is the one telling us to supply exactly one argument:

```asm
johnny:crackmes$ xxd crackme02 | grep 00000800 --after-context=1
00000800: 0100 0200 4e65 6564 2065 7861 6374 6c79  ....Need exactly
00000810: 206f 6e65 2061 7267 756d 656e 742e 0070   one argument..p
```

The more intereseting part of the program execution happens when we take the `je` at `0x69d` to `0x6b5`, meaning we supplied the program with exactly one argument. Here you can see we load the string at `0x81f` into two local variables and take the jump to `0x726`.

What is the value stored at `0x81f`? Let's take a look:
```asm
johnny:crackmes$ xxd crackme02 | grep 00000810 --after-context=1
00000810: 206f 6e65 2061 7267 756d 656e 742e 0070   one argument..p
00000820: 6173 7377 6f72 6431 004e 6f2c 2025 7320  assword1.No, %s 
```

Looking at the memory dump we can see that it is the same `password1` string from the first binary. We know that the plain string does not work as the key to the crackme, so it is probably being manipulated in some way.

Let's take a look at the disassembly a little further. When one argument is supplied to the binary, we take the jump to `0x6b5`:

```asm
 6b5:	48 8d 05 63 01 00 00 	lea    rax,[rip+0x163]        # 81f <_IO_stdin_used+0x1f>
 6bc:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
 6c0:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [rbp-0xc],0x0
 6c7:	eb 5d                	jmp    726 <main+0x9c>
 ```

 The string "password1" is loaded into `rax`, assigned to some local variables, and we jump to `0x726`.

 When we jump, we encounter a null-byte (end of string) check:

```asm
 726:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
 729:	48 63 d0             	movsxd rdx,eax
 72c:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
 730:	48 01 d0             	add    rax,rdx
 733:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 736:	84 c0                	test   al,al
 738:	74 1e                	je     758 <main+0xce>
 ```
 If the string has ended, we will jump to a failure point at `0x758`. The failure string is loaded and the program stops execution.

 However, if the string has not yet terminated, we take a jump to `0x6c9`, where the pieces begin to fall into place:

<pre>
 6c9:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
 6cc:	48 63 d0             	movsxd rdx,eax
 6cf:	48 8b 45 f8          	mov    rax,QWORD PTR [rbp-0x8]
 6d3:	48 01 d0             	add    rax,rdx
 6d6:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 6d9:	0f be c0             	movsx  eax,al
 <b>
 6dc:	8d 48 ff             	lea    ecx,[rax-0x1]
 </b>
 6df:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
 6e3:	48 83 c0 08          	add    rax,0x8
 6e7:	48 8b 10             	mov    rdx,QWORD PTR [rax]
 6ea:	8b 45 f4             	mov    eax,DWORD PTR [rbp-0xc]
 6ed:	48 98                	cdqe   
 6ef:	48 01 d0             	add    rax,rdx
 6f2:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 6f5:	0f be c0             	movsx  eax,al
<b>
 6f8:	39 c1                	cmp    ecx,eax
</b>
 6fa:	74 26                	je     722 <main+0x98>
 </pre>

Here, a byte is taken from `$rax` ("password1"), subtracted by 1 at `0x6dc`, and loaded into `$ecx`. The next few instructions load our supplied argument ("foobar") into `$rax`, is indexed by `$rdx`, and then compared at `0x6f8`. It would appear this loop takes each character in the string "password1", subtracts 1, and then compares it against our supplied argument. By subtracting 1 from each character in "password1", we get the string "o```rrvnqc0`".

Let's try checking it against our program (with an escape \ for the backtick):

```bash
johnny:crackmes$ ./crackme02 o\`rrvnqc0ff
Yes, o`rrvnqc0ff is correct!
```

CrackMe solved!

An interesting note: Simply passing the letter "o" to the program will solve it as well. I'll have to follow up on why this is. Other combinations of characters starting with the letter "o" do not work.

```bash
johnny:crackmes$ ./crackme02 o
Yes, o is correct!
johnny:crackmes$ ./crackme02 oo
No, oo is not correct.
johnny:crackmes$ ./crackme02 ohno
No, ohno is not correct.
```
