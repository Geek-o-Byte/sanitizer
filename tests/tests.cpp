#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <sanitizer.h>
#include <pthread.h>

// Декларация функций-заглушек для создания потоков
void* thread1_func(void* arg);
void* thread2_func(void* arg);

pthread_mutex_t mutex1, mutex2; // Объявляем переменные mutex1 и mutex2 здесь

TEST_CASE("just_example") {
CHECK(4 == 4);
}

TEST_CASE("deadlock_detection") {
pthread_mutex_init(&mutex1, NULL);
pthread_mutex_init(&mutex2, NULL);

pthread_t thread1, thread2;

pthread_create(&thread1, NULL, thread1_func, &mutex1);
pthread_create(&thread2, NULL, thread2_func, &mutex2);

pthread_join(thread1, NULL);
pthread_join(thread2, NULL);

CHECK(deadlock_found);
}

void* thread1_func(void* arg) {
    pthread_mutex_t* mutex1 = static_cast<pthread_mutex_t*>(arg);

    pthread_mutex_lock(mutex1);
    pthread_mutex_lock(&mutex2);
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(mutex1);

    return nullptr;
}

void* thread2_func(void* arg) {
    pthread_mutex_t* mutex2 = static_cast<pthread_mutex_t*>(arg);

    pthread_mutex_lock(mutex2);
    pthread_mutex_lock(&mutex1);
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(mutex2);

    return nullptr;
}

// Добавьте дополнительные тесты здесь
