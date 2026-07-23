## M2C Modula-2 Compiler & Translator Project ##
Welcome to the Wiki of the M2C Modula-2 Compiler & Translator Project

### Objective ###

The objective of this project is to develop a highly portable, reliable, readable and maintainable multi-dialect Modula-2 to C translator and via-C compiler that generates authentic and readable C source code.

### Purpose ###

The intended purpose for the M2C compiler is to provide a means:

* to compile and run program examples from early Modula-2 books (PIM3/PIM4), in particular works by Wirth.
* to cross-compile, bootstrap and deploy [M2BSK](https://github.com/m2sf/m2bsk), a Modula-2 hosted compiler for the [BSK subset of Modula-2 R10](https://github.com/m2sf/PDFs/blob/master/M2BSK%20Language%20Description.pdf).

### Dialects ###

The M2C compiler shall support the classic Modula-2 language described in the third and fourth editions of "Programming in Modula-2" (Wirth, 1983-1985) and selected features of the revised Modula-2 language described in "Modula-2 Revision 2010" (Kowarsch and Sutcliffe, 2010-2015). For details, see section [Dialects & Language](https://github.com/m2sf/m2c/wiki/Dialects-&-Language). The ISO Modula-2 dialect will **not** be supported.

### Grammar ###

The grammar of the compiler is in the project repository

https://github.com/trijezdci/m2c/blob/master/m2c-grammar.gll

For a graphical representation of the grammar, see section [Syntax Diagrams](https://github.com/m2sf/m2c/wiki/Syntax-Diagrams).

### License ###

The M2C compiler is licensed under the GNU Lesser General Public License (LGPL) both v.2.1 and v.3.

### Prerequisites ###

The M2C compiler is written in C99 and therefore a C99 compiler is required to build it. There are no dependencies on any third party libraries.

### OS support ###

The M2C compiler is supported on the following operating systems:
* AmigaOS
* BSD
* Linux
* MacOS
* MS-DOS
* OpenVMS
* Windows
* Solaris

### IDE support ###

The M2C compiler has been integrated with Vim.

### Contact ###

If you have questions or would like to contribute to the project, get in touch via

* [Modula2 Telegram group](https://t.me/+hTKSWC2mWoM1OGVl) chat

* [email](mailto:REMOVE+REVERSE.com.gmail@trijezdci) to the project maintainer

+++
