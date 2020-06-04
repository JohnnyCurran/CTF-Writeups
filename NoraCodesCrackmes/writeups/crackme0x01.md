## Crackme0x01

All crackmes are sourced from [this repo](https://github.com/leotindall/crackmes)

Analysis of binary performed on a VirtualBox Ubuntu 18.04 VM

The crackmes are provided with source which means symbols are not stripped when we are analyzing them. This is a helpful, yet atypical, benefit of being able to compile the crackme binaries

### Getting Started ###

After compiling, `file` utility shows us exactly what we expect. A 64-bit ELF binary

Running the binary complains about a lack of arguments

```bash
~: ./crackme01
Need exactly one argument.
```

While running with an argument reveals that the binary is presumably checking that the argument matches some password value in the binary.

```bash
~: ./crackme01 foo
No, foo is not correct.
```

### Analysis ###

A quick `strings` on the binary reveals several interesting looking options (many ommitted for brevity)

```bash
~: strings crackme01
Need exactly one argument.
password1
No, %s is not correct.
Yes, %s is correct
```

`password1` is certainly interesting and so we give it a crack as the binary argument:

```bash
~: ./crackme01 password1
Yes, password1 is correct!
```

And we've solved the crackme. I guess there's a reason this is level 1.
