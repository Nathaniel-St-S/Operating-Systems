# 3.3 Process Description

You can think of the OS as an entity that manages the use of system resources by processes.

### Operating System Control Structures
- The OS manages tables of information about each entity it is managing.
1. Memory Tables
  - Keeps track and main and virtual memory.
  - Includes information about allocation of main and virtual memory to processes, any protection attributes (permission, etc.), and any other information needed to manage virtual  memory.
2. I/O Tables
  - Manages I/O devices and channels
  - Needs to know the status and location of the I/O operation.
3. File Tables
   - Provide information about the existence of files, their location in secondary memory, and their current status.
   - Can be delegated to a file management system instead.
4. Process Tables
- All of these tables must be linked somehow

### Process Control Structures
The OS must know two things about a process to manage it:
1. Location
2. Attributes (e.g., process ID and state)

#### Process Location
- A process must contain
  1. The programs and data it is using
  2. A procedure stack
  3. Attributes (process control block)
- All three of these define the **process image**
- The process image is usually stored on the disk and partly in main memory
  - The OS must know the location
  - To execute it, the entire process must be loaded into main/virtual memory
- Process tables store the location of each page of each process image

#### Process Attributes
1. Identifiers: Numeric IDs
  - ID of the process
  - Identifier of parent process
  - User ID
2. State information
  - User visible register
  - Control and status registers
    - Program control
    - Condition codes: result of most recent operation
    - Status information: e.g. interrupts enabled?
  - Program status word
3. Control information
  - Scheduling and state information
    - Process state
    - Priority
    - Scheduling
    - Event
  - Data structuring: queue, ring, etc
  - Interprocess communication
  - Process privileges
  - Memory management
  - Resource ownership and utilization

#### The Role of the Process Control Block
- Defines the state of the OS
  - Problems
    1. A bug in a single routine could damage control blocks
    2. A design change would affect most of the other modules in an OS
