# htm-unordered-queue
A parallel-friendly unordered queue based on Hardware Transactional Memory

# Intro
Parallel programming is innately hard. If everything is running under an embarassingly parallel setting, the world will be easy. However, conflicts and concurrency make everything complicated.
This library targets at the traditional work-list problem: giving a set of jobs in which each jobs is independent of others in terms of order, provide a program to process all the jobs. During this procedure, processing one job may produce new jobs.
Example situations include the [Delaunay Mesh Refinement problem] (https://en.wikipedia.org/wiki/Delaunay_triangulation), where a work-list is used for holding all the bad triangles that needed to be changed into good triangles.

# Usage
This library can be used to replace a C++ STL queue or C++ STL unordered-set, givin the different situation.<br><br>
Please include the "unorderedset-array-of-queue-dynamic.hpp" file in your source code and replace your STL initialization with this library.

# Interface
## 1. Constructor
```cpp
UnorderedSetArrayOfQueueDynamic<Key>(size_t totalNumberOfThreads);
```

## 2. Insert
Insert into the queue can be done by calling: 
```cpp
void insert(Key val, size_t threadId);
```

## 3. Remove
Remove element from the data structure (order DOESN'T matter)
```cpp
Key remove(size_t threadId);
```

## 4. Empty
Check if the work-list is empty
```cpp
bool empty();
```
