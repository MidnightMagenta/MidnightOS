# MBuild object tree generator

## Syntax

An MBuild file is made of a sequence of list operation statments

A single list operation follows the below described syntax:

```
IDENT . OP? ( LIST ) QUALIFIERS? ;
```

IDENT defines the name of the list on which to perform the operation OP.

OP is an optional specifier that changes what will be done with the list. The default operation will cause the contents of the parameter list specified in the parenthases to be appended to the end of the list.

The following operations are available:

|operation|effect                                                                    |
|---------|--------------------------------------------------------------------------|
|add      | appends the contents of the parenthases to the end of the list           |
|set      | sets the contents of the list to the contents of the partenthases        |
|remove   | removes the entries specified in the parenthases from the list           |

The QUALIFIERS are additional options for adjusting the way the operation is performed.

The following qualifiers are available

|qualifier   |effect                                                                                                   |
|------------|---------------------------------------------------------------------------------------------------------|
|prepend_dir |prepends the path relative to the root file (the file which was scanned first) to each entry in the list |

The LIST in parenthases is a comma seperated list of entries.

## Directory inclusion

If an item in a list ends with a slash symbol (/) and a directory exists in the same directory as the file that defined it exists, an attempt to include a file named `MBuild` (not case sensitive) located in that direcotry will be made. If a directory contains multiple `MBuild` files, the compiler will show an error and stop compilations.

If the file or directory does not exist, the entry is ignored with a warning.