## Crackme0x03e

All crackmes are sourced from [this repo](https://github.com/leotindall/crackmes)

Analysis of binary performed on a VirtualBox Ubuntu 18.04 VM

### Getting Started ###

As per usual, we'll begin with a `strings` call on the binary:

<pre>
johnny@johnny-VirtualBox:~/reversing/crackmes$ strings a.out
/lib64/ld-linux-x86-64.so.2
libc.so.6
puts
&#5;&#5;stack&#5;chk&#5;fail
printf
<b>strlen</b>
&#5;&#5;cxa&#5;finalize
&#5;&#5;libc&#5;start&#5;main
GLIBC&#5;2.4
GLIBC&#5;2.2.5
&#5;ITM&#5;deregisterTMCloneTable
&#5;&#5;gmon&#5;start&#5;&#5;
&#5;ITM&#5;registerTMCloneTable
=i   
5b   
<b>passwordH</b>
AWAVI
AUATL
[]A\A]A^A&#5;
Need exactly one argument.
Yes, %s is correct!
No, %s is not correct.
;&#42;3$"
GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0
crtstuff.c
deregister&#5;tm&#5;clones
&#5;&#5;do&#5;global&#5;dtors&#5;aux
completed.7698
&#5;&#5;do&#5;global&#5;dtors&#5;aux&#5;fini&#5;array&#5;entry
frame&#5;dummy
&#5;&#5;frame&#5;dummy&#5;init&#5;array&#5;entry
crackme03e.c
&#5;&#5;FRAME&#5;END&#5;&#5;
&#5;&#5;init&#5;array&#5;end
&#5;DYNAMIC
&#5;&#5;init&#5;array&#5;start
&#5;&#5;GNU&#5;EH&#5;FRAME&#5;HDR
&#5;GLOBAL&#5;OFFSET&#5;TABLE&#5;
&#5;&#5;libc&#5;csu&#5;fini
&#5;ITM&#5;deregisterTMCloneTable
puts@@GLIBC&#5;2.2.5
&#5;edata
strlen@@GLIBC&#5;2.2.5
&#5;&#5;stack&#5;chk&#5;fail@@GLIBC&#5;2.4
printf@@GLIBC&#5;2.2.5
&#5;&#5;libc&#5;start&#5;main@@GLIBC&#5;2.2.5
&#5;&#5;data&#5;start
&#5;&#5;gmon&#5;start&#5;&#5;
&#5;&#5;dso&#5;handle
&#5;IO&#5;stdin&#5;used
&#5;&#5;libc&#5;csu&#5;init
<b>check&#5;pw</b>
&#5;&#5;bss&#5;start
main
</pre>

Just like in `Crackme0x03`, we can see a `strlen` call, a string `passwordH`, and a `check_pw` method.

Using `radare2` and examining the disassembly of the `main` function we can see this binary expects exactly one argument of length 8. 

The exactly one argument check happens at `0x7e9` - The `argc` variable is compared with the immediate value `2` (the name of the program in addition to the password argument passed to the function).

The length check happens at the comparison at `0x852` - The length of our passed argument is compared with the immediate value 8.

