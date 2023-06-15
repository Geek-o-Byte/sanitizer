#pragma once

#include <pthread.h>
#include <unordered_map>
#include <vector>

struct MutexGraph {
    std::unordered_map<pthread_mutex_t *, std::vector<pthread_mutex_t *>> graph;
};

struct DeadlockData {
    std::unordered_map<pthread_mutex_t *, pthread_t> lockMap;
    std::vector<pthread_mutex_t *> lockStack;
    bool deadlockFound = false;
};

void checkDeadlock(MutexGraph &mutexGraph, DeadlockData &deadlockData);