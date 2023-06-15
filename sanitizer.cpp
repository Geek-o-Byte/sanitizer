#include <pthread.h>
#include <dlfcn.h>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include "sanitizer.h"
using namespace std;

bool detectCycle(pthread_mutex_t *mutex, MutexGraph &mutexGraph, unordered_map<pthread_mutex_t *, int> &colors);

static int (*orig_pthread_mutex_lock)(pthread_mutex_t *__mutex) = NULL;
static int (*orig_pthread_mutex_unlock)(pthread_mutex_t *__mutex) = NULL;

MutexGraph mutexGraph;
DeadlockData deadlockData;

pthread_mutex_t san_mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {
int pthread_mutex_lock(pthread_mutex_t *__mutex) {
    if (orig_pthread_mutex_lock == NULL) {
        orig_pthread_mutex_lock = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_lock");
    }
    if (orig_pthread_mutex_unlock == NULL) {
        orig_pthread_mutex_unlock = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    }

    orig_pthread_mutex_lock(&san_mutex);
    deadlockData.lockMap[__mutex] = pthread_self();
    deadlockData.lockStack.push_back(__mutex);

    if (!deadlockData.lockStack.empty()) {
        pthread_mutex_t *prevMutex = deadlockData.lockStack.back();
        mutexGraph.graph[prevMutex].push_back(__mutex);
    }

    checkDeadlock(mutexGraph, deadlockData);

    orig_pthread_mutex_unlock(&san_mutex);

    return orig_pthread_mutex_lock(__mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *__mutex) {
    if (orig_pthread_mutex_lock == NULL) {
        orig_pthread_mutex_lock = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_lock");
    }
    if (orig_pthread_mutex_unlock == NULL) {
        orig_pthread_mutex_unlock = (int (*)(pthread_mutex_t *)) dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    }

    orig_pthread_mutex_lock(&san_mutex);
    deadlockData.lockMap.erase(__mutex);
    deadlockData.lockStack.pop_back();

    mutexGraph.graph.erase(__mutex);

    checkDeadlock(mutexGraph, deadlockData);

    orig_pthread_mutex_unlock(&san_mutex);

    return orig_pthread_mutex_unlock(__mutex);
}
}

void checkDeadlock(MutexGraph &mutexGraph, DeadlockData &deadlockData) {
    unordered_map<pthread_mutex_t *, int> colors;

    for (auto it = mutexGraph.graph.begin(); it != mutexGraph.graph.end(); ++it) {
        pthread_mutex_t *mutex = it->first;
        colors[mutex] = 0;  // Color::WHITE
    }

    for (auto it = mutexGraph.graph.begin(); it != mutexGraph.graph.end(); ++it) {
        pthread_mutex_t *mutex = it->first;
        if (colors[mutex] == 0) {  // Color::WHITE
            if (detectCycle(mutex, mutexGraph, colors)) {
                cout << "potential deadlock found" << endl;
                deadlockData.deadlockFound = true;
                break;
            }
        }
    }
}

bool detectCycle(pthread_mutex_t *mutex, MutexGraph &mutexGraph, unordered_map<pthread_mutex_t *, int> &colors) {
    colors[mutex] = 1;  // Color::GRAY

    for (pthread_mutex_t *nextMutex : mutexGraph.graph[mutex]) {
        if (colors[nextMutex] == 1) {  return true;  // Cycle detected
        } else if (colors[nextMutex] == 0) {  // Color::WHITE
            if (detectCycle(nextMutex, mutexGraph, colors)) {
                return true;  // Cycle detected
            }
        }
    }

    colors[mutex] = 2;  // Color::BLACK
    return false;  // No cycle detected
}