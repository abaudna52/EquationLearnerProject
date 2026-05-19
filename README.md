# EquationLearner

Десктопное приложение для изучения нелинейных уравнений. Вводишь уравнение — получаешь подробный анализ, интерактивный график, блок-схему алгоритма решения и готовый код на C++.

![C++](https://img.shields.io/badge/C++-17-blue?logo=cplusplus)
![Qt](https://img.shields.io/badge/Qt-6-green?logo=qt)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3-red)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS-lightgrey)

---

## Возможности

- **Анализ уравнения** — подробное объяснение типа уравнения, его свойств и корней
- **Интерактивный график** — отрисовка f(x) через OpenGL 3.3, масштаб колесом мыши, перетаскивание
- **Блок-схема алгоритма** — пошаговое визуальное объяснение метода решения
- **Генерация кода C++** — полная программа с комментариями и подсветкой синтаксиса
- **Выбор метода** — Ньютона, бисекции, секущих, итерации, аналитический или авто
- **История запросов** — сохраняется между сессиями, клик по записи восстанавливает уравнение

## Стек

| Компонент | Технология |
|-----------|-----------|
| Язык | C++ 17 |
| GUI | Qt 6 (Widgets, Network, OpenGL) |
| Графика | OpenGL 3.3 Core Profile |
| Сборка | CMake 3.16+ |

## Установка и сборка

### Требования

- [Qt 6](https://www.qt.io/download-qt-installer) с компонентами:
  - Qt 6.x.x → MinGW 64-bit
  - Qt 6.x.x → CMake
  - Qt 6.x.x → MinGW Toolchain

### Windows

Запусти `build_windows.bat` в папке проекта — скрипт сам найдёт Qt, соберёт проект и скопирует все нужные DLL.

```
build_windows.bat
```

Готовый `.exe` появится в папке `build_win\`.

### Вручную (CMake)

```bash
cmake -S . -B build -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/platform
cmake --build build --parallel
```

## Использование

1. Запусти приложение
2. Введи URL API, название модели и API-ключ в верхней строке
3. Введи уравнение в поле ввода, например:
   - `x^3 - 2*x + 1 = 0`
   - `sin(x) - x/2 = 0`
   - `exp(x) - 3*x = 0`
4. Выбери метод решения (или оставь «Авто»)
5. Нажми **Анализировать** и смотри четыре вкладки

### Поддерживаемый синтаксис уравнений

```
Операторы:  + - * / ^ (степень)
Константы:  pi, e
Функции:    sin, cos, tan, sqrt, log, ln, exp, abs
Примеры:    2x, 2(x+1)  — неявное умножение
```

## Структура проекта

```
EquationLearner/
├── core/
│   └── evaluator.h       — парсер математических выражений (чистый C++)
├── ui/
│   ├── graphwidget.h/cpp  — OpenGL-график
│   ├── highlighter.h/cpp  — подсветка синтаксиса C++
│   ├── explanation.h/cpp  — блок-схема алгоритма
│   ├── llmclient.h/cpp    — клиент внешнего API
│   └── mainwindow.h/cpp   — главное окно
├── qt/
│   ├── main.cpp           — точка входа
│   └── mainwindow.ui      — разметка интерфейса (Qt Designer)
├── CMakeLists.txt         — конфигурация сборки
└── build_windows.bat      — скрипт сборки для Windows
```

## Лицензия

MIT
