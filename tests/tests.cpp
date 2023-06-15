#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "sanitizer.h"
#include <pthread.h>

// Test Case 1: No Deadlock
TEST_CASE("No Deadlock") {
  pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

  auto thread1 = [](void *arg) -> void * {
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex2);
    // Critical section
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
  };

  auto thread2 = [](void *arg) -> void * {
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex2);
    // Critical section
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
  };

  pthread_t t1, t2;
  pthread_create(&t1, NULL, thread1, NULL);
  pthread_create(&t2, NULL, thread2, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  // No deadlock should be detected
  CHECK(!deadlockData.deadlockFound);
}

// Test Case 2: Potential Deadlock
TEST_CASE("Potential Deadlock") {
  pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

  auto thread1 = [](void *arg) -> void * {
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex2);
    // Critical section
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
  };

  auto thread2 = [](void *arg) -> void * {
    pthread_mutex_lock(&mutex2);
    pthread_mutex_lock(&mutex1);
    // Critical section
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
  };

  pthread_t t1, t2;
  pthread_create(&t1, NULL, thread1, NULL);
  pthread_create(&t2, NULL, thread2, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  // Potential deadlock should be detected
  CHECK(deadlockData.deadlockFound);
}

// Test Case 3: No Deadlock (Nested Lock)
TEST_CASE("No Deadlock (Nested Lock)") {
  pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

  auto thread1 = [](void *arg) -> void * {
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex1);
    // Critical section
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex1);
    return NULL;
  };

  pthread_t t1;
  pthread_create(&t1, NULL, thread1, NULL);
  pthread_join(t1, NULL);

  // No deadlock should be detected
  CHECK(!deadlockData.deadlockFound);
}

// Test Case 4: Potential Deadlock (Self-Lock)
TEST_CASE("Potential Deadlock (Self-Lock)") {
  pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

  auto thread1 = [](void *arg) -> void * {
    pthread_mutex_lock(&mutex1);
    pthread_mutex_lock(&mutex1);
    // Critical section
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex1);
    return NULL;
  };

  pthread_t t1;
  pthread_create(&t1, NULL, thread1, NULL);
  pthread_join(t1, NULL);

  // Potential deadlock should be detected
  CHECK(deadlockData.deadlockFound);
}
