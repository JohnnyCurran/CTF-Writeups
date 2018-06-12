## Crackme0x02

Far too in-depth line-by-line write-up of reverse engineering Crackme0x02 from [this repo](https://github.com/leotindall/crackmes)

The crackme's are also hosted in this repo in the crackmes directory

The [tutorial](https://leotindall.com/tutorial/an-intro-to-x86_64-reverse-engineering/) for this Crackme reviews a dissassembly that was different than the one I had when I ran it through objdump and radare2 both on my mac and a virtual linux box.

So, after loading up the binary in radare2 I went to work on figuring out the differences in my binary and the one presented in the tutorial. I did a quick overview of the tutorial text so admittedly I knew what I was looking for going in, but I still believe it was a good exercise due to the differences in the instructions that were executed.

### Disassembly with radare2 ###

To begin, I looked at the beginning block of the main function, or `entry0`. Below is the output from radare2 after executing `pdf@entry0`

```asm
0x100000e20      55             push rbp                   ; [00] -r-x section size 281 named 0.__TEXT.__text
0x100000e21      4889e5         mov rbp, rsp
0x100000e24      4883ec30       sub rsp, 0x30              ; '0'
0x100000e28      c745fc000000.  mov dword [local_4h], 0
0x100000e2f      897df8         mov dword [local_8h], edi
0x100000e32      488975f0       mov qword [local_10h], rsi
0x100000e36      837df802       cmp dword [local_8h], 2    ; rdi ; [0x2:4]=-1
0x100000e3a      0f841d000000   je 0x100000e5d
```
In the above block, the stack frame pointer is pushed onto the stack, the stack pointer is set to the frame pointer, and 30 bytes are allocated to the stack. Then, 3 local variables are insantiated. The line that matters here is `mov dword [local_8h], edi`. This moves the lower 32 bits of the `RDI` register into the local variable `[local_8h]`. This value is then compared against the constant 2 in `cmp dword [local_8h], 2`. So what this line is *really* doing is this:

`cmp edi, 2`

Which is comparing the number of arguments passed to the function to the constant 2. The 2 arguments are the name of the program and the password. If there are not exactly 2 arguments (really just one), then the program will execute the following failure block below:

```asm
0x100000e40      488d3d130100.  lea rdi, str.Need_exactly_one_argument. ; section.3.__TEXT.__cstring ; 0x100000f5a ; "Need exactly one argument.\n"
0x100000e47      b000           mov al, 0
0x100000e49      e8ec000000     call sym.imp.printf        ; int printf(const char *format)
0x100000e4e      c745fcffffff.  mov dword [local_4h], 0xffffffff ; -1
0x100000e55      8945e0         mov dword [local_20h], eax
0x100000e58      e9d3000000     jmp 0x100000f30
```

The above block of assembly loads the failure string into `RDI`, then calls `printf` to print the failure message to the screen, and `jmp`s to the exit block.

After the first argument check is out of the way, the password-checking part of the program begins. This is really where the disassembly I was shown and the disassembly presented on the tutorial website began to diverge. Where the tutorial showed 4 lines of easy to dissect assembly, the version I had disassembled was slightly less clear:

```asm
0x100000e5d      488d05120100.  lea rax, str.password1     ; 0x100000f76 ; "password1"
0x100000e64      488945e8       mov qword [local_18h], rax
0x100000e68      c745e4000000.  mov dword [local_1ch], 0
0x100000e6f      31c0           xor eax, eax
0x100000e71      88c1           mov cl, al
0x100000e73      488b55e8       mov rdx, qword [local_18h]
0x100000e77      486375e4       movsxd rsi, dword [local_1ch]
0x100000e7b      0fbe0432       movsx eax, byte [rdx + rsi]
0x100000e7f      83f800         cmp eax, 0
0x100000e82      884ddf         mov byte [local_21h], cl
0x100000e85      0f841b000000   je 0x100000ea6
```

Starting at `0x100000e5d`, the memory address of `str.password1` is loaded in to `RAX`. Although this string seems suspicious at first glance, attempting to input it to the crackme results in a failure. 

The next two lines both declare local variables. The variable `[local_18h]` becomes a placeholder for `str.password1`'s location in memory, and `[local_1ch]` becomes a 16-bit variable with value 0.

Next, the lower 32 bits `EAX` of the `RAX` register are zeroed out by the XOR instruction.

Then, `al` is loaded into `cl`. Because `eax` was just zeroed out, `cl` now holds a value of 0.

`rdx` is then loaded with the memory address of `str.password` which is stored in `[local_18h]`

`movsxd rsi, dword [local_1ch]` is a very roundabout way of zeroing out the entire `rsi` register. It loads the 16-bit 0 value from the local variable and extends the sign through the rest of the 48 bits in the register.

`movsx eax, byte[rdx + rsi]` loads the first byte of the value at memory address `[rdx + rsi]` into `eax`. Because `rsi` is 0, and `rdx` contains the memory location of `str.password1`, this instruction became an *incredibly* long-winded way of performing the following 3 instructions.

```asm
lea rax, str.password1 ; 0x100000f76
mov rdx, rax
movsx eax, byte[rdx]
```

So - after all is said and done, `eax` contains the lowest byte of the value stored at memory location `rax`, or `0x10000f76` which we know contains the string `password1`. The first byte of this string is `p`.

The next instruction, `cmp eax,0` compares this value to 0. This comparison will of course always fail, as `p` will never be equal to `0`.

The next instruction `mov byte[local_21h], cl` moves the 0 value stored in cl into the first byte of local variable `[local_21h]`. 

After all of that confusing asm executed, a jump is taken/not taken based on the result of the `cmp eax, 0`. As noted, this comparison will always fail, so the jump will never be taken.

The assembly that is executed after the untaken jump is much more pertinent to the execution of the program.

```asm
0x100000e8b      488b45f0       mov rax, qword [local_10h]
0x100000e8f      488b4008       mov rax, qword [rax + 8]   ; [0x8:8]=-1 ; 8
0x100000e93      48634de4       movsxd rcx, dword [local_1ch]
0x100000e97      0fbe1408       movsx edx, byte [rax + rcx]
0x100000e9b      83fa00         cmp edx, 0
0x100000e9e      400f95c6       setne sil
0x100000ea2      408875df       mov byte [local_21h], sil
0x100000ea6      8a45df         mov al, byte [local_21h]
0x100000ea9      a801           test al, 1                 ; rsi
0x100000eab      0f8505000000   jne 0x100000eb6
0x100000eb1      e95a000000     jmp 0x100000f10
```

`mov rax, qword [local_10h]` Moves the value at `[local_10h]` into `RAX`. This is the value of the `rsi` register at the beginning of the program.

`move rax, qword [rax+8]` moves the memory value at `rax + 8` into the rax register. When this location in memory is examined, it turns out to be the attempted password argument we passed to the crackme. Now we're getting somewhere.

`movsxd rcx, dword [local_1ch]` zeroes out the `rcx` register

`movsx edx, byte [rax + rcx]` moves the first byte of the value at `[rax + rcx]` into `edx`. Because `rcx` was zeroed out, this instruction is essentially:
`movsx edx, byte [rax]` where, as we just saw, `rax` contains the string that we passed in as an argument to the function. Therefore, `dl` now contains the first letter of the password we input to the crackme.

`cmp edx, 0`. This instruction compares our first letter to the constant 0. Therefore, we know now that the first letter must be a 0. The next 3 instructions:

```asm
setne sil
mov byte [local_21h], sil
mov al, byte [local_21h]
```

Set the comparison flag and store it in a local variable as well as the lower 8 bits of the `rax` register, `al`. The result of the flag is then tested:

`test al, 1`. Test the result of the `cmp` against the value of 1. 

The unconditional `jmp` jumps to the success condition, so that is the jump we want to take. The `jne`/`jnz` will drop us into a block of asm with one more conditional check and a failure condition if it is not met. 

It was at this point that I patched the binary by writing a `jmp` to the `jne` instruction, so that the success condition is always hit. 

I saved the altered binary as crackme02_patch and the binary now accepts any password you pass to it:

```bash
./crackme02_patch wrongPassword
Yes, wrongPassword is correct!
```

If you're so inclined, you can get the patched binary under the [patched crackme's](/patchedCrackmes) directory of the repo
