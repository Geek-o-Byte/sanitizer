#pragma once

#include <pthread.h>
#include <unordered_map>
#include <vector>

extern std::unordered_map<pthread_mutex_t*, pthread_t> mutex_lock_map;
extern std::vector<pthread_mutex_t*> mutex_lock_stack;
extern bool deadlock_found;

void check_deadlock_helper(pthread_mutex_t* mutex, std::unordered_map<pthread_mutex_t*, int>& color);
void check_deadlock();