## Practical Binary Analysis Chapter 5 Level 3

Chapter 5 of Practical Binary Analysis by Dennis Andriesse contains several CTF challenges at the end of chapter 5.

All challenges are solved on a VM provided by the book.

### Getting Started

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 034fc4f6a536f2bf74f8d6d3816cdf88 -h
Fix four broken things
```

### Analysis

We check out the binary with the `file` utility:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ file lvl3
lvl3: ERROR: ELF 64-bit LSB executable, Motorola Coldfire, version 1 (Novell Modesto) error reading (Invalid argument)
```

Right away, we notice this file signature does not look correct. Notably, **Motorola Coldfire** and  **Novell Modesto** are not in line with what we'd expect to see.

Attempting to run the binary is unsuccessful:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ ./lvl3
bash: ./lvl3: cannot execute binary file: Exec format error
```

Let's check the ELF header and see what it tells us:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ readelf -h lvl3
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 0b 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            Novell - Modesto
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Motorola Coldfire
  Version:                           0x1
  Entry point address:               0x4005d0
  Start of program headers:          4022250974 (bytes into file)
  Start of section headers:          4480 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         9
  Size of section headers:           64 (bytes)
  Number of section headers:         29
  Section header string table index: 28
readelf: Error: Reading 0x1f8 bytes extends past end of file for program headers
```

In the header, we can see that **Version**, **Machine**, and **Start of program headers** are incorrect.

Taking a look at the header bytes reveals a little more information:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ xxd lvl3 | head
00000000: 7f45 4c46 0201 010b 0000 0000 0000 0000  .ELF............
00000010: 0200 3400 0100 0000 d005 4000 0000 0000  ..4.......@.....
00000020: dead beef 0000 0000 8011 0000 0000 0000  ................
00000030: 0000 0000 4000 3800 0900 4000 1d00 1c00  ....@.8...@.....
00000040: 0600 0000 0500 0000 4000 0000 0000 0000  ........@.......
00000050: 4000 4000 0000 0000 4000 4000 0000 0000  @.@.....@.@.....
00000060: f801 0000 0000 0000 f801 0000 0000 0000  ................
00000070: 0800 0000 0000 0000 0300 0000 0400 0000  ................
00000080: 3802 0000 0000 0000 3802 4000 0000 0000  8.......8.@.....
00000090: 3802 4000 0000 0000 1c00 0000 0000 0000  8.@.............
```

In e\_header byte 7, EI\_OSABI, is incorrectly marked as `0x0b` (OSABI\_MODESTO), when it should have a value of `0x00` (OSABI\_SYSV).

At byte `0x7` (EI\_OSABI bit in the ELF Header) it is incorrectly marked as `0x34` (Motorola Coldfire) when it should be `0x3e` (EM\_X86\_64)

At `0x20`, we can see the bytes `dead beef` which are causing the program header to hold an incorrect value. The size of the program header offset as defined by the ELF standard will always be `0x40` bytes.

To fix these issues with the program header, we fire up vim and pipe to xxd. Once we edit the bytes and save the file, our new `readelf` output is:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ readelf -h patched
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x4005d0
  Start of program headers:          64 (bytes into file)
  Start of section headers:          4480 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         9
  Size of section headers:           64 (bytes)
  Number of section headers:         29
  Section header string table index: 28

binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ xxd patched | head
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 3e00 0100 0000 d005 4000 0000 0000  ..>.......@.....
00000020: 4000 0000 0000 0000 8011 0000 0000 0000  @...............
00000030: 0000 0000 4000 3800 0900 4000 1d00 1c00  ....@.8...@.....
```

As we can see, our **Machine**, **Version**, and **Start of program headers** have been correctly fixed. The hex dump of the program header no longer contains bad values and we can execute our program:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ ./patched
04f2d253b3e6550d83269cf0c561aee5  ./patched
```

It appears the program is now outputting a flag, which, according to the `ltrace`, is the md5 sum of the program:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ ltrace ./patched
__libc_start_main(0x400550, 1, 0x7ffc26859368, 0x4006d0 <unfinished ...>
__strcat_chk(0x7ffc26858e60, 0x400754, 1024, 0)                                               = 0x7ffc26858e60
__strncat_chk(0x7ffc26858e60, 0x7ffc2685a349, 1016, 1024)                                     = 0x7ffc26858e60
system("md5sum ./patched"04f2d253b3e6550d83269cf0c561aee5  ./patched
```

Let's try giving the output of the program to the oracle:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 04f2d253b3e6550d83269cf0c561aee5
Invalid flag: 04f2d253b3e6550d83269cf0c561aee5
```

Our flag is invalid. There must be something we missed. According to the hint, there are four broken things we must fix. We've only fixed three (machine, version, program headers offset).

Let's disassemble the binary to see what's going on.

Right away, it's clear something is missing. There is no `.text` section in the disassembly:


```asm
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ objdump -M intel -d patched | grep "\."
Disassembly of section .init:
00000000004004c0 <.init>:
Disassembly of section .plt:
Disassembly of section .plt.got:
0000000000400540 <.plt.got>:
Disassembly of section .fini:
0000000000400744 <.fini>:
```

Looking at the section headers, the `.text` section contains a `NOBITS` flag when it should be marked `PROGBITS` (some sections omitted for brevity):

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ readelf --sections -W patched
There are 29 section headers, starting at offset 0x1180:

Section Headers:
[Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
...
[14] .text             NOBITS          0000000000400550 000550 0001f2 00  AX  0   0 16
```

In order to find the section header of .text, we need to calculate its offset. The formula is `sh_off + (sh_num * 64) + sh_type`, where 64 is the number of bytes the section header takes up. sh\_off is the size of the section header offset, and sh\_type is the number of bytes to skip in order to jump directly to the bytes we need to edit. Omitting sh\_type from the calculation will take us to the beginning of the section header.
For the .text section, that works out to be `4480 + (14 * 64) + 4 = 5380 = 0x1504`. Without the 4 byte offset for sh\_type, the start of the .text section header is located at `0x1500`.

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ xxd patched | grep 1500 | tail -n 1
00001500: 8d00 0000 0800 0000 0600 0000 0000 0000  ................
```

You can see byte `0x1504` contains the value `0x08`, SHT\_NOBITS, when the value should be `0x01`, SHT\_PROGBITS. This should be the final modification we need to make. After editing the binary with vim and xxd, we can see that `.text` contains the proper flag and shows up in the disassembly:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ readelf --sections -W newpatch 
There are 29 section headers, starting at offset 0x1180:

Section Headers:
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
.......
  [14] .text             PROGBITS        0000000000400550 000550 0001f2 00  AX  0   0 16
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ objdump -M intel -d newpatch | grep "\."
Disassembly of section .init:
00000000004004c0 <.init>:
Disassembly of section .plt:
Disassembly of section .plt.got:
0000000000400540 <.plt.got>:
Disassembly of section .text:
0000000000400550 <.text>:
Disassembly of section .fini:
0000000000400744 <.fini>:
```

Running the patched binary shows a new md5 hash which we pass to oracle:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvl3$ ./newpatch
3a5c381e40d2fffd95ba4452a0fb4a40  ./newpatch
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 3a5c381e40d2fffd95ba4452a0fb4a40
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
| Level 3 completed, unlocked lvl4         |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
Run oracle with -h to show a hint
```

On to level 4
