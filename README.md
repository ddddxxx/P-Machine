#PL/0 Complier

[![No Maintenance Intended](http://unmaintained.tech/badge.svg)](http://unmaintained.tech/)

A simple PL/0 compiler and VM.

Language definition [PL/0 User's Manual](doc/PL0 User's Manual.pdf)

##Instructions

From the [project directory](Compiler/), run the following:

Build

```
$ make
```

Usage

```
$ ./compiler [options...] [filename]

    -t  generate abstract syntax tree file
    -c  generate pcode file
    -r  generate hex file for Logisim
    -o  optimize code
    -h  help
```

Automatic tests

```
$ make test
```

Clean up

```
$ make clean
```

#P-Machine Circuit

A logism project creates a processor to implement p-code instructions.

##Instructions

1. Open [CPU.circ](Circuit/CPU.circ) in logism.
2. Load hex file generated by complier above in ROM.
3. Activate the simulator, the last output will be presented.