
### Project 1 - Basic Computer System Simulator
- [✓] Module 1: CPU and Instruction Cycle
- [✓] Module 2: Memory Management
- [✓] Module 3: Process Scheduling and Multitasking
- [✓] Module 4: Interrupt Handling and Dispatcher
- [✓] Module 5: Multithreading or Forking
- [ ] Documentation

## To Install 
To compile this project, it is recommended to install `make`.

**If you are on mac you will need Homebrew**

Open up a terminal and run

```
xcode-select --install
```

That will install all the necessary tooling for this

Next run

```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

This command actually installs brew for you

You can verify it's installed by running

```
brew help
```

If it's installed properly you'll get output

If not you'll get "command not found"

### Install Make
- **Install Make on Windows**
  - open up powershell (not cmd)
  - run the following command to install make
    ```
    winget install ezwinports.make
    ```

- **Install Make on Linux**
    For Linux based systems use your prefered package manager

    For Arch based distrobutions run
    ```
    sudo pacman -S make
    ```

    For Debian/Ubunto distrobutions run
    ```
    sudo apt install make
    ```

    For Redhat/CentOS
    ```
    sudo dnf install make
    ```

    Or for earlier versions of Fedora
    ```
    sudo yum install make
    ```

- **Install Make on MacOS (using Homebrew)**
    ```
    brew install make
    ```

### Install gcc
    gcc is the GNU C Compiler and is necessary to run C

-   **If You're on windows, I can't help you with that**
    **So follow the instructions microsoft made for this**

-   [VSCode](https://code.visualstudio.com/docs/cpp/config-mingw)
    
    **GCC comes pr-installed on linux**

-   **Install gcc on MacOS**
    ```
    brew install gcc
    ```

### Now To run the project
    Go to the directory/folder where you have this saved and run
    ```
    make clean
    make
    ```
    (This will trigger the make file in the directory and
    create two executables named "demo" and "tests")
    run the executable(Either "demo" or "./demo")

-   **You can also use make to run the executables**

    To do so, run
    ```
    make run-demo

    ```
    To run just the demo, and conversely

    ```
    make run-test
    ```
    To run just the tests.

    If you would like to run all of them together, run

    ```
    make run-all
    ```
    If you choose you only want to build the demo or tests, run
    ```
    make demo
    ```
    and
    ```
    make tests
    ```
    respectively.

-   **For more help, you can run**
    ```
    make help
    ```
-   **For a list of commands to run**

Now you're all set. Have a good day professor

### Important Notes

-   **Instruction Format**
    Instructions use the following 32-bit format:
    ```
    [31:24] Opcode
    [23:20] Destination Register (DR)
    [19:16] Source Register 1 (SR1)
    [15:12] Mode (1=immediate, 0=register)
    [11:0]  Operand/Immediate Value
    ```

-   **Memory Adress Space**
    ```
    0x000-0x0FF: Program code
    0x100-0x1FF: Data section
    0x200-0x3FF: Process memory allocations
    ```

### Debugging Tips

**If the program doesn't compile make sure 
both make and gcc are installed (steps above)**

make sure pthread support is enabled on your machine

re-run ```make clean``` to remove any old object files
that may be causing linkage errors. Then do a base rebuild
with just ```make```

