
### Project 1 - Basic Computer System Simulator
- [ ] Module 1: CPU and Instruction Cycle
- [ ] Module 2: Memory Management
- [ ] Module 3: Process Scheduling and Multitasking
- [ ] Module 4: Interrupt Handling and Dispatcher
- [ ] Module 5: Multithreading or Forking
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
    If by some off chance you have linux installed

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
    (Obviously, but it needs to be spelled out for you)

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
    create a executable named "prog")
    run the executable(Either "prog" or "./prog")
    
Now you're all set. Have a good day professor
