## Dialect Selection ##

M2C supports the following Modula-2 dialects:
* following Wirth's 3rd Edition (PIM3)
* following Wirth's 4th Edition (PIM4)
* a PIM subset with language extensions (Ext)

#### Selecting PIM3 Mode ####
To select the PIM3 dialect, use ...

    m2c --pim3 sourcefile

#### Selecting PIM4 Mode ####
To select the PIM4 dialect, use ...

    m2c --pim4 sourcefile

#### Selecting Extended Mode ####
To select the extended dialect, use ...

    m2c --ext sourcefile

## Language Features of the PIM Dialects ##
When a PIM dialect is selected, certain outdated or unsafe language features specified in Wirth's language reports are disabled by default:
* synonyms
* octal literals
* cast syntax
* coroutines
* variant records
* local modules

Any of these features may be individually enabled using [compiler options](https://github.com/m2sf/m2c/wiki/Compiler-Options). For full PIM compliance use ...

    m2c --pim3 --compliant sourcefile
    m2c --pim4 --compliant sourcefile

#### Synonyms ####
In place of PIM synonyms `~` and `&`, reserved words `NOT` and `AND` should be used.
```modula-2
foo := NOT bar; (* in place of ~ *)
foo := bar AND baz; (* in place of & *)
```
In place of PIM synonym `<>`, symbol `#` should be used.
```modula-2
foo := bar # baz; (* in place of <> *)
```

#### Octal Literals ####
In place of octal character code literals, the built-in `CHR` function should be used.
```modula-2
CONST newLine = CHR(10); (* in place of 12C *)
```
In place of octal number literals, decimal number literals should be used.
```modula-2
CONST foo = 255; (* in place of 377B *)
```

#### Cast Syntax ####
In place of PIM cast syntax, the PIM dialect modes provide a `CAST` function which may be imported from module `SYSTEM`.

```modula-2
IMPORT SYSTEM;

foo := SYSTEM.CAST(FooType, bar); (* in place of FooType(bar) *)
bar := SYSTEM.CAST(BarType, foo); (* in place of BarType(foo) *)
```

#### Module Priority ####
Module priority syntax is recognised but always ignored.

#### Legacy Export ####
When compiler option `--legacy-export` is used, export directives in definition modules are permitted but ignored, allowing compilation of PIM2 sources in PIM3 or PIM4 dialect mode.

#### Concurrency ####

Alias type `PROC` is not defined. Coroutines are not implemented. This may or may not be implemented in the future.


## Language Features of the Extended Dialect ##

### Omissions ###
Outdated and unsafe PIM language features are omitted in the extended dialect:
* no synonyms
* no suffix literals
* no octal literals
* no unqualified import
* no implicit cast
* no variant records
* no module priority
* no local modules
* no `WITH` statement

### Changes ###
Some changes relative to PIM3 and PIM4 apply:
* the set difference operator is `\` instead of `-`
* opaque type definitions are marked by reserved word `OPAQUE`
* all arrays are zero-index based, the lowest index is always 0
* the `|` symbol within `CASE` statements is a branch prefix, not a separator
* pseudo-module `SYSTEM` has been renamed to `UNSAFE`
* cardinal types are not subrange types of integer types (same as in PIM3)
* `ARRAY OF CHAR` values are always terminated by ASCII NUL (same as in PIM4)
* global variables are always read-only when imported (recommended by PIM)
* various built-in functions and procedures have been moved or renamed or replaced


#### Zero-Index Based Arrays ####
All arrays are zero-index based. Array declarations do not specify an index range but the number of elements.
```modula-2
TYPE Str80 = ARRAY 80 OF CHAR; (* equivalent to ARRAY [0..79] OF CHAR in PIM3/4 dialect mode *)
```
In the above example, the lowest index of array type `Str80` is `0` and the highest is `79`. The index type is `LONGCARD`.

#### Changes in Built-in Functions and Procedures

PIM3/4                      | Ext                                  | Change
----------------------------|--------------------------------------|-------------
pervasive function `CAP`    | library function `Char.toUpper()`    | replaced
pervasive function `FLOAT`  | pervasive function `CONV`            | obsoleted
pervasive function `HIGH`   | pervasive function `COUNT`           | obsoleted
pervasive function `MAX`    | pervasive function `TMAX`            | renamed
pervasive function `MIN`    | pervasive function `TMIN`            | renamed
pervasive function `TRUNC`  | library function `RealMath.trunc()`  | replaced
pervasive function `SIZE`   | -                                    | removed
function `SYSTEM.TSIZE`     | pervasive function `TSIZE`           | moved
pervasive function `VAL`    | pervasive function `CONV`            | renamed
pervasive procedure `HALT`  | built-in `UNSAFE.HALT`               | moved
pervasive procedure `INC`   | postfix increment syntax `++`        | replaced
pervasive procedure `DEC`   | postfix decrement syntax `--`        | replaced
pervasive procedure `INCL`  | pervasive procedure `INSERT`         | renamed
pervasive procedure `EXCL`  | pervasive procedure `REMOVE`         | renamed

The return type of pervasive function `TSIZE` is `LONGCARD`, its result is given in octets

### Extensions ###
In addition to the basic PIM subset, the extended dialect supports a number of language extensions drawn from Oberon and Modula-2 R10 which are described below.
* line comments
* lowline character in identifiers
* prefix literals replacing suffix literals
* escaped tab and newline in string literals
* increment and decrement statements
* `CONST` parameters
* variadic parameters
* extensible record types
* indeterminate record types
* explicit typecast function `UNSAFE.CAST`
* additional pervasive functions and built-ins

#### Line Comments ####
In addition to block comments, the extended dialect supports Fortran-style line comments.
```fortran
! this is a line comment, terminating at the end-of-line
```

#### Lowline Character in Identifiers ####
The extended dialect supports the use of non-leading, non-trailing and non-consecutive lowline `_` in identifiers.
```modula-2
CONST ASCII_TAB = CHR(9);
```
This feature is disabled by default and may be enabled by compiler option `--lowline-identifiers`. It is primarily intended for use with foreign function interfaces. In some cases it may be useful to visibly separate an all-caps acronym from a preceding or following word or another all-caps acronym within a Modula-2 identifier.

#### Prefix Literals ####
In place of PIM-style suffix literals, the extended dialect uses C-style prefix literals.
```modula-2
CONST int = 0xFF00; (* whole number *)
CONST newLine = 0uA; (* character *)
```

#### Escaped Tab and Newline ####
The extended dialect supports C-style `\t` and `\n` escaped character codes within string literals.
```modula-2
CONST header = "Foo\tBar\tBaz";
CONST newLine = "This is the end of the line\n";
```

#### Increment and Decrement Statements ####
The extended dialect supports C-style postfix increment and decrement statements.
```modula-2
index++;
counter--;
```
Unlike C, the notation is only permitted in statements, not in expressions. 

#### CONST Parameters ####
The extended dialect supports the `CONST` attribute in formal types and formal parameters. Parameters marked with the `CONST` attribute are immutable within the procedure or function.
```modula-2
TYPE P = PROCEDURE ( CONST ARRAY OF CHAR );
PROCEDURE WriteString ( CONST s : ARRAY OF CHAR );
```

#### Variadic Parameters ####
The extended dialect supports the `ARGLIST` attribute in formal types and formal parameters. Parameters marked with the `ARGLIST ` attribute may be passed a variable number of arguments.
```modula-2
PROCEDURE newVector ( values : ARGLIST OF REAL ) : Vector;

VAR v1, v2, v3 : Vector;

v1 := newVector(1.2, 3.4); (* two arguments *)
v2 := newVector(1.2, 3.4, 5.6); (* three arguments *)
v3 := newVector(1.2, 3.4, 5.6, 7.8); (* four arguments *)
```
Within the procedure or function, the argument count may be obtained using built-in function `COUNT` and the arguments are addressable using array subscript notation.
```modula-2
PROCEDURE PrintList ( values : ARGLIST OF REAL );
VAR index : CARDINAL;
BEGIN
  FOR index := 0 TO COUNT(values)-1 DO
    WriteReal(values[index]);
    WriteLn
  END
END PrintList;
```

#### Extensible Record Types ####
In place of variant record types, the extended dialect provides Oberon-style extensible record types.
```modula-2
TYPE Base = RECORD ( NIL )
  (* compiler inserts hidden type tag *)
  foo : Foo
END;
```
In the above example, `NIL` is specified as the base type of the new record type. This tells the compiler that this type is intended as a base type to be extended in other declarations. As a result, the compiler inserts a hidden type tag field into its field list. By contrast, a record type declaration without a base type parameter will not receive a hidden type tag field and will not be extensible.

```modula-2
TYPE ExtBar = RECORD ( Base )
  (* inherits foo from Base *)
  bar : Bar
END;

TYPE ExtBaz = RECORD ( ExtBar )
  (* inherits foo and bar from ExtBar *)
  baz : Baz
END;
```
In the above example, type `ExtBar` is declared to be an extension of type `Base` and type `ExtBaz` is declared to be an extension of type `ExtBar`. An extension type inherits all the fields from its base type, including the hidden type tag field.
```modula-2
PROCEDURE bam ( x : Base );
BEGIN
  CASE x OF (* type: *)
  | ExtBar : x.bar := BarValue
  | ExtBaz : x.bar := BarValue; x.baz := BazValue
  END
END bam;
```
In the above example, the `CASE` statement is used to test the actual type of a record of an extensible type. The hidden type tag field of the record is used to determine its actual type at runtime. Only those fields that are present in the actual type are accessible. Addressing such fields outside of the appropriate case branch in a `CASE` statement raises a compile time error. This facility obsoletes Oberon type guard syntax.

#### Indeterminate Record Types ####
The extended dialect supports a type safe and bounds checked variant of C99-style variable length array members.
```modula-2
TYPE Buffer = POINTER TO RECORD
  (* field declarations may appear here *)
+ buffer (* indeterminate field *) : ARRAY size (* hidden field of type LONGCARD *) OF SomeType
END;
```
The example above is equivalent to the following C99 declaration:
```c
struct buffer_t {
  unsigned size;
  sometype_t buffer[];
}
```
However, in C the compiler does not associate the size field with the buffer array and consequently the array is not bounds checked. It is the responsibility of the C programmer to insert any such checks manually.

By contrast, M2C automatically inserts code to calculate the size for the buffer at allocation, write the value into the hidden `size` field. The buffer array is then automatically bounds checked against the value stored in the `size` field. Built-in function `COUNT()` may be used to obtain the value of the size field.
```modula-2
VAR b : Buffer;

NEW(b, 100); (* allocate buffer of size 100 *)

FOR i := 0 TO COUNT(b) - 1 DO
  b^.buffer[i] := someValue
END;

b^.size := 0; (* compile time error: no such field 'size' *)
```

#### Explicit Cast Function ####

In place of PIM cast syntax, the extended dialect provides a `CAST` function which may be imported from module `UNSAFE`.
```modula-2
IMPORT UNSAFE;

foo := UNSAFE.CAST(FooType, bar);
bar := UNSAFE.CAST(BarType, foo);
```

#### Additional Built-in Types ####
The extended dialect supports the following additional built-in types:
```modula-2
OCTET, LONGCARD;
```
Module `UNSAFE` further provides type `BYTE` and constant `BytesPerWord`.

#### Additional Built-in Functions ####

The extended dialect supports the following additional built-in functions:
```modula-2
SGN, ENTIER, PRED, SUCC, COUNT, LENGTH;
```
Further, module `UNSAFE` provides the following bit manipulation primitives:
```modula-2
SHL, SHR, BWNOT, BWAND, BWOR, BWXOR;
```


## Implementation Defined Features Common To All Dialects ##

#### Foreign Function Interface Pragmas ####
M2C supports foreign function interface pragmas `FFI` to declare a Modula-2 definition module as an interface for a foreign implementation module written in C and `FFIDENT` to map a Modula-2 identifier to a C library provided identifier.
```modula-2
(* Modula-2 interface for C library jurassic_flintstones *)
DEFINITION MODULE Flintstones <*FFI="C"*>
  <*FFIDENT="jurassic_flintstones"*>;

PROCEDURE Fred ( foo : INTEGER ) <*FFIDENT="flintstones_fred"*>;
(* => public void fred (int foo); *)

PROCEDURE wilma ( bar : CARDINAL ) : INTEGER <*FFIDENT="flintstones_wilma"*>;
(* => public int wilma (uint bar); *)
...
END Flintstones.
```

#### Custom Identifier Translation Pragma ####
M2C supports pragma `CNAME` to specify a user supplied translation of a Modula-2 identifier to a custom C identifier.
```modula-2
VAR fooBar <*CNAME="foobar"*> : BazType;
```
In the example above, Modula-2 identifier `fooBar` is translated to `foobar` in the generated C source code.


**Note** that pragma delimiters in PIM3/4 mode are `(*$` and `*)` instead of `<*` and `*>`.


#### Disabling Sections of Source Code ####

M2C supports special non-nesting tags `?<` and `>?` to temporarily and safely disable arbitrary sections of source code. The tags must always be used at the first column of a line. The compiler emits a warning for each disabled code section.
```modula-2
MODULE Foo;
?<
CONST delimiter = '*)';
>?
BEGIN
...
END Foo.
```
Please note that these tags are debugging aids. They should be removed again before the code is committed to a repository or published or shipped. They should not be used for commenting either. Hence the warnings.

#### Size of Type WORD ####

The size of built-in type `WORD` is implementation defined. In M2C it is defined as one octet.

#### Source File Types ####

M2C recognises source files according to their file types.

Source File            | File Types
-----------------------|------------------
Definition modules     | `.def` or `.DEF`
Implementation modules | `.mod` or `.MOD`
Program modules        | `.mod` or `.MOD`

+++