<pre>
[0x000007cb]> pdf@main
            ;-- main:
	    / (fcn) main 271
	    |   main ();
	    |           ; var int local&#5;30h @ rbp-0x30
	    |           ; var int local&#5;24h @ rbp-0x24
	    |           ; var int local&#5;1ah @ rbp-0x1a
	    |           ; var int local&#5;12h @ rbp-0x12
	    |           ; var int local&#5;11h @ rbp-0x11
	    |           ; var int local&#5;10h @ rbp-0x10
	    |           ; var int local&#5;fh @ rbp-0xf
	    |           ; var int local&#5;eh @ rbp-0xe
	    |           ; var int local&#5;dh @ rbp-0xd
	    |           ; var int local&#5;bh @ rbp-0xb
	    |           ; var int local&#5;ah @ rbp-0xa
	    |           ; var int local&#5;9h @ rbp-0x9
	    |           ; var int local&#5;8h @ rbp-0x8
	    |              ; DATA XREF from 0x0000064d (entry0)
	    |           0x000007cb      55             push rbp
	    |           0x000007cc      4889e5         mov rbp, rsp
	    |           0x000007cf      4883ec30       sub rsp, 0x30               ; '0'
	    |           0x000007d3      897ddc         mov dword [local&#5;24h], edi
	    |           0x000007d6      488975d0       mov qword [local&#5;30h], rsi
	    |           0x000007da      64488b042528.  mov rax, qword fs:[0x28]    ; [0x28:8]=0x19e0 ; '('
	    |           0x000007e3      488945f8       mov qword [local&#5;8h], rax
	    |           0x000007e7      31c0           xor eax, eax
<b>	    |           0x000007e9      837ddc02       cmp dword [local&#5;24h], 2    ; [0x2:4]=0x102464c</b>
	    |       ,=< 0x000007ed      7416           je 0x805
	    |       |   0x000007ef      488d3d6e0100.  lea rdi, qword str.Need&#5;exactly&#5;one&#5;argument. ; 0x964 ; "Need exactly one argument." ; const char &#42; s
	    |       |   0x000007f6      e8e5fdffff     call sym.imp.puts           ; int puts(const char &#42;s)
	    |       |   0x000007fb      b8ffffffff     mov eax, 0xffffffff         ; -1
	    |      ,==< 0x00000800      e9bf000000     jmp 0x8c4
	    |      ||      ; JMP XREF from 0x000007ed (main)
	    |      |`-> 0x00000805      48b870617373.  movabs rax, 0x64726f7773736170
	    |      |    0x0000080f      488945e6       mov qword [local&#5;1ah], rax
	    |      |    0x00000813      c645ee00       mov byte [local&#5;12h], 0
	    |      |    0x00000817      48c745ef0000.  mov qword [local&#5;11h], 0
	    |      |    0x0000081f      c645f700       mov byte [local&#5;9h], 0
	    |      |    0x00000823      c645ef03       mov byte [local&#5;11h], 3
	    |      |    0x00000827      c645f005       mov byte [local&#5;10h], 5
	    |      |    0x0000082b      c645f102       mov byte [local&#5;fh], 2
	    |      |    0x0000082f      c645f204       mov byte [local&#5;eh], 4
	    |      |    0x00000833      c645f301       mov byte [local&#5;dh], 1
	    |      |    0x00000837      c645f503       mov byte [local&#5;bh], 3
	    |      |    0x0000083b      c645f601       mov byte [local&#5;ah], 1
	    |      |    0x0000083f      488b45d0       mov rax, qword [local&#5;30h]
	    |      |    0x00000843      4883c008       add rax, 8
	    |      |    0x00000847      488b00         mov rax, qword [rax]
	    |      |    0x0000084a      4889c7         mov rdi, rax                ; const char &#42; s
	    |      |    0x0000084d      e89efdffff     call sym.imp.strlen         ; size&#5;t strlen(const char &#42;s)
<b>	    |      |    0x00000852      4883f808       cmp rax, 8</b>
	    |      |,=< 0x00000856      7548           jne 0x8a0
</pre>

Just like in `Crackme0x03`, let's check out what arguments are passed to `check_pw` when we pass in a password of appropriate length:

<pre>
johnny@johnny-VirtualBox:~/reversing/crackmes$ gdb a.out
GNU gdb (Ubuntu 8.1-0ubuntu3.2) 8.1.0.20180409-git
Copyright (C) 2018 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86&#5;64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from a.out...(no debugging symbols found)...done.
(gdb) set args 12345678
(gdb) b check&#5;pw
Breakpoint 1 at 0x73e
(gdb) run
Starting program: /home/johnny/reversing/crackmes/a.out 12345678

Breakpoint 1, 0x000055555555473e in check&#5;pw ()
    (gdb) x/i $rip
    => 0x55555555473e <check&#5;pw+4>:	mov    QWORD PTR [rbp-0x18],rdi
    (gdb) x/10i $rip
    => 0x55555555473e <check&#5;pw+4>:	mov    QWORD PTR [rbp-0x18],rdi
    0x555555554742 <check&#5;pw+8>:	mov    QWORD PTR [rbp-0x20],rsi
    0x555555554746 <check&#5;pw+12>: mov    QWORD PTR [rbp-0x28],rdx
    0x55555555474a <check&#5;pw+16>:	mov    DWORD PTR [rbp-0x4],0x0
    0x555555554751 <check&#5;pw+23>:   mov    BYTE PTR [rbp-0x6],0x1
    0x555555554755 <check&#5;pw+27>:	mov    eax,DWORD PTR [rbp-0x4]
    0x555555554758 <check&#5;pw+30>:	movsxd rdx,eax
    0x55555555475b <check&#5;pw+33>:  mov    rax,QWORD PTR [rbp-0x20]
    0x55555555475f <check&#5;pw+37>:	add    rax,rdx
    0x555555554762 <check&#5;pw+40>:    movzx  eax,BYTE PTR [rax]
    (gdb) x/s $rdi
    0x7fffffffe3b3:  "12345678"
    (gdb) x/s $rsi
    0x7fffffffdf46:  "password"
    (gdb) x/8x $rdx
    0x7fffffffdf4f:  0x03    0x05    0x02    0x04    0x01    0x00    0x03    0x01
</pre>

It looks like this `check_pw` function is also applying a mask - this time to the string `password` with the immediate values in the `$rdx` register.

Adding each byte value to the string `password` results in the string `sfuwxoue` which, when we give it to the binary is revealed to be correct:

```bash
johnny@johnny-VirtualBox:~/reversing/crackmes$ ./a.out sfuwxoue
Yes, sfuwxoue is correct!
```
