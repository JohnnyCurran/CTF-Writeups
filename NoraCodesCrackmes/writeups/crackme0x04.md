## Crackme0x04

All crackmes are sourced from [this repo](https://github.com/leotindall/crackmes)

The crackmes are provided with source which means symbols are not stripped when we are analyzing them. This is a helpful, yet atypical, benefit of being able to compile the crackme binaries

### Getting Started

Like the others, this crackme expects 1 argument. Unlike the others, `strings`, `ltrace`, and `strace` do not reveal any clues about a potential password.

Let's pop it into `radare2` to examine the `main` method and see what we can find out.

### Analysis

To begin, we load up the binary into `radare2` and auto analyze it, then examine all of the functions with `afl` (all functions list)

<pre>
johnny@johnny-VirtualBox:~/reversing/crackmes$ r2 -AA a.out
[x] Analyze all flags starting with sym. and entry0 (aa)
[x] Analyze len bytes of instructions for references (aar)
[x] Analyze function calls (aac)
[x] Emulate code to find computed references (aae)
[x] Analyze consecutive function (aat)
[x] Constructing a function name for fcn.&#42; and sym.func.&#42; functions (aan)
[x] Type matching analysis for all functions (afta)
[0x00000580]> afl
0x00000000    2 40           sym.imp.&#5;&#5;libc&#5;start&#5;main
0x00000528    3 23           sym.&#5;init
0x00000550    1 6            sym.imp.puts
0x00000560    1 6            sym.imp.printf
0x00000570    1 6            sub.&#5;&#5;cxa&#5;finalize&#5;248&#5;570
0x00000580    1 43           entry0
0x000005b0    4 50   -> 40   sym.deregister&#5;tm&#5;clones
0x000005f0    4 66   -> 57   sym.register&#5;tm&#5;clones
0x00000640    4 49           sym.&#5;&#5;do&#5;global&#5;dtors&#5;aux
0x00000680    1 10           entry1.init
0x0000068a   13 233          main
0x00000780    4 101          sym.&#5;&#5;libc&#5;csu&#5;init
0x000007f0    1 2            sym.&#5;&#5;libc&#5;csu&#5;fini
0x000007f4    1 9            sym.&#5;fini
</pre>


Nothing very interesting in here. We can see some calls to `printf`, `puts`, and some libc methods. Seems most of the functionality is going to take place in `main`. Let's take a look:

```asm
[0x00000580]> s main
[0x0000068a]> pdf
            ;-- main:
	    / (fcn) main 233
	    |   main ();
	    |           ; var int local_20h @ rbp-0x20
	    |           ; var int local_14h @ rbp-0x14
	    |           ; var int local_9h @ rbp-0x9
	    |           ; var int local_8h @ rbp-0x8
	    |           ; var int local_4h @ rbp-0x4
	    |              ; DATA XREF from 0x0000059d (entry0)
	    |           0x0000068a      55             push rbp
	    |           0x0000068b      4889e5         mov rbp, rsp
	    |           0x0000068e      4883ec20       sub rsp, 0x20
	    |           0x00000692      897dec         mov dword [local_14h], edi
	    |           0x00000695      488975e0       mov qword [local_20h], rsi
	    |           0x00000699      c645f700       mov byte [local_9h], 0
	    |           0x0000069d      837dec02       cmp dword [local_14h], 2    ; [0x2:4]=0x102464c
	    |       ,=< 0x000006a1      7416           je 0x6b9
	    |       |   0x000006a3      488d3d5a0100.  lea rdi, qword str.Need_exactly_one_argument. ; 0x804 ; "Need exactly one argument." ; const char * s
	    |       |   0x000006aa      e8a1feffff     call sym.imp.puts           ; int puts(const char *s)
	    |       |   0x000006af      b8ffffffff     mov eax, 0xffffffff         ; -1
	    |      ,==< 0x000006b4      e9b8000000     jmp 0x771
	    |      ||      ; JMP XREF from 0x000006a1 (main)
	    |      |`-> 0x000006b9      c745f8000000.  mov dword [local_8h], 0
	    |      |    0x000006c0      c745fc000000.  mov dword [local_4h], 0
	    |      |,=< 0x000006c7      eb20           jmp 0x6e9
	    |     .---> 0x000006c9      488b45e0       mov rax, qword [local_20h]
	    |     :||   0x000006cd      4883c008       add rax, 8
	    |     :||   0x000006d1      488b10         mov rdx, qword [rax]
	    |     :||   0x000006d4      8b45f8         mov eax, dword [local_8h]
	    |     :||   0x000006d7      4898           cdqe
	    |     :||   0x000006d9      4801d0         add rax, rdx                ; '('
	    |     :||   0x000006dc      0fb600         movzx eax, byte [rax]
	    |     :||   0x000006df      0fbec0         movsx eax, al
	    |     :||      ; DATA XREF from 0x00000709 (main)
	    |     :||   0x000006e2      0145fc         add dword [local_4h], eax
	    |     :||   0x000006e5      8345f801       add dword [local_8h], 1
	    |     :||      ; JMP XREF from 0x000006c7 (main)
	    |     :|`-> 0x000006e9      488b45e0       mov rax, qword [local_20h]
	    |     :|    0x000006ed      4883c008       add rax, 8
	    |     :|    0x000006f1      488b10         mov rdx, qword [rax]
	    |     :|    0x000006f4      8b45f8         mov eax, dword [local_8h]
	    |     :|    0x000006f7      4898           cdqe
	    |     :|    0x000006f9      4801d0         add rax, rdx                ; '('
	    |     :|    0x000006fc      0fb600         movzx eax, byte [rax]
	    |     :|    0x000006ff      84c0           test al, al
	    |     `===< 0x00000701      75c6           jne 0x6c9
	    |      |    0x00000703      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x3e0003
	    |      |,=< 0x00000707      7510           jne 0x719
	    |      ||   0x00000709      817dfce20600.  cmp dword [local_4h], 0x6e2 ; [0x6e2:4]=0x83fc4501
	    |     ,===< 0x00000710      7507           jne 0x719
	    |     |||   0x00000712      b801000000     mov eax, 1
	    |    ,====< 0x00000717      eb05           jmp 0x71e
	    |    ||||      ; JMP XREF from 0x00000710 (main)
	    |    |`-`-> 0x00000719      b800000000     mov eax, 0
	    |    | |       ; JMP XREF from 0x00000717 (main)
	    |    `----> 0x0000071e      8845f7         mov byte [local_9h], al
	    |      |    0x00000721      807df700       cmp byte [local_9h], 0
	    |      |,=< 0x00000725      7426           je 0x74d
	    |      ||   0x00000727      488b45e0       mov rax, qword [local_20h]
	    |      ||   0x0000072b      4883c008       add rax, 8
	    |      ||   0x0000072f      488b00         mov rax, qword [rax]
	    |      ||   0x00000732      4889c6         mov rsi, rax
	    |      ||   0x00000735      488d3de30000.  lea rdi, qword str.Yes___s_is_correct ; 0x81f ; "Yes, %s is correct!\n" ; const char * format
	    |      ||   0x0000073c      b800000000     mov eax, 0
	    |      ||   0x00000741      e81afeffff     call sym.imp.printf         ; int printf(const char *format)
	    |      ||   0x00000746      b800000000     mov eax, 0
	    |     ,===< 0x0000074b      eb24           jmp 0x771
	    |     |||      ; JMP XREF from 0x00000725 (main)
	    |     ||`-> 0x0000074d      488b45e0       mov rax, qword [local_20h]
	    |     ||    0x00000751      4883c008       add rax, 8
	    |     ||    0x00000755      488b00         mov rax, qword [rax]
	    |     ||    0x00000758      4889c6         mov rsi, rax
	    |     ||    0x0000075b      488d3dd20000.  lea rdi, qword str.No___s_is_not_correct. ; 0x834 ; "No, %s is not correct.\n" ; const char * format
	    |     ||    0x00000762      b800000000     mov eax, 0
	    |     ||    0x00000767      e8f4fdffff     call sym.imp.printf         ; int printf(const char *format)
	    |     ||    0x0000076c      b801000000     mov eax, 1
	    |     ||       ; JMP XREF from 0x0000074b (main)
	    |     ||       ; JMP XREF from 0x000006b4 (main)
	    |     ``--> 0x00000771      c9             leave
	    \           0x00000772      c3             ret
```

After passing our exactly 1 argument requirement at `0x69d`, execution passes to `0x6e9` and enters the main loop.
Interestingly, as the password argument we passed to the binary is iterated over, it is never compared to a "source of truth" value - only the end of the string is checked for at `0x6ff`.

It is after this "end-of-string" check that is most interesting to us in terms of the solution:

```asm
0x000006ff      84c0           test al, al
0x00000701      75c6           jne 0x6c9
0x00000703      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x3e0003
0x00000707      7510           jne 0x719
0x00000709      817dfce20600.  cmp dword [local_4h], 0x6e2 ; [0x6e2:4]=0x83fc4501
```

Here, we can see that the local variable [local\_8h] is compared to the immediate value `0x10`, or 16. Immediately after that, the local variable [local\_4h] is compared to immediate value `0x6e2`. What are these magic numbers and what do they mean?

Inside the main loop these values are only manipulated once apiece:

```asm
0x000006e2      0145fc         add dword [local_4h], eax
0x000006e5      8345f801       add dword [local_8h], 1
```

Each loop iteration, [local\_8h] is incremented by 1 and [local\_4h] adds the current value of `$eax`. local\_8h may hold the value of the current index while `$eax` may hold the byte value of each character of our password. A quick verification with `gdb` shows this to be true:

```bash
Breakpoint 10, 0x00005555555546e9 in main ()
(gdb) x/x $rbp-0x8
0x7fffffffdf58:	0x00
(gdb) c
Continuing.

Breakpoint 10, 0x00005555555546e9 in main ()
(gdb) x/x $rbp-0x8
0x7fffffffdf58:	0x01
(gdb) c
Continuing.

Breakpoint 10, 0x00005555555546e9 in main ()
(gdb) x/x $rbp-0x8
0x7fffffffdf58:	0x02
(gdb) x/x $rbp-0x8
0x7fffffffdf58:	0x02
```
So we can see that `local_8h` is our iterator. What about `local_4h`? We can check by setting a breakpoint on the `add` instruction and check the value of `$eax`. We execute the program with the password argument `0123456789aebcdef`:

```bash
Breakpoint 13, 0x00005555555546e2 in main ()
(gdb) info registers $rax
rax            0x30 48
(gdb) c
Continuing.

Breakpoint 13, 0x00005555555546e2 in main ()
(gdb) info registers $eax
eax            0x31 49
```
On these two iterations, the ASCII values of `0x30` and `0x31` are `0` and `1`, respectively. It looks safe to say that `local_4h` is incremented by the byte value of our password on each iteration.

So - what's the password? Let's revisit the final comparisons:

```asm
0x00000703      837df810       cmp dword [local_8h], 0x10  ; [0x10:4]=0x3e0003
0x00000707      7510           jne 0x719
0x00000709      817dfce20600.  cmp dword [local_4h], 0x6e2 ; [0x6e2:4]=0x83fc4501
```

`local_8h`, our iterator (length) value, is compared to `0x10`. Our password should have a length, then, of 16 characters.

`local_4h`, our accumulator (sum) value, is compared to `0x6e2`. What values can we pass to reach this amount? Some quick math gives us an answer:

`0x6e2 == 1,762`. `1,762 / 16 = 110.125`. No ASCII characters can have decimal values, so we will take `15 * 110 = 1,650`. That leaves us with `1,762 - 1,650 = 112`.

In hex, 110 is `0x6e`, or `n`. 112 is `0x70`, or `p`. Combined, these form the string:

`nnnnnnnnnnnnnnnp`

Let's give it to our binary:

```bash
johnny:crackmes$ ./a.out nnnnnnnnnnnnnnnp
Yes, nnnnnnnnnnnnnnnp is correct!
```

Now the question is, how many combinations of 0x6e2 can be made? To be continued
