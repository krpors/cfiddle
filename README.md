# cfiddle

This repository contains some small projects where I fiddle around with C:

1. `daemon` - writing an old fashioned SysV Linux daemon using `fork()` and
   and others.
1. `hashtable` - a hashtable implementation using the djb2 hash algorithm.
    Currently just maps `char*` to `char*`.
1. `kbd` - proof of concept for capturing keyboard input events system-wide 
   (a.k.a. a keylogger).
1. `list` - generic (naive) linked list.
1. `mcast` - multicasting using POSIX socket cruft.

Usually they are just proof of concepts and I put them here before I forget how
I did things.
