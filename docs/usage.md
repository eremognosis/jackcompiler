# Usage


## To Build


### Check Dependencies
```bash
    sudo apt update && sudo apt install build-essential cmake git
```

### Clone the repo (this part works for windows tho)
```bash 
    git clone https://github.com/eremognosis/jackcompiler
    cd jackcompiler
```

### make it
```bash
    mkdir build && cd build
    cmake ..
    make
```


## To use

Get the binary from releases or build it

To use in directory lets say `codebase`

```bash
    jack codebase
```

or in a file

```bash
    jack file.jack
```

The compiler writes both outputs next to each input file:
- `File.vm` (VM code)
- `File.xml` (syntax-tree XML)
