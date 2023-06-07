#include "sanitizer.h"
#include <dlfcn.h>
#include <pthread.h>

//std::unordered_map<pthread_mutex_t*, pthread_t> mutex_lock_map;
//std::vector<pthread_mutex_t*> mutex_lock_stack;
//bool deadlock_found = false;

static int (*orig_pthread_mutex_lock)(pthread_mutex_t *mutex) = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_lock");
static int (*orig_pthread_mutex_unlock)(pthread_mutex_t *mutex) = (int (*)(pthread_mutex_t *))dlsym(RTLD_NEXT, "pthread_mutex_unlock");

extern "C" {
    int pthread_mutex_lock(pthread_mutex_t* mutex) {
        // Сохранение информации о захвате мьютекса
        mutex_lock_map[mutex] = pthread_self();
        mutex_lock_stack.push_back(mutex);

        // Вызов реальной функции pthread_mutex_lock
        int result = orig_pthread_mutex_lock(mutex);

        // Проверка наличия deadlock после захвата мьютекса
        check_deadlock();

        // Возврат результата выполнения реальной функции pthread_mutex_lock
        return result;
    }

    // Обертка для функции pthread_mutex_unlock
    int pthread_mutex_unlock(pthread_mutex_t* mutex) {
        // Удаление информации о захвате мьютекса
        mutex_lock_map.erase(mutex);
        mutex_lock_stack.pop_back();

        // Вызов реальной функции pthread_mutex_unlock
        int result = orig_pthread_mutex_unlock(mutex);

        // Проверка наличия deadlock после освобождения мьютекса
        check_deadlock();

        // Возврат результата выполнения реальной функции pthread_mutex_unlock
        return result;
    }
}

// Функция для проверки deadlock с помощью DFS
void check_deadlock_helper(pthread_mutex_t* mutex, std::unordered_map<pthread_mutex_t*, int>& color) {
    color[mutex] = 1; // Красим текущий мьютекс в серый

    for (auto it = mutex_lock_map.begin(); it != mutex_lock_map.end(); ++it) {
        pthread_mutex_t* next_mutex = it->first;
        pthread_t thread_id = it->second;

        if (next_mutex == mutex) {
            continue; // Пропустить текущий мьютекс
        }

        if (color[next_mutex] == 0) {
            check_deadlock_helper(next_mutex, color); // Рекурсивный вызов для следующего мьютекса

            if (deadlock_found) {
                return; // Если deadlock уже найден, выходим из функции
            }
        }
        else if (color[next_mutex] == 1 && thread_id == pthread_self()) {
            deadlock_found = true; // Найден потенциальный deadlock
            return;
        }
    }

    color[mutex] = 2; // Красим текущий мьютекс в черный
}

// Функция для проверки наличия deadlock
void check_deadlock() {
    if (deadlock_found) {
        return; // Если deadlock уже найден, прекращаем проверку
    }

    std::unordered_map<pthread_mutex_t*, int> color;

    for (auto it = mutex_lock_map.begin(); it != mutex_lock_map.end(); ++it) {
        pthread_mutex_t* mutex = it->first;

        if (color[mutex] == 0) {
            check_deadlock_helper(mutex, color); // Запуск DFS для каждого мьютекса
        }
    }
}