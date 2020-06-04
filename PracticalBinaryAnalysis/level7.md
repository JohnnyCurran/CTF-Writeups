## Practical Binary Analysis Chapter 5 Level 7

Chapter 5 of Practical Binary Analysis by Dennis Andriesse contains several CTF challenges at the end of chapter 5.

All challenges are solved on a VM provided by the book.

### Getting Started

Hint for level 7:
```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 2e29c64a0f03a6ee2a307fecc8c3ff42 -h
  First observe my runtime state,
then watch closely while I replicate
```

The solution appears it will take close observation of dynamic runtime state

### Analysis

This binary differs from the others in that it is not immediately an executable but actually a compressed archive:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ file lvl7
lvl7: gzip compressed data, last modified: Sat Dec  1 17:30:15 2018, from Unix
```

Let's use the `file` utility with the `z` flag to take a look at what's inside the archive:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ file -z lvl7
lvl7: POSIX tar archive (GNU) (gzip compressed data, last modified: Sat Dec  1 17:30:15 2018, from Unix)
```

So if we unzip `lvl7`, we will obtain a tarball. Let's unzip it and see what we get:

```
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ tar xzvf lvl7
stage1
stage2.zip

binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ ls
lvl7  stage1  stage2.zip

binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ file stage1
stage1: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=ac71081a0951af729a4064c1dafbc5713b1537e3, stripped

binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ file stage2.zip
stage2.zip: Zip archive data, at least v2.0 to extract
```

The archive contained two separate files, stage1 and stage2.zip. Stage1 has a typical binary signature and stage2 is another zip archive. Looks like this is a two part challenge.

Getting started, let's try to unzip stage2.zip to see what we can get.

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ unzip stage2.zip 
Archive:  stage2.zip
[stage2.zip] tmp password: 
password incorrect--reenter: 
   skipping: tmp                     incorrect password
   skipping: stage2                  incorrect password
```

The archive is password-protected and we can't get into it. Even though we were unsuccessful, it does reveal some information about the archive: It appears to hold two files, `tmp`, and `stage2`. Presumably `stage1` contains the password to `stage2`.

### Stage 1

`strings`, `ltrace`, and `strace` don't reveal any interesting information about the binary. Executing stage1 produces no output, arguments or not.

Looking at the hex dump with `xxd` we stumble upon something interesting:

```asm
000005a0: 0100 0200 2053 2954 4111 47fa deff 4532  .... S)TA.G...E2
000005b0: 204b 458a 5900 0000 011b 033b 3400 0000   KE.Y......;4...
```

Looks like the string `STAGE2KEY` is in memory but separated by random bytes to prevent it from showing up `strings`. On a whim, I gave it a try at unlocking `stage2`:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ unzip stage2.zip 
Archive:  stage2.zip
[stage2.zip] tmp password: 
  inflating: tmp                     
  inflating: stage2    
```

Turns out `STAGE2KEY` was, in fact, the password. Let's take a closer look on how it was actually constructed inside of `stage1` by looking at the disassembly.

<pre>
isassembly of section .text:
00000000004003e0 <.text>:
  <b>4003e0:	ba a4 05 40 00       	mov    edx,0x4005a4</b>
  4003e5:	0f 1f 00             	nop    DWORD PTR [rax]
  4003e8:	0f be 02             	movsx  eax,BYTE PTR [rdx]
  4003eb:	83 f8 30             	cmp    eax,0x30
  4003ee:	7c 12                	jl     400402 <__libc_start_main@plt+0x42>
  4003f0:	83 f8 5a             	cmp    eax,0x5a
  4003f3:	7f 0d                	jg     400402 <__libc_start_main@plt+0x42>
  4003f5:	eb 09                	jmp    400400 <__libc_start_main@plt+0x40>
  4003f7:	64 75 6d             	fs jne 400467 <__libc_start_main@plt+0xa7>
  4003fa:	70 20                	jo     40041c <__libc_start_main@plt+0x5c>
  4003fc:	65 63 78 00          	movsxd edi,DWORD PTR gs:[rax+0x0]
  400400:	89 c1                	mov    ecx,eax
  <b>400402:	48 83 c2 01          	add    rdx,0x1
  400406:	48 81 fa b5 05 40 00 	cmp    rdx,0x4005b5
  40040d:	75 d9                	jne    4003e8 <__libc_start_main@plt+0x28></b>
  40040f:	31 c0                	xor    eax,eax
  400411:	c3                   	ret 
