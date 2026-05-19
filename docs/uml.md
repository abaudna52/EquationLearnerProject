# UML — ExpressionEvaluator

Класс из `src/main.cpp`. Чистый C++17, без зависимостей от Qt.

---

## Диаграмма классов

```mermaid
classDiagram
    class ExpressionEvaluator {
        -string m_expr
        -int m_pos
        -double m_x
        -bool m_valid
        -string m_error

        +setExpression(string expr) void
        +evaluate(double x) double
        +isValid() bool
        +getError() string

        -peek() char
        -take() char
        -parseExpr() double
        -parseTerm() double
        -parsePower() double
        -parseFactor() double
        -applyFn(string name, double arg) double
    }
```

---

## Схема рекурсивного парсера

Показывает, как `evaluate()` разбирает выражение по приоритету операций.

```mermaid
flowchart TD
    A([evaluate&#40;x&#41;]) --> B[parseExpr]

    B -->|"слагаемые через + / -"| C[parseTerm]
    B -->|"пока есть + или -"| B

    C -->|"множители через * / /"| D[parsePower]
    C -->|"неявное умножение 2x"| D
    C -->|"пока есть * или /"| C

    D -->|"основание"| E[parseFactor]
    D -->|"если есть ^"| D2[parsePower рекурсивно]
    D2 -.->|правый операнд| E

    E --> F{Что за символ?}

    F -->|"цифра / точка"| G["число (stod)"]
    F -->|"минус / плюс"| H["унарный знак → parseFactor"]
    F -->|"скобка ("| I["parseExpr → ожидаем )"]
    F -->|"буква"| J{Что за имя?}

    J -->|x| K[вернуть m_x]
    J -->|pi| L[вернуть M_PI]
    J -->|e| M[вернуть M_E]
    J -->|"функция + ("| N[applyFn]

    N --> O["sin / cos / tan\nsqrt / log / ln\nexp / abs"]
```

---

## Жизненный цикл объекта

```mermaid
sequenceDiagram
    participant App
    participant EE as ExpressionEvaluator
    participant Parser as Внутренний парсер

    App->>EE: setExpression("x^2 - 2*x + 1")
    EE->>Parser: parseExpr() — проверка синтаксиса
    Parser-->>EE: успех → m_valid = true

    App->>EE: isValid()
    EE-->>App: true

    loop для каждой точки графика
        App->>EE: evaluate(x)
        EE->>Parser: parseExpr() с m_x = x
        Parser-->>EE: результат double
        EE-->>App: f(x)
    end

    App->>EE: setExpression("sin(x ++ 1")
    EE->>Parser: parseExpr() — проверка синтаксиса
    Parser-->>EE: исключение — m_valid = false

    App->>EE: isValid()
    EE-->>App: false
    App->>EE: getError()
    EE-->>App: "Неожиданный символ: '+'"
```
