This is an interpreter for the CHIP-8 system designed to run on ARM64, complete with a debugging UI that displays registers, stack, memory, keyboard inputs, and other customizable inputs. 

To build with CMake, navigate to the CHIP8_Interpreter directory and run:

```
cmake .
cmake --build .
```

This will produce an executable called 'chip8'.

The format for running this executable on a given rom is
```
./chip8 <path-to-rom-file>
```


