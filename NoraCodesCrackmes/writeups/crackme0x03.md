## Crackme0x03

All crackmes are sourced from [this repo](https://github.com/leotindall/crackmes)

Analysis of binary performed on a VirtualBox Ubuntu 18.04 VM

The crackmes are provided with source which means symbols are not stripped when we are analyzing them. This is a helpful, yet atypical, benefit of being able to compile the crackme binaries

### Getting Started ###

Let's start with a `strings` call on the binary (some omitted for brevity):

<pre>
johnny@johnny-VirtualBox:~/reversing/crackmes$ strings crackme03
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
<b>lAmBf</b>
AWAVI
AUATL
[]A\A]A^A&#5;
Need exactly one argument.
Yes, %s is correct!
No, %s is not correct.
;&#42;3$"
........
<b>check&#5;pw</b>
</pre>

`strings` reveals some things to take a closer look at. There's presumably a length requirement or check on the password due to the presence of `strlen`. The `lAmBf` string is a little suspicious since it doesn't appear to be random noise in the hex dump. Finally, there's a reference to `check_pw` which is presumably a function name and one we definitely want to look at further.

### Analysis

As expected, our binary expects exactly one argument which is checked against a password:

```bash
johnny@johnny-VirtualBox:~/reversing/crackmes$ ./crackme03
Need exactly one argument.

johnny@johnny-VirtualBox:~/reversing/crackmes$ ./crackme03 password
No, password is not correct.
```

