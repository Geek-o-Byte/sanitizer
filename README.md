# Sanitizer. Алгоритм поиска deadlock

Вам нужно реализовать Google TSan алгоритм поиска deadlock в многопоточном коде с использование механизма LD\_PRELOAD.

**Алгоритм:** Вершинами графа являются мьютексы. Строится граф в котором проводится направленное ребро в порядке захвата мьютексов. Если в графе существует цикл, то значит в коде возможен потенциальный deadlock.

Необходимо выполнить четыре пункта:
* Разработать разделяемую библиотеку sanitizer.so
* Разработать unit тесты в файле tests/tests.cpp
* Разработать как минимум три теста на которых показать что разработанный алгоритм работает. Тесты должны быть в отдельной директории со своим Makefile и скриптами для запуска (используют sanitizer.so)
* Если в коде существует потенциальный deadlock, то должна быть выведена полезная информация, которая позволит понять где этот deadlock произошел# Sanitizer. Алгоритм поиска deadlock

Вам нужно реализовать Google TSan алгоритм поиска deadlock в многопоточном коде с использование механизма LD\_PRELOAD.

**Алгоритм:** Вершинами графа являются мьютексы. Строится граф в котором проводится направленное ребро в порядке захвата мьютексов. Если в графе существует цикл, то значит в коде возможен потенциальный deadlock.

Необходимо выполнить четыре пункта:
* Разработать разделяемую библиотеку sanitizer.so
* Разработать unit тесты в файле tests/tests.cpp
* Разработать как минимум три теста на которых показать что разработанный алгоритм работает. Тесты должны быть в отдельной директории со своим Makefile и скриптами для запуска (используют sanitizer.so)
* Если в коде существует потенциальный deadlock, то должна быть выведена полезная информация, которая позволит понять где этот deadlock произошел
