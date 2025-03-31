# Operating Systems II (Project)

This repo contains two projects covering the course material of "Systemy Operacyjne 2" @ Wroclaw University of Science and Technology (3rd year of Computer Engineering)

## Dining Philosophers Project

### 1. Problem Description

The "Dining Philosophers" problem is a classic synchronization problem used in computer science to teach about concurrency and resource management. It involves a group of philosophers who are seated at a table, each thinking and eating, but sharing a limited number of forks. Each philosopher needs two forks to eat, one from their left and one from their right. The challenge arises in ensuring that all philosophers get a chance to eat without running into two key issues:

- **Deadlock**: This happens when every philosopher picks up one fork and then waits for the other, causing the system to halt because no one can eat.
- **Starvation**: This occurs if some philosophers never get a chance to eat because others are constantly eating.

The goal is to implement a solution that allows all philosophers to eat while avoiding these issues.

### 2. Running Instructions

To get started with the project, follow these steps:

#### 1. Prerequisites:

- Ensure you have `gcc` (GNU Compiler Collection) and `make` installed on your system.

#### 2. Building the Project:

- Clone the repository or copy the source code to your local machine.
- Open a terminal in the project directory and run the following command to compile the project:

  ```bash
  make
  ```

#### 3. Running the Simulation:

- After the project is built, you can run the simulation by specifying the number of philosophers as an argument:

  ```bash
  ./dining_philosophers <number_of_philosophers>
  ```

  Example:

  ```bash
  ./dining_philosophers 5
  ```

- This will start the simulation with 5 philosophers. The program will run indefinitely, simulating philosophers thinking, becoming hungry, and eating.

#### 4. Running the Tests:

- The project includes a test script to check the functionality of the simulation. To run the tests, execute the following command:

  ```bash
  ./test.sh
  ```

  The script will run a series of tests with different numbers of philosophers and durations to ensure that the program behaves as expected.

### 3. Walkthrough of the Implementation and Tests

#### Main Components of the Implementation

##### 1. **Philosopher Thread (Concurrency)**:

- Each philosopher is represented by a separate thread. The thread continuously goes through three phases:

  - **Thinking**: The philosopher is thinking, and no fork is involved.
  - **Hungry**: The philosopher is hungry and tries to pick up the forks.
  - **Eating**: The philosopher eats after acquiring both forks.

- Philosophers use two mutexes (locks) to manage access to the forks. Each philosopher has a left and right fork, and the order in which they acquire the forks is adjusted to avoid deadlock:
  - Even-numbered philosophers acquire the left fork first, then the right fork.
  - Odd-numbered philosophers acquire the right fork first, then the left fork.

##### 2. **Critical Sections and Their Solution**:

- **Deadlock (Critical Section)**:

  - The main issue to address in this problem is deadlock, where all philosophers are holding one fork and waiting for the other, thus preventing any philosopher from eating.
  - **Solution**: Philosophers are split into two groups:
    - Even philosophers pick up the left fork first, and then the right fork.
    - Odd philosophers pick up the right fork first, then the left fork.
  - This ensures that not all philosophers will wait for the same fork, thus avoiding deadlock.

- **Starvation (Critical Section)**:

  - Starvation occurs when one or more philosophers are unable to eat because others are always eating. While the implementation ensures that each philosopher gets a chance to eat, it doesn't guarantee perfect fairness (though it minimizes starvation by allowing philosophers to alternate between eating and thinking).

- **Mutex Locks**:
  - `pthread_mutex_lock` and `pthread_mutex_unlock` are used to synchronize access to the forks, ensuring that only one philosopher can use a fork at a time.

##### 3. **Test Script (`test.sh`)**:

The script `test.sh` runs a series of tests to verify the behavior of the program:

- **Test Case Format**:
  Each test case is defined by three parameters:

  - Test ID
  - Number of philosophers
  - Duration (in seconds)

- **Expected Behavior**:

  - For valid philosopher counts (natural numbers greater than 1), the program checks that every philosopher eats at least once. This is confirmed by checking the logs for the "eating" message for each philosopher.
  - For invalid philosopher counts (less than 2), the program checks that the error message "At least two philosophers are required" is printed.

- **Test Execution**:
  The script runs the program with the specified number of philosophers and logs the output. It then checks for the expected messages in the log files, reporting whether each test passed or failed.

#### Summary of Threads and Critical Sections

- **Threads**:
  - The main program thread
  - 1 threads per philosopher (i.e. 5 philosophers result in 6 threads total)
- **Critical Sections**:
  - The critical sections involve acquiring the forks (mutex locks) to avoid race conditions.
  - The solution to deadlock involves alternating the order of fork acquisition between even and odd philosophers.
