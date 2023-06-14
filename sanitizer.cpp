#include "sanitizer.h"
#include <dlfcn.h>
#include <pthread.h>

MutexGraph mutexGraph;
DeadlockData deadlockData;

static int (*orig_pthread_mutex_lock)(pthread_mutex_t *mutex) =
(int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_lock");
static int (*orig_pthread_mutex_unlock)(pthread_mutex_t *mutex) =
(int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_unlock");

extern "C" {
int pthread_mutex_lock(pthread_mutex_t *mutex) {
    deadlockData.lockMap[mutex] = pthread_self();
    deadlockData.lockStack.push_back(mutex);

    // Добавление мьютекса в граф и создание ребер от предыдущих мьютексов
    if (!deadlockData.lockStack.empty()) {
        pthread_mutex_t *prevMutex = deadlockData.lockStack.back();
        mutexGraph.graph[prevMutex].push_back(mutex);
    }

    checkDeadlock(mutexGraph, deadlockData);

    return orig_pthread_mutex_lock(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    deadlockData.lockMap.erase(mutex);
    deadlockData.lockStack.pop_back();

    // Удаление мьютекса из графа
    mutexGraph.graph.erase(mutex);

    checkDeadlock(mutexGraph, deadlockData);

    return orig_pthread_mutex_unlock(mutex);
}
}

void checkDeadlockHelper(pthread_mutex_t *mutex,
                         std::unordered_map<pthread_mutex_t *, int> &color,
                         MutexGraph &mutexGraph, DeadlockData &deadlockData) {
    color[mutex] = 1;

    for (pthread_mutex_t *nextMutex : mutexGraph.graph[mutex]) {
        if (color[nextMutex] == 0) {
            checkDeadlockHelper(nextMutex, color, mutexGraph, deadlockData);

            if (deadlockData.deadlockFound) {
                return;
            }
        } else if (color[nextMutex] == 1 &&
                   deadlockData.lockMap[nextMutex] == pthread_self()) {
            deadlockData.deadlockFound = true;
            return;
        }
    }

    color[mutex] = 2;
}

void checkDeadlock(MutexGraph &mutexGraph, DeadlockData &deadlockData) {
    if (deadlockData.deadlockFound) {
        return;
    }

    std::unordered_map<pthread_mutex_t *, int> color;

    for (auto it = deadlockData.lockMap.begin(); it != deadlockData.lockMap.end();
         ++it) {
        pthread_mutex_t *mutex = it->first;

        if (color[mutex] == 0) {
            checkDeadlockHelper(mutex, color, mutexGraph, deadlockData);
        }
    }
}
