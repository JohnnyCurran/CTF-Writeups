## Crackme03 patch

The patch applied to this binary allows the binary to accept any password as correct. It no longer checks the length of the string or the contents of the password

```bash
./crackme03_patch wrong
Yes, wrong is correct!

./crackme03_patch 123456
Yes, 123456 is correct!
```