Let's look at the disassembly:
<pre>
johnny@johnny-VirtualBox:~/reversing/crackmes$ objdump -M intel -d crackme03
00000000000007cb <main>:
 7cb:	55                   	push   rbp
 7cc:	48 89 e5             	mov    rbp,rsp
 7cf:	48 83 ec 30          	sub    rsp,0x30
 7d3:	89 7d dc             	mov    DWORD PTR [rbp-0x24],edi
 7d6:	48 89 75 d0          	mov    QWORD PTR [rbp-0x30],rsi
 7da:	64 48 8b 04 25 28 00 	mov    rax,QWORD PTR fs:0x28
 7e1:	00 00 
 7e3:	48 89 45 f8          	mov    QWORD PTR [rbp-0x8],rax
 7e7:	31 c0                	xor    eax,eax
 <b>7e9:	83 7d dc 02          	cmp    DWORD PTR [rbp-0x24],0x2
 7ed:	74 16                	je     805 <main+0x3a>
 7ef:	48 8d 3d 6e 01 00 00 	lea    rdi,[rip+0x16e]        # 964 <&#5;IO&#5;stdin&#5;used+0x4>
 7f6:	e8 e5 fd ff ff       	call   5e0 <puts@plt>
 7fb:	b8 ff ff ff ff       	mov    eax,0xffffffff
 800:	e9 bb 00 00 00       	jmp    8c0 <main+0xf5></b>
 805:	c7 45 ea 6c 41 6d 42 	mov    DWORD PTR [rbp-0x16],0x426d416c
 80c:	66 c7 45 ee 64 41    	mov    WORD PTR [rbp-0x12],0x4164
 812:	c6 45 f0 00          	mov    BYTE PTR [rbp-0x10],0x0
 816:	c7 45 f1 00 00 00 00 	mov    DWORD PTR [rbp-0xf],0x0
 81d:	66 c7 45 f5 00 00    	mov    WORD PTR [rbp-0xb],0x0
 823:	c6 45 f7 00          	mov    BYTE PTR [rbp-0x9],0x0
 827:	c6 45 f1 02          	mov    BYTE PTR [rbp-0xf],0x2
 82b:	c6 45 f2 03          	mov    BYTE PTR [rbp-0xe],0x3
 82f:	c6 45 f3 02          	mov    BYTE PTR [rbp-0xd],0x2
 833:	c6 45 f4 03          	mov    BYTE PTR [rbp-0xc],0x3
 837:	c6 45 f5 05          	mov    BYTE PTR [rbp-0xb],0x5
 83b:	48 8b 45 d0          	mov    rax,QWORD PTR [rbp-0x30]
 83f:	48 83 c0 08          	add    rax,0x8
 843:	48 8b 00             	mov    rax,QWORD PTR [rax]
 846:	48 89 c7             	mov    rdi,rax
 <b>849:	e8 a2 fd ff ff       	call   5f0 <strlen@plt>
 84e:	48 83 f8 06          	cmp    rax,0x6</b>
 852:	75 48                	jne    89c <main+0xd1>
 854:	48 8b 45 d0          	mov    rax,QWORD PTR [rbp-0x30]
 858:	48 83 c0 08          	add    rax,0x8
 85c:	48 8b 00             	mov    rax,QWORD PTR [rax]
 85f:	48 8d 55 f1          	lea    rdx,[rbp-0xf]
 863:	48 8d 4d ea          	lea    rcx,[rbp-0x16]
 867:	48 89 ce             	mov    rsi,rcx
 86a:	48 89 c7             	mov    rdi,rax
 <b>86d:	e8 c8 fe ff ff       	call   73a <check&#5;pw>
 872:	85 c0                	test   eax,eax
 874:	74 26                	je     89c <main+0xd1></b>
 876:	48 8b 45 d0          	mov    rax,QWORD PTR [rbp-0x30]
 87a:	48 83 c0 08          	add    rax,0x8
 87e:	48 8b 00             	mov    rax,QWORD PTR [rax]
 881:	48 89 c6             	mov    rsi,rax
 884:	48 8d 3d f4 00 00 00 	lea    rdi,[rip+0xf4]        # 97f <&#5;IO&#5;stdin&#5;used+0x1f>
 88b:	b8 00 00 00 00       	mov    eax,0x0
 890:	e8 7b fd ff ff       	call   610 <printf@plt>
 895:	b8 00 00 00 00       	mov    eax,0x0
 89a:	eb 24                	jmp    8c0 <main+0xf5>
 <b>89c:	48 8b 45 d0          	mov    rax,QWORD PTR [rbp-0x30]
 8a0:	48 83 c0 08          	add    rax,0x8
 8a4:	48 8b 00             	mov    rax,QWORD PTR [rax]
 8a7:	48 89 c6             	mov    rsi,rax
 8aa:	48 8d 3d e3 00 00 00 	lea    rdi,[rip+0xe3]        # 994 <&#5;IO&#5;stdin&#5;used+0x34></b>
 8b1:	b8 00 00 00 00       	mov    eax,0x0
 8b6:	e8 55 fd ff ff       	call   610 <printf@plt>
 8bb:	b8 01 00 00 00       	mov    eax,0x1
 8c0:	48 8b 4d f8          	mov    rcx,QWORD PTR [rbp-0x8]
 8c4:	64 48 33 0c 25 28 00 	xor    rcx,QWORD PTR fs:0x28
 8cb:	00 00 
 8cd:	74 05                	je     8d4 <main+0x109>
 8cf:	e8 2c fd ff ff       	call   600 <&#5;&#5;stack&#5;chk&#5;fail@plt>
 8d4:	c9                   	leave  
 8d5:	c3                   	ret    
 8d6:	66 2e 0f 1f 84 00 00 	nop    WORD PTR cs:[rax+rax&#42;1+0x0]
 8dd:	00 00 00 
</pre>

There are 4 sections the `main` method (in bold) that are worth taking a look at further.

The first is the argument comparison starting at `0x7e9`. The number of arguments passed to the program is assigned to `$rdi`, loaded into the stack on local variable `[rbp-0x24]`, and checked against immediate value `0x02` to make sure there was exactly 1 argument passed to the program (The first argument is the name of the program, the second is the password we pass to the binary).
If the comparison succeeds, the jump is taken and program execution continues at `0x805` otherwise failure code -1 is loaded into `$rax`, the "one argument needed" error message is printed to the console, and program execution returns and exits.

The second section of interest, when the exactly 1 argument requirement is satisfied, is the string comparison at `0x849`. The `main` method calls out to `strlen`, compares the length of the string to the immediate value `0x06` and exits to failure if this condition is not met.

At this point we know the password to this crackme will be exactly 6 characters long.

The third section beginning at `0x86d` is the call to the `check_pw` method. This is where the password is presumably checked for correctness before the program exits with failure or success

The fourth and final interesting section is the success condition which begins at `0x89c`. The `jmp` to this section is taken when the `check_pw` method completes successfully.

Let's dig further into `check_pw` to see what the correct password should be by taking a look at the function's disassembly:

<pre>
000000000000073a <check&#5;pw>:
 73a:	55                   	push   rbp
 73b:	48 89 e5             	mov    rbp,rsp
 73e:	48 89 7d e8          	mov    QWORD PTR [rbp-0x18],rdi
 742:	48 89 75 e0          	mov    QWORD PTR [rbp-0x20],rsi
 746:	48 89 55 d8          	mov    QWORD PTR [rbp-0x28],rdx
 74a:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [rbp-0x4],0x0
 751:	c6 45 fa 01          	mov    BYTE PTR [rbp-0x6],0x1
 755:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
 758:	48 63 d0             	movsxd rdx,eax
 75b:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
 75f:	48 01 d0             	add    rax,rdx
 762:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 765:	89 c1                	mov    ecx,eax
 767:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
 76a:	48 63 d0             	movsxd rdx,eax
 76d:	48 8b 45 d8          	mov    rax,QWORD PTR [rbp-0x28]
 771:	48 01 d0             	add    rax,rdx
 774:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 777:	01 c8                	add    eax,ecx
 779:	88 45 fb             	mov    BYTE PTR [rbp-0x5],al
 77c:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
 77f:	48 63 d0             	movsxd rdx,eax
 782:	48 8b 45 e8          	mov    rax,QWORD PTR [rbp-0x18]
 786:	48 01 d0             	add    rax,rdx
 789:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 78c:	38 45 fb             	cmp    BYTE PTR [rbp-0x5],al
 78f:	74 07                	je     798 <check&#5;pw+0x5e>
 791:	b8 00 00 00 00       	mov    eax,0x0
 796:	eb 31                	jmp    7c9 <check&#5;pw+0x8f>
 798:	83 45 fc 01          	add    DWORD PTR [rbp-0x4],0x1
 79c:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
 79f:	48 63 d0             	movsxd rdx,eax
 7a2:	48 8b 45 e0          	mov    rax,QWORD PTR [rbp-0x20]
 7a6:	48 01 d0             	add    rax,rdx
 7a9:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 7ac:	84 c0                	test   al,al
 7ae:	74 14                	je     7c4 <check&#5;pw+0x8a>
 7b0:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
 7b3:	48 63 d0             	movsxd rdx,eax
 7b6:	48 8b 45 e8          	mov    rax,QWORD PTR [rbp-0x18]
 7ba:	48 01 d0             	add    rax,rdx
 7bd:	0f b6 00             	movzx  eax,BYTE PTR [rax]
 7c0:	84 c0                	test   al,al
 7c2:	75 91                	jne    755 <check&#5;pw+0x1b>
 7c4:	b8 01 00 00 00       	mov    eax,0x1
 7c9:	5d                   	pop    rbp
 7ca:	c3                   	ret    
</pre>

`check_pw` takes 3 arguments - using `gdb` we were able to identify what they were:

```bash
(gdb) b check_pw
Breakpoint 5 at 0x55555555473e
(gdb) run
Starting program: /home/johnny/reversing/crackmes/crackme03 123456

Breakpoint 5, 0x000055555555473e in check_pw ()
(gdb) x/10i $rip-0x5
   0x555555554739 <frame_dummy+9>:	call   QWORD PTR [rbp+0x48]
   0x55555555473c <check_pw+2>:	mov    ebp,esp
=> 0x55555555473e <check_pw+4>:	mov    QWORD PTR [rbp-0x18],rdi
   0x555555554742 <check_pw+8>:	mov    QWORD PTR [rbp-0x20],rsi
   0x555555554746 <check_pw+12>:	mov    QWORD PTR [rbp-0x28],rdx
   0x55555555474a <check_pw+16>:	mov    DWORD PTR [rbp-0x4],0x0
   0x555555554751 <check_pw+23>:	mov    BYTE PTR [rbp-0x6],0x1
   0x555555554755 <check_pw+27>:	mov    eax,DWORD PTR [rbp-0x4]
   0x555555554758 <check_pw+30>:	movsxd rdx,eax
   0x55555555475b <check_pw+33>:	mov    rax,QWORD PTR [rbp-0x20]
(gdb) x/5x $rdx
0x7fffffffdf51:	0x02	0x03	0x02	0x03	0x05
(gdb) x/s $rdi
0x7fffffffe3b1:	"123456"
(gdb) x/s $rsi
0x7fffffffdf4a:	"lAmBdA"
```

`$rdi` holds the argument passed to the binary. In this case, `123456`

`$rsi` holds `lAmBdA`

`$rdx` holds an array containing values `[2, 3, 2, 3, 5]`

This function is basically a big loop that stretches from `0x755` to `0x7c2`

The assembly loads the local variables and juggles them around the registers. The interesting part of the code takes place at `0x777`:

`777:	01 c8                	add    eax,ecx`

The character `l` has been loaded into `$ecx` and the immediate value `2` has been loaded into `$eax`. This increases the value of `l` (`0x6e`) by 2 to `n` (`0x6c`).

On this next iteration of this, we see the letter `A` (`0x41`) is now in the `$ecx` register and the immediate value `3` is inside the `$eax` register. When added, this transforms `A` into `D` (`0x44`).

It appears this function is taking the array and applying a simple mask to the value `lAmBdA`. After adding the masked values to each character (with the final A adding a value of 0 because the array is only 5 elements long, keeping it as an A), we obtain the value `nDoEiA`.

Passing this value to our binary reveals it is the correct password:

```bash
johnny:crackmes$ ./crackme03 nDoEiA
Yes, nDoEiA is correct!
```
