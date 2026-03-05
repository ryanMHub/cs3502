# Project 01 - Threads and Deadlocks (Phases 1 - 4)
This project is used to demonstrate deadlocks and the solutions to solve deadlock problems between threads and global resources.
There are 5 C files in this project. I know 4 were expected, but I did complete 2 of the phase 4 strategies. This project under went
4 phases. In the first phase we established the process of using threads and the race conditions that can occur. In the second stage
we embarked on the use of a locking mechanism to prevent the race condition from occurring. The third stage was used to learn about
deadlocks and what makes them occur. On the final stage we developed two strategies to solve the deadlock problems. A lock ordering
implementation which uses a sortable shared global resource and locks the accounts in their ascending order. The second strategy
uses a timedlock which if it is unable to lock a resource because it is waiting will backout of the process and continuously retry
until it is finally available. 

## Requirements
- GCC (or compatible C compiler)
- 'make'
- POSIX threads support (pthreads)

These should be base products in a linux distro

## Files
- `phase1.c` -> builds `phase1`
- `phase2.c` -> builds `phase2`
- `phase3.c` -> builds `phase3`
- `phase4.c` -> builds `phase4`
- `phase4_timeout.c` -> builds `phase4_timeout`
- `Makefile` -> build automation

## Buidl Instructions

### Option A: Build with Makefile (recommended)
From the project directory

```bash
make clean
