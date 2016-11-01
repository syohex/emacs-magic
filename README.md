# magic

[libmagic](https://linux.die.net/man/3/libmagic) binding of Emacs Lisp.

## Requirements

- Emacs 25.1 or higher versions
- libmagic

## How to compile

```
cd emacs_source_code_directory
cd modules
git clone https://github.com/syohex/emacs-magic.git
cd emacs-magic
make
```

## Sample code

```lisp
(magic-file "/etc/shadow" '(none))
"regular file, no read permission"
```
