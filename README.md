# Jack Compiler

This was the last part of the nand2tetris project
this is a jack compiler written in C
(update) NOW COMES WITH THE XML SYNTAX TREE THAT IS USELESS BUT STILL
# Usage (BUILD)
```bash
git clone https://github.com/eremognosis/jackcompiler
cd jackcompiler
mkdir build && cd build
cmake ..
make
```

This outputs the binary called `jack`

Check [this](docs/usage.md) for more details


Thw whole project can be found on https://nand2tetris.org

My implementation are in repos

The OS : [jackos](https://github.com/eremognosis/jackos)  
Compiler : [this](https://github.com/eremognosis/jackcompiler)   takes .jack ouputs .vm and syntax-tree .xml  (executable : `jack`)
VM : [vminc](https://github.com/eremognosis/vminc)    takes .vm ouputs .asm  (executable : `vmtrans`)
Assembler : [assembler](https://github.com/eremognosis/assembler)  takes .asm ouputs .hack  (executable : `assemble`)


# Credits
- Noam Nisan et. al
- Myself (@eremognosis)
- My cat


## LICENSE
GPL-3

## Contribution
PRs
