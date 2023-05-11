# translator
studying translator implementation

# How to install
```bash
git clone git@github.com:zeitgeists/translator.git && \
cd translator && \
git submodule update --init --recursive && \
./vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake && \
cmake --build build
```

# Run tests
To run tests firstly go into tests directory and then run `ctest`

# Завдання
Вычисление и построение графиков функций.

Пользователь вводит аналитическое выражение описывающее функцию одной переменной.
Выражение компилируется при помощи Clang в исполняемый модуль
который запускается из основной программы, производит необходимые вычисления
и сохраняет результат на диске.
В выражениях можно использовать тернарный оператор, например:
`x = ? a!=0 { y/a : 0 } + c`
