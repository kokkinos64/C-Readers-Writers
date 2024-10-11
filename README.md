# C-Readers-Writers
This project was a programming assignment on the OS course at NKUA and showcases the Readers-Writers process synchronization problem, using the POSIX API.

# Programs  
The entire project consists of the following programs:
1.	The **Allocator**, which is responsible for creating the semaphores and the shared memory segment, and also deleting them (the allocator can also act as a dislocator at the end of its operation).
3. The **Dislocator**, which dislocates the resources allocated by the allocator. I haven't used it  much, since the allocator has dislocating capabilities, but is handy to have.
2.	The **Monitor**, which prints the current state of the semaphores and the shared memory segment in real-time.
3.	The **MallocCheck** and **SharedStruct** libraries. The first file contains custom helper functions for memory allocation checks and shared segment management (I simplified the API, incorporating the necessary checks so I wouldn’t clutter my main programs with code). The latter contains the definition of a shared data structure, along with helper functions for accessing it.
4.	The **Reader** and **Writer** programs.
5.	The **createReaders** and **createWriters** programs, which act as a convenient way of spawning multiple Reader and Writer processes at random time to demonstrate the concurrency.
Now, let's take a look at each program, starting with the Allocator.
## Allocator
As the name suggests, the Allocator allocates memory and semaphores needed by the readers and writers. We use a total of three semaphores. The first is the well-known mutex for mutual exclusion, the second is wrt (short for writer), which blocks/unblocks the writers, and finally, queue, which ensures starvation-free concurrency. The initial values of these three semaphores are set to 1.
The Allocator creates the semaphores with sem_open using the O_CREAT flag, then constructs the shared segment, which is of SharedStruct type, and maps it into memory. The Allocator is also responsible for error handling.

## Dislocator
The Dislocator does the opposite of the Allocator. It deletes the semaphores and the shared memory segment created by the Allocator, releasing the resources.

## Monitor
The Monitor continuously prints the state of the semaphores and the shared memory segment while the system is running. This allows us to observe the current state of the system in real-time.

## Reader and Writer programs
Readers and Writers are responsible for accessing the shared memory concurrently. Readers use the semaphores to ensure mutual exclusion and avoid race conditions. Writers need exclusive access to modify the shared memory, and semaphores coordinate access between readers and writers to maintain consistency and avoid starvation.

## createReaders and createWriters
To make development easier, I also wrote two additional programs: createReaders and createWriters. These programs, through the use of fork() and exec() system calls, spawn 10 readers and writers at random time intervals. This setup allows for testing of the concurrent behavior in a controlled environment.

# How to Run
After downloading, navigate to the project's root folder and run the `make` command to compile the code. Make sure to have the `SampleData4Proj3` folder in the same directory as the programs. The `createReaders` and `createWriters` programs will look for files in that directory.

To run everything correctly, you'll need four terminal windows (I recommend using *Terminator* or *Tilix*). 
In one window, run the **allocator**, which will generate a shared segment ID. 
In the second window, run the **monitor**, passing it the ID printed by the Allocator. 
In the last two windows, run the **`createReaders`** and **`createWriters`**, which will also request the shared memory segment ID. 

Once they start, **observe the monitor window** as Readers and Writers go in and out of the shared memory segment.

You will notice that after the 20 readers and writers finish their job, they will "hang". This is expected, because there is no `wait_pid()` call in the createReaders and createWriters code. Therefore, once they are done, terminate them with the `CTRL + C` interrupt. 

# How to Terminate 
Once the processes are finished, press any key in the allocator side of the terminal window to print statistics that were collected during runtime. Finally, press y to indicate that you want to destroy the segment along with the semaphores, to dislocate system resources.

# Further documentation
For an in-depth look at the code, please refer to the documentation found in the `docs` folder. To view the offline documentation page, open the `index.html` file in your web browser.
