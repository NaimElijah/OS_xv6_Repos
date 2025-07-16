// Long-term locks for processes
struct petersonlock {
  int locked;               // Is the lock held? (0 = not held, 1 = held)

  //  Variables for the Peterson Lock Algorithm
  int turn;                 // The turn variable (0 or 1)
  int interested[2];        // Array to indicate interest of each process:   0 = not interested(false)   ,  1 = interested(true)

  // For Debugging:
  int twospids[2];          // Array to store the process IDs of the two processes   //    <<<------------  not really needed, but can be used for debugging
  int holdingPid;           // Process holding lock.   //    <<<------------  not really needed, but can be used for debugging
};
