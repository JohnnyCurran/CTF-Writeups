## Crackme0x05

All crackmes are sourced from [this repo](https://github.com/leotindall/crackmes)

The crackmes are provided with source which means symbols are not stripped when we are analyzing them. This is a helpful, yet atypical, benefit of being able to compile the crackme binaries

### Getting Started

Like the others, this crackme expects a single argument: The correct password

```bash
johnny@johnny-VirtualBox:~/reversing/crackmes$ ./crack5
Need exactly one argument.

johnny@johnny-VirtualBox:~/reversing/crackmes$ ./crack5 password
No, password is not correct.
```

`strace` does not reveal anything interesting but `ltrace` with a password argument reveals a call to `strlen`. This tells us our password will most likely need to meet some minimum length.

### Disassembly

We start by loading up the binary into `radare2` and examining the `main` method

```asm
[0x00000776]> pdf@main
            ;-- main:
	    / (fcn) main 330
	    |   main ();
	    |           ; var int local_20h @ rbp-0x20
	    |           ; var int local_14h @ rbp-0x14
	    |           ; var int local_ch @ rbp-0xc
	    |           ; var int local_8h @ rbp-0x8
	    |              ; DATA XREF from 0x0000062d (entry0)
	    |           0x000007c9      55             push rbp
	    |           0x000007ca      4889e5         mov rbp, rsp
	    |           0x000007cd      4883ec20       sub rsp, 0x20
	    |           0x000007d1      897dec         mov dword [local_14h], edi
	    |           0x000007d4      488975e0       mov qword [local_20h], rsi
	    |           0x000007d8      837dec02       cmp dword [local_14h], 2    ; [0x2:4]=0x102464c
	    |       ,=< 0x000007dc      7416           je 0x7f4
	    |       |   0x000007de      488d3dec0100.  lea rdi, qword str.Need_exactly_one_argument. ; r8 ; 0x9d1 ; "Need exactly one argument." ; const char * s
	    |       |   0x000007e5      e8d6fdffff     call sym.imp.puts           ; int puts(const char *s)
	    |       |   0x000007ea      b8ffffffff     mov eax, 0xffffffff         ; -1
	    |      ,==< 0x000007ef      e91d010000     jmp 0x911
	    |      ||      ; JMP XREF from 0x000007dc (main)
	    |      |`-> 0x000007f4      488b45e0       mov rax, qword [local_20h]
	    |      |    0x000007f8      488b4008       mov rax, qword [rax + 8]    ; [0x8:8]=0
	    |      |    0x000007fc      488945f8       mov qword [local_8h], rax
	    |      |    0x00000800      488b45f8       mov rax, qword [local_8h]
	    |      |    0x00000804      bee8030000     mov esi, 0x3e8
	    |      |    0x00000809      4889c7         mov rdi, rax
	    |      |    0x0000080c      e8cffdffff     call sym.imp.strnlen
	    |      |    0x00000811      8945f4         mov dword [local_ch], eax
	    |      |    0x00000814      837df410       cmp dword [local_ch], 0x10  ; [0x10:4]=0x3e0003
	    |      |,=< 0x00000818      740c           je 0x826
	    |      ||   0x0000081a      488b45f8       mov rax, qword [local_8h]
	    |      ||   0x0000081e      4889c7         mov rdi, rax
	    |      ||   0x00000821      e822ffffff     call sym.fail
```

Looking at this section, we can see the `cmp` to check if we've passed the binary a password argument happening at `0x7d8`.

Next, at `0x80c` we can see the call to `strlen` which was shown in the `ltrace`. At `0x814`, the result of `strlen` is compared to the constant `0x10`, or `16`. This gives us our first clue about the password: It must be exactly 16 characters in length.

Let's examine what happens after the `strlen` check completes successfully:

```asm
	    |      ||      ; JMP XREF from 0x00000818 (main)
	    |      |`-> 0x00000826      488b45f8       mov rax, qword [local_8h]
	    |      |    0x0000082a      4883c002       add rax, 2
	    |      |    0x0000082e      0fb600         movzx eax, byte [rax]
	    |      |    0x00000831      3c42           cmp al, 0x42                ; 'B'
	    |      |,=< 0x00000833      740c           je 0x841
	    |      ||   0x00000835      488b45f8       mov rax, qword [local_8h]
	    |      ||   0x00000839      4889c7         mov rdi, rax
	    |      ||   0x0000083c      e807ffffff     call sym.fail
	    |      ||      ; JMP XREF from 0x00000833 (main)
	    |      |`-> 0x00000841      488b45f8       mov rax, qword [local_8h]
	    |      |    0x00000845      4883c00d       add rax, 0xd
	    |      |    0x00000849      0fb600         movzx eax, byte [rax]
	    |      |    0x0000084c      3c51           cmp al, 0x51                ; 'Q'
	    |      |,=< 0x0000084e      740c           je 0x85c
	    |      ||   0x00000850      488b45f8       mov rax, qword [local_8h]
	    |      ||   0x00000854      4889c7         mov rdi, rax
	    |      ||   0x00000857      e8ecfeffff     call sym.fail
```

Here, take a look at `0x831` and `0x84c`. At `0x831`, the password argument we supply to the binary is loaded into `$rax`, indexed at position 2, and compared with the constant `0x42` which `radare2` helpfully identifies for us as the ASCII character `B`.
If the character at position `2` is not the character `B`, the binary exits with a failure.

A similar comparison happens at `0x84c`. This time, the character at position `0xd` (13) is compared to the character `Q`. If it's not `Q`, our password is considered invalid and the binary exits.

Now we know the following 3 conditions about our password:

1. It must be exactly 16 characters in length
2. The 3rd character (index 2) must be the letter `B`
3. The 14th character (index 13) must be the letter `Q`

After these 3 checks are passed, the following call to `check_with_mod` appears 4 times, with different values passed into the argument registers `$rdi`, `$rsi`, and `$rdx` at each call. Let's take a look at the function and see if we can figure out what's going on.

```asm
|      |`-> 0x0000085c      488b45f8       mov rax, qword [local_8h]
|      |    0x00000860      ba03000000     mov edx, 3
|      |    0x00000865      be04000000     mov esi, 4
|      |    0x0000086a      4889c7         mov rdi, rax
|      |    0x0000086d      e804ffffff     call sym.check_with_mod
|      |    0x00000872      85c0           test eax, eax
|      |,=< 0x00000874      750c           jne 0x882
|      ||   0x00000876      488b45f8       mov rax, qword [local_8h]
|      ||   0x0000087a      4889c7         mov rdi, rax
|      ||   0x0000087d      e8c6feffff     call sym.fail
```

#### Analyzing check\_with\_mod

Let's take a look at the disassembly for the `check_with_mod` function:

```asm
[0x00000776]> pdf@sym.check_with_mod
/ (fcn) sym.check_with_mod 83
|   sym.check_with_mod ();
|           ; var int local_20h @ rbp-0x20
|           ; var int local_1ch @ rbp-0x1c
|           ; var int local_18h @ rbp-0x18
|           ; var int local_8h @ rbp-0x8
|           ; var int local_4h @ rbp-0x4
|              ; CALL XREF from 0x0000086d (main)
|              ; CALL XREF from 0x00000897 (main)
|              ; CALL XREF from 0x000008c1 (main)
|              ; CALL XREF from 0x000008eb (main)
|           0x00000776      55             push rbp
|           0x00000777      4889e5         mov rbp, rsp
|           0x0000077a      48897de8       mov qword [local_18h], rdi
|           0x0000077e      8975e4         mov dword [local_1ch], esi
|           0x00000781      8955e0         mov dword [local_20h], edx
|           0x00000784      c745f8000000.  mov dword [local_8h], 0
|           0x0000078b      c745fc000000.  mov dword [local_4h], 0
|       ,=< 0x00000792      eb1a           jmp 0x7ae
|       |      ; JMP XREF from 0x000007b4 (sym.check_with_mod)
|      .--> 0x00000794      8b45fc         mov eax, dword [local_4h]
|      :|   0x00000797      4863d0         movsxd rdx, eax
|      :|   0x0000079a      488b45e8       mov rax, qword [local_18h]
|      :|   0x0000079e      4801d0         add rax, rdx                ; '('
|      :|   0x000007a1      0fb600         movzx eax, byte [rax]
|      :|   0x000007a4      0fbec0         movsx eax, al
|      :|   0x000007a7      0145f8         add dword [local_8h], eax
|      :|   0x000007aa      8345fc01       add dword [local_4h], 1
|      :|      ; JMP XREF from 0x00000792 (sym.check_with_mod)
|      :`-> 0x000007ae      8b45fc         mov eax, dword [local_4h]
|      :    0x000007b1      3b45e4         cmp eax, dword [local_1ch]
|      `==< 0x000007b4      7cde           jl 0x794
|           0x000007b6      8b45f8         mov eax, dword [local_8h]
|           0x000007b9      99             cdq
|           0x000007ba      f77de0         idiv dword [local_20h]
|           0x000007bd      89d0           mov eax, edx
|           0x000007bf      85c0           test eax, eax
|           0x000007c1      0f94c0         sete al
|           0x000007c4      0fb6c0         movzx eax, al
|           0x000007c7      5d             pop rbp
\           0x000007c8      c3             ret
```

This function takes an offset, divides by an argument passed to it, and does modulo division to check the remainder. If the remainder is 0, we pass the check.


String length needs to be 16.

Execution then jumps to `0x826`. Then it chops off the first two characters of our password string, leaving `23456789abcdef`

Compares `2` with `B`. So position `2` should be char `B`.

It then compares position `d` (13) to the char `Q`. So we replace `d` in our string with `Q`.

Now we need to dig into this `check_with_mod` method.

rdi, rsi, rdx <-- argument order

RDI has pw string
rsi has 4 - Iterator
rdx has 3 - Divisor

`check_with_mod` method:

Loops 4 times (0-3).

It indexes the password string, then adds that byte value to local8h var.

Is idiv instruction using rdx, with value 3? So it's mod 3?

Sets return value to 1 if the remainder of the division was 0 (I think. Need to double check how the ZF is set on the idiv).

So is this gonna check different sections of the password with a modulo division? - Yes, it appears it does do that


First pass thru: `$eax` holds `0xd6` or `214` -- decimal values summed of `01B3` equal `214` so that was right.

Yes - divided by 3.

```bash
(gdb) 
eax            0x47 71
edx            0x1  1
```

`$eax` holds result of division which is `71` and `edx` holds remainder which is `1`.

We then check to see if `$eax` is `0` with `tst` instruction

We set `al` to 1 if `$eax` was 0. So we don't want a remainder?

`main` checks to see if return value `$eax` is `0` and jumps to the next comparison if the return value is NOT equal to 0.

So we want a clean division based on the byte values that it sums

Byte value of `216` will give us what we want. So we have `B` in there for sure. Let's increment a value by 2 to give us that value. Let's alter `0` at position `0` to `2`:

New password to pass the first check:

`21B3456789abcQef`

Str len 16: Takes every 4 char section and mods by a diff value - 3, 4, 5, 4. Index 2 must be 'B', index 0xD, 13, must be 'Q'.

Wrote a C# and C++ Program to be a keygen.
