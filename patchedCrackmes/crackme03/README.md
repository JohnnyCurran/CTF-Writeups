## Crackme03 patch

The patch applied to this binary allows the binary to accept any password as correct. It no longer checks the length of the string or the contents of the password.

```bash
./crackme03_patch wrong
Yes, wrong is correct!

./crackme03_patch 123456
Yes, 123456 is correct!
```

### How the patch was applied

In the linux binary, I achieved this by removing the `jmp` for the password length, and inside of the symbol `check_pw`, I forced `eax` to always be set to `1` by 

In the macOS binary, I changed the `cmp` instruction for password length to always compare against itself, resulting in an always successful check.  Then i `nop`d out the `jne` call to the failure condition. The resulting block of asm logic looks like:

```asm
0x100000eb4      4839c0         cmp rax, rax ; change cmp to always succeed
0x100000eb7      90             nop 
0x100000eb8      0f8543000000   jne 0x100000f01
0x100000ebe      488d55e2       lea rdx, [local_1eh]
0x100000ec2      488d75e9       lea rsi, [local_17h]
0x100000ec6      488b45f0       mov rax, qword [local_10h]
0x100000eca      488b7808       mov rdi, qword [rax + 8]   ; [0x8:8]=-1 ; 8
0x100000ece      e89dfeffff     call sym._check_pw
0x100000ed3      83f800         cmp eax, 0
0x100000ed6      90             nop ; nop check_pw jmp
0x100000ed7      90             nop
0x100000ed8      90             nop
0x100000ed9      90             nop
0x100000eda      90             nop
0x100000edb      90             nop
0x100000edc      488d3d9c0000.  lea rdi, str.Yes___s_is_correct ; 0x100000f7f ; "Yes, %s is correct!\n"
```
