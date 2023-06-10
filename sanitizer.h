#pragma once

#include <pthread.h>
#include <unordered_map>
#include <vector>

// Структура, представляющая граф мьютексов
struct MutexGraph {
  std::unordered_map<pthread_mutex_t *, std::vector<pthread_mutex_t *>> graph;
};

// Структура, содержащая данные для проверки deadlock
struct DeadlockData {
  std::unordered_map<pthread_mutex_t *, pthread_t> lockMap;
  std::vector<pthread_mutex_t *> lockStack;
  bool deadlockFound;
};

// Функция для проверки наличия deadlock
void checkDeadlock(MutexGraph &mutexGraph, DeadlockData &deadlockData);

// Функция для проверки deadlock с помощью DFS
void checkDeadlockHelper(pthread_mutex_t *mutex,
                         std::unordered_map<pthread_mutex_t *, int> &color,
                         MutexGraph &mutexGraph, DeadlockData &deadlockData);
