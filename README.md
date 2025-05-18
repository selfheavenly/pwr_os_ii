# Operating Systems II (Project)

This repo contains two projects covering the course material of "Systemy Operacyjne 2" @ Wroclaw University of Science and Technology (3rd year of Computer Engineering)

## 01 - Dining Philosophers Project

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

## 02 - Sensor Gateway System Project

### Overview

This project implements a multithreaded **Sensor Gateway System** designed to simulate and manage data from multiple sensor nodes in a room-based environment. It involves:

- Simulated temperature sensor nodes sending data via TCP.
- A central gateway receiving, processing, averaging, logging, and storing data.
- A `logger` subprocess writing all system events to `gateway.log`.

---

### Key Components Overview

#### 1. `sensor_node`

Simulates individual sensors that:

- Generate temperature data.
- Connect to the gateway over TCP.
- Send periodic measurements (ID, temperature, timestamp).

#### 2. `sensor_gateway`

Central server program that:

- Listens for incoming sensor connections.
- Buffers sensor data in memory.
- Launches:

  - A connection manager thread (`connmgr`) – accepts data from clients.
  - A data manager thread (`datamgr`) – processes and validates data.
  - A storage manager thread (`sensor_db`) – logs to `data.csv`.

- Starts a separate logger process to write system events.

#### 3. `file_creator`

Generates:

- A `room_sensor.map` mapping room IDs to sensor IDs.
- A simulated binary sensor dataset (`sensor_data`) for testing.

---

### File Structure

```
.
├── Makefile
├── config.h
├── connmgr.c         # Connection Manager (Manages Active Connections)
├── connmgr.h
├── data.csv
├── datamgr.c         # Data Manager (Interprets the incoming sensor data)
├── datamgr.h
├── file_creator.c
├── gateway.log
├── lib               # Helper utilities: TCP Connection & Buffer Data Structure
│   ├── dplist.c
│   ├── dplist.h
│   ├── tcpsock.c
│   └── tcpsock.h
├── main.c            # Main Program
├── room_sensor.map
├── sbuffer.c         # Data Manager (Stores Sensor Measurements to the file)
├── sbuffer.h
├── sensor_db.c       # Storage Manager (Stores Sensor Measurements to the csv file)
├── sensor_db.h
├── sensor_node.c     # Virtual Room Sensor
├── test3.sh
└── test5.sh

2 directories, 22 files
```

---

### How It Works

#### Workflow

1. **Sensor nodes** start and connect to the server.
2. The **Connection Manager** accepts TCP connections and writes data to a **shared buffer**.
3. The **Data Manager** reads unprocessed data, computes a **running average**, and checks for **temperature thresholds**.
4. The **Storage Manager** writes processed data to `data.csv`.
5. The **Logger** reads messages from a pipe and logs to `gateway.log`.

---

### Setup Instructions

#### 1. Compile the Project

```bash
make all
```

#### 2. Run the Test Script

Two automated tests available:

```bash
bash test5.sh
```

```bash
bash test5.sh
```

Each:

- Starts the gateway and logger.
- Starts either 3 or 5 sensor nodes.
- Waits, then shuts down all processes.

---

### Manual Usage

#### Start Gateway

```bash
./sensor_gateway <port> <max_clients>
# Example:
./sensor_gateway 5678 5
```

#### Start Sensor Node

```bash
./sensor_node <sensor_id> <sleep_time> <server_ip> <port>
# Example:
./sensor_node 15 2 127.0.0.1 5678
```

---

### Output Files

- `gateway.log`: Event log (e.g., temp warnings, sensor activity).
- `data.csv`: Stored sensor data (ID, temperature, timestamp).
- `room_sensor.map`: Generated sensor-to-room mapping.

---

### Multithreading, Critical Sections & Communication

#### Synchronization & Critical Sections

This system uses **POSIX threads (`pthreads`)** for concurrency and manages **shared resources** using **mutexes and condition variables**:

- **Shared Buffer (`sbuffer`)**:

  - A central, thread-safe buffer shared among:

    - `connmgr` (writes new sensor data),
    - `datamgr` (reads and processes data),
    - `sensor_db` (reads and stores processed data).

  - **Critical sections** in `sbuffer.c` are protected by a `pthread_mutex_t` to prevent race conditions.
  - **Condition variables (`pthread_cond_t`)** are used to block consumer threads until data is available.

#### Threaded Architecture

The system launches **three dedicated threads** in `main.c`:

1. **Connection Manager Thread (`connmgr`)**
2. **Data Manager Thread (`datamgr`)**
3. **Storage Manager Thread (`sensor_db`)**

These run concurrently and coordinate via the `sbuffer`.

#### Logger Process via `fork()` and `pipe()`

- The **Logger** is implemented as a **separate process** using `fork()`.
- A **POSIX pipe** (`PIPE_READ`, `PIPE_WRITE`) enables communication between the gateway (parent) and the logger (child).
- All components (`connmgr`, `datamgr`, `sensor_db`) write logs to the pipe via a thread-safe `write_to_pipe()` function.
- The logger process continuously reads from the pipe and writes timestamped entries to `gateway.log`.

#### Pipe Safety

- The pipe is accessed in a **thread-safe** manner:

  ```c
  pthread_mutex_lock(&pipe_mutex);
  write(PIPE_WRITE, message, len);
  pthread_mutex_unlock(&pipe_mutex);
  ```

This ensures that log messages are not interleaved when multiple threads write concurrently.

---

### Summary of Thread & IPC Usage

| Component   | Type    | Synchronization         | Description                                   |
| ----------- | ------- | ----------------------- | --------------------------------------------- |
| `sbuffer`   | Shared  | `pthread_mutex`, `cond` | Shared queue for sensor data                  |
| `connmgr`   | Thread  | Inserts into buffer     | Reads TCP sensor data and inserts into buffer |
| `datamgr`   | Thread  | Reads from buffer       | Calculates average, logs alerts               |
| `sensor_db` | Thread  | Reads processed entries | Writes final data to `data.csv`               |
| `logger`    | Process | Pipe (`write`, `read`)  | Logs system messages to `gateway.log`         |

---

### Notes

- Temperature limits:

  - Too Cold: < 10.0°C
  - Too Hot: > 16.5°C

- The running average is based on the last 5 values.
- Supports up to 8 simulated sensor nodes by default.
- The gateway runs multiple threads + a subprocess using `fork()`.
