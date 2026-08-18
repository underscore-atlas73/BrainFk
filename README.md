# BrainFk
A very simple (and maybe poorly made) BrainFk interpreter programmed in C. A compiler may be on the way.

Just made as a fun programming project.

## Usage
Linux: `./BrainFk {<BrainFk Code> | -f <Path to BrainFk Code File>} [-i <Input String> | -if <Path to Input File>]` <br>
Windows: `BrainFk.exe {<BrainFk Code> | -f <Path to BrainFk Code File>} [-i <Input String> | -if <Path to Input File>]`

The program will attempt to check for mismatched and unbalanced brackets, though no guarantees can be made.

### Examples
#### Echo back the provided string
`./BrainFk ",[.,]" -i "TEST STRING__"`
#### Print out the source code of the executable using the interpreter (use the provided Test.bf)
`./BrainFk -f Test.bf`
#### Print out the source code of the executable
`./BrainFk ",[.,]" -if main.c`

## Basic BrainFk Spec (credit to Urban Müller, 1993)
Originally directed towards AmigaOS/2.0 on the M68k family.
- '>' -- Move the memory pointer right by one cell.
- '<' -- Move the memory pointer left by one cell.
- '+' -- Add 1 to the value in the current cell.
- '-' -- Subtract 1 from the value in the current cell.
- '.' -- Output the ASCII character of the value in the current cell.
- ',' -- Read one character of input and store its value in the current cell.
- '[' -- Jump forward to the matching ']' if the current cell's value is 0.
- ']' -- Jump back to the matching '[' if the current cell's value is not 0.
- '#' -- Dump the entire 65536 cell tape to `mem_dump.txt`. Although not in the original compiler, it was mentioned as part of the debug interpreter.
