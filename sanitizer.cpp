#include "sanitizer.h"
#include <dlfcn.h>
#include <pthread.h>
#include <unordered_map>
#include <vector>

std::unordered_map<pthread_mutex_t*, pthread_t> mutex_lock_map;
std::vector<pthread_mutex_t*> mutex_lock_stack;
bool deadlock_found = false;

static int (*orig_pthread_mutex_lock)(pthread_mutex_t *mutex) = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_lock");
static int (*orig_pthread_mutex_unlock)(pthread_mutex_t *mutex) = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_unlock");

std::unordered_map<pthread_mutex_t*, std::vector<pthread_mutex_t*>> mutex_graph;

extern "C" {
int pthread_mutex_lock(pthread_mutex_t* mutex) {
    mutex_lock_map[mutex] = pthread_self();
    mutex_lock_stack.push_back(mutex);

    check_deadlock();

    return orig_pthread_mutex_lock(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t* mutex) {
    mutex_lock_map.erase(mutex);
    mutex_lock_stack.pop_back();

    check_deadlock();

    return orig_pthread_mutex_unlock(mutex);
}
}

void check_deadlock_helper(pthread_mutex_t* mutex, std::unordered_map<pthread_mutex_t*, int>& color) {
    color[mutex] = 1;

    for (pthread_mutex_t* next_mutex : mutex_graph[mutex]) {
        if (color[next_mutex] == 0) {
            check_deadlock_helper(next_mutex, color);

            if (deadlock_found) {
                return;
            }
        }
        else if (color[next_mutex] == 1 && mutex_lock_map[next_mutex] == pthread_self()) {
            deadlock_found = true;
            return;
        }
    }

    color[mutex] = 2;
}

void check_deadlock() {
    if (deadlock_found) {
        return;
    }

    std::unordered_map<pthread_mutex_t*, int> color;

    for (auto it = mutex_lock_map.begin(); it != mutex_lock_map.end(); ++it) {
        pthread_mutex_t* mutex = it->first;

        if (color[mutex] == 0) {
            check_deadlock_helper(mutex, color);
        }
    }
}
