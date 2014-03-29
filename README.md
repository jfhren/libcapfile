libcapfile
==========

This library provides a way to read, modify, generate and write Java Card 2.2.2
CAP files and the associated export files.

A read CAP file is represented through a straightforward C structure
representation and can be them analyzed to fill a more easily tweak-able one.
From this tweaked C structure a new straightforward one can be generated and
them written to disk as a CAP file.

The intermediary representation allows one not to care about offset and token
computation, constant pool entries sorting and all the troublesome parts of a
Java Card CAP file.

This library can also read export files which are useful when fetching
external references information. Support for writing export file will be added
later since it is somewhat useful when tweaking a library (i.e. tokens can
change making linking with other CAP files impossible.

A CAP file being a zip file, this library is making use of `libzip` for this
purpose and should be linked with it.

A few tools are provided to dump a straightforward CAP file representation, an
analyzed one, a generated one or an export file. They are most useful when
debugging tweaking.