</pre>

At `0x4003e0`, the memory location for `S` in `STAGE2KEY`, `0x4005a4` is loaded into `$edx`. The first byte is then loaded into `$eax`. This byte value is then compared to make sure it is greater than `0` (`0x30`), and less than `Z` (`0x5a`).

This is equivalent to (in pseudo-code):
`if (char c < 0 || char c > 'Z')`

The significance of this is that while the program is walking memory, it takes only ASCII characters 0-9 and [aA]-[zZ]. This means all invalid ASCII characters and special characters such as `)` (at location `0x4005a5`) are skipped.

If a byte value does not fit inside the specified range, execution is transferred to `0x400402` where the valid character is loaded into `ecx`, memory index is incremented by 1, and then checked to see if we have reached the end of the string (`Y` at `0x4005b5`).

Let's move on to stage 2.

### Stage 2

Analyzing `tmp` and `stage2` with `file` show they are both executable binaries.

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ file tmp
tmp: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=377416069d2f4238cb934608d3e580a1df3d0b58, stripped

binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ file stage2
stage2: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=377416069d2f4238cb934608d3e580a1df3d0b58, stripped
```

Something interesting happens when we execute the binary. It outputs its own source code! This behavior is an example of a [quine](https://en.wikipedia.org/wiki/Quine_(computing)). A quine is a program that, when executed, outputs its own source.


(Formatting & indentation applied to make it easier to read)
```c++
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven$ ./stage2
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

int main()
{
    std::vector<char> hex;
    char q[] = "#include <stdio.h>\n#include <string.h>\n#include <vector>\n#include <algorithm>\n\nint main()\n{\nstd::vector<char> hex;\nchar q[] = \"%s\";\nint i, _0F;\nchar c, qc[4096];\n\nfor(i = 0; i < 32; i++) for(c = '0'; c <= '9'; c++) hex.push_back(c);\nfor(i = 0; i < 32; i++) for(c = 'A'; c <= 'F'; c++) hex.push_back(c);\nstd::srand(55);\nstd::random_shuffle(hex.begin(), hex.end());\n\n_0F = 0;\nfor(i = 0; i < strlen(q); i++)\n{\nif(q[i] == 0xa)\n{\nqc[_0F++] = 0x5c;\nqc[_0F] = 'n';\n}\nelse if(q[i] == 0x22)\n{\nqc[_0F++] = 0x5c;\nqc[_0F] = 0x22;\n}\nelse if(!strncmp(&q[i], \"0F\", 2) && (q[i-1] == '_' || i == 545))\n{\nchar buf[3];\nbuf[0] = q[i];\nbuf[1] = q[i+1];\nbuf[2] = 0;\nunsigned j = strtoul(buf, NULL, 16);\nqc[_0F++] = q[i++] = hex[j];\nqc[_0F] = q[i] = hex[j+1];\n}\nelse qc[_0F] = q[i];\n_0F++;\n}\nqc[_0F] = 0;\n\nprintf(q, qc);\n\nreturn 0;\n}\n";
    int i, _0F;
    char c, qc[4096];

    for(i = 0; i < 32; i++) for(c = '0'; c <= '9'; c++) hex.push_back(c);
    for(i = 0; i < 32; i++) for(c = 'A'; c <= 'F'; c++) hex.push_back(c);
    std::srand(55);
    std::random_shuffle(hex.begin(), hex.end());

    _0F = 0;
    for(i = 0; i < strlen(q); i++)
    {
	if(q[i] == 0xa)
	{
	    qc[_0F++] = 0x5c;
	    qc[_0F] = 'n';
	}
	else if(q[i] == 0x22)
	{
	    qc[_0F++] = 0x5c;
	    qc[_0F] = 0x22;
	}
	else if(!strncmp(&q[i], "0F", 2) && (q[i-1] == '_' || i == 545))
	{
	    char buf[3];
	    buf[0] = q[i];
	    buf[1] = q[i+1];
	    buf[2] = 0;
	    unsigned j = strtoul(buf, NULL, 16);
	    qc[_0F++] = q[i++] = hex[j];
	    qc[_0F] = q[i] = hex[j+1];
	}
	else qc[_0F] = q[i];
	_0F++;
    }
    qc[_0F] = 0;

    printf(q, qc);

    return 0;
}
```

We can compile this source and see if anything changes - "watch closely while I replicate" as the hint told us.

There is a change in the output, but it is subtle:

```c++
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven/quines$ g++ src.cc

binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven/quines$ ./a.out 
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

int main()
{
    std::vector<char> hex;
    char q[] = "#include <stdio.h>\n#include <string.h>\n#include <vector>\n#include <algorithm>\n\nint main()\n{\nstd::vector<char> hex;\nchar q[] = \"%s\";\nint i, _25;\nchar c, qc[4096];\n\nfor(i = 0; i < 32; i++) for(c = '0'; c <= '9'; c++) hex.push_back(c);\nfor(i = 0; i < 32; i++) for(c = 'A'; c <= 'F'; c++) hex.push_back(c);\nstd::srand(55);\nstd::random_shuffle(hex.begin(), hex.end());\n\n_25 = 0;\nfor(i = 0; i < strlen(q); i++)\n{\nif(q[i] == 0xa)\n{\nqc[_25++] = 0x5c;\nqc[_25] = 'n';\n}\nelse if(q[i] == 0x22)\n{\nqc[_25++] = 0x5c;\nqc[_25] = 0x22;\n}\nelse if(!strncmp(&q[i], \"25\", 2) && (q[i-1] == '_' || i == 545))\n{\nchar buf[3];\nbuf[0] = q[i];\nbuf[1] = q[i+1];\nbuf[2] = 0;\nunsigned j = strtoul(buf, NULL, 16);\nqc[_25++] = q[i++] = hex[j];\nqc[_25] = q[i] = hex[j+1];\n}\nelse qc[_25] = q[i];\n_25++;\n}\nqc[_25] = 0;\n\nprintf(q, qc);\n\nreturn 0;\n}\n";
    int i, _25;
    char c, qc[4096];

    for(i = 0; i < 32; i++) for(c = '0'; c <= '9'; c++) hex.push_back(c);
    for(i = 0; i < 32; i++) for(c = 'A'; c <= 'F'; c++) hex.push_back(c);
    std::srand(55);
    std::random_shuffle(hex.begin(), hex.end());

    _25 = 0;
    for(i = 0; i < strlen(q); i++)
    {
	if(q[i] == 0xa)
	{
	    qc[_25++] = 0x5c;
	    qc[_25] = 'n';
	}
	else if(q[i] == 0x22)
	{
	    qc[_25++] = 0x5c;
	    qc[_25] = 0x22;
	}
	else if(!strncmp(&q[i], "25", 2) && (q[i-1] == '_' || i == 545))
	{
	    char buf[3];
	    buf[0] = q[i];
	    buf[1] = q[i+1];
	    buf[2] = 0;
	    unsigned j = strtoul(buf, NULL, 16);
	    qc[_25++] = q[i++] = hex[j];
	    qc[_25] = q[i] = hex[j+1];
	}
	else qc[_25] = q[i];
	_25++;
    }
    qc[_25] = 0;

    printf(q, qc);

    return 0;
}
```

The 2 character hex value has changed from `0F` to `25` -- Let's compile this new source and see if it changes yet again:


```c++
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven/quines$ ./a.out > src2.cc
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven/quines$ g++ src2.cc 
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven/quines$ ./a.out
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>

int main()
{
std::vector<char> hex;
char q[] = "#include <stdio.h>\n#include <string.h>\n#include <vector>\n#include <algorithm>\n\nint main()\n{\nstd::vector<char> hex;\nchar q[] = \"%s\";\nint i, _E5;\nchar c, qc[4096];\n\nfor(i = 0; i < 32; i++) for(c = '0'; c <= '9'; c++) hex.push_back(c);\nfor(i = 0; i < 32; i++) for(c = 'A'; c <= 'F'; c++) hex.push_back(c);\nstd::srand(55);\nstd::random_shuffle(hex.begin(), hex.end());\n\n_E5 = 0;\nfor(i = 0; i < strlen(q); i++)\n{\nif(q[i] == 0xa)\n{\nqc[_E5++] = 0x5c;\nqc[_E5] = 'n';\n}\nelse if(q[i] == 0x22)\n{\nqc[_E5++] = 0x5c;\nqc[_E5] = 0x22;\n}\nelse if(!strncmp(&q[i], \"E5\", 2) && (q[i-1] == '_' || i == 545))\n{\nchar buf[3];\nbuf[0] = q[i];\nbuf[1] = q[i+1];\nbuf[2] = 0;\nunsigned j = strtoul(buf, NULL, 16);\nqc[_E5++] = q[i++] = hex[j];\nqc[_E5] = q[i] = hex[j+1];\n}\nelse qc[_E5] = q[i];\n_E5++;\n}\nqc[_E5] = 0;\n\nprintf(q, qc);\n\nreturn 0;\n}\n";
int i, _E5;
char c, qc[4096];

for(i = 0; i < 32; i++) for(c = '0'; c <= '9'; c++) hex.push_back(c);
for(i = 0; i < 32; i++) for(c = 'A'; c <= 'F'; c++) hex.push_back(c);
std::srand(55);
std::random_shuffle(hex.begin(), hex.end());

_E5 = 0;
for(i = 0; i < strlen(q); i++)
{
if(q[i] == 0xa)
{
qc[_E5++] = 0x5c;
qc[_E5] = 'n';
}
else if(q[i] == 0x22)
{
qc[_E5++] = 0x5c;
qc[_E5] = 0x22;
}
else if(!strncmp(&q[i], "E5", 2) && (q[i-1] == '_' || i == 545))
{
char buf[3];
buf[0] = q[i];
buf[1] = q[i+1];
buf[2] = 0;
unsigned j = strtoul(buf, NULL, 16);
qc[_E5++] = q[i++] = hex[j];
qc[_E5] = q[i] = hex[j+1];
}
else qc[_E5] = q[i];
_E5++;
}
qc[_E5] = 0;

printf(q, qc);

return 0;
}
```

The byte has changed again - this time to `E5`. Let's write a quick script to compile all of these files back to back and concatenate the flag:

```bash
!/bin/bash
flag="0F"
for i in {1..15}
do
  g++ "src${i}.cc"
  ./a.out > "src$((i + 1)).cc"
  #echo "compiled"
  line=$(awk 'NR==18 {print $1}' "src$((i + 1)).cc")
  #echo "awk done"
  flag="${flag}${line:1}"
done

echo $flag
```

And when we run it:

```bash
binary@binary-VirtualBox:~/crackmes/chapter5/lvlSeven/quines$ ./compileAll 
0F25E512A7763EEFB7696B3AEDA1F964
```

Giving that value to our oracle unlocks level 8!

```bash
binary@binary-VirtualBox:~/crackmes/chapter5$ ./oracle 0F25E512A7763EEFB7696B3AEDA1F964
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
| Level 7 completed, unlocked lvl8         |
+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
Run oracle with -h to show a hint
```
