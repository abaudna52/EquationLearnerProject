#pragma once

#include <string>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <algorithm>

using namespace std;

// =============================================================================
// ExpressionEvaluator — парсер математических выражений (рекурсивный спуск)
// Чистый C++17, без зависимостей от Qt.
//
// Поддерживает: + - * / ^ скобки, унарный минус
// Константы:    pi, e
// Функции:      sin cos tan sqrt log ln exp abs
// Неявное умножение: 2x, 2(x+1)
// =============================================================================

class ExpressionEvaluator {
public:

    void setExpression(string expr) {
        for (int i = 0; i < (int)expr.size() - 1; i++) {
            if (expr[i] == '*' && expr[i + 1] == '*') {
                expr[i] = '^'; expr.erase(i + 1, 1);
            }
        }
        m_expr = expr; m_valid = false; m_error = ""; m_pos = 0; m_x = 1.0;
        try   { parseExpr(); m_valid = true; }
        catch (exception& e) { m_error = e.what(); }
    }

    double evaluate(double x) {
        if (!m_valid) throw runtime_error("Выражение некорректно");
        m_x = x; m_pos = 0;
        return parseExpr();
    }

    bool   isValid()  { return m_valid; }
    string getError() { return m_error; }

private:
    string m_expr;
    int    m_pos   = 0;
    double m_x     = 0.0;
    bool   m_valid = false;
    string m_error;

    char peek() {
        while (m_pos < (int)m_expr.size() && m_expr[m_pos] == ' ') m_pos++;
        return m_pos < (int)m_expr.size() ? m_expr[m_pos] : '\0';
    }
    char take() { char c = peek(); m_pos++; return c; }

    double parseExpr() {
        double r = parseTerm();
        while (peek() == '+' || peek() == '-') {
            char op = take(); double rhs = parseTerm();
            r = (op == '+') ? r + rhs : r - rhs;
        }
        return r;
    }

    double parseTerm() {
        double r = parsePower();
        while (true) {
            char c = peek();
            if      (c == '*') { take(); r *= parsePower(); }
            else if (c == '/') {
                take(); double d = parsePower();
                if (fabs(d) < 1e-300) throw runtime_error("Деление на ноль");
                r /= d;
            }
            else if (isalpha(c) || c == '(') { r *= parsePower(); }
            else break;
        }
        return r;
    }

    double parsePower() {
        double base = parseFactor();
        if (peek() == '^') { take(); return pow(base, parsePower()); }
        return base;
    }

    double parseFactor() {
        char c = peek();
        if (c == '-') { take(); return -parseFactor(); }
        if (c == '+') { take(); return  parseFactor(); }
        if (c == '(') {
            take(); double v = parseExpr();
            if (peek() != ')') throw runtime_error("Ожидалась ')'");
            take(); return v;
        }
        if (isdigit(c) || c == '.') {
            int s = m_pos;
            while (m_pos < (int)m_expr.size() &&
                   (isdigit(m_expr[m_pos]) || m_expr[m_pos] == '.')) m_pos++;
            return stod(m_expr.substr(s, m_pos - s));
        }
        if (isalpha(c)) {
            int s = m_pos;
            while (m_pos < (int)m_expr.size() && isalpha(m_expr[m_pos])) m_pos++;
            string name = m_expr.substr(s, m_pos - s);
            if (name == "x")  return m_x;
            if (name == "pi") return M_PI;
            if (name == "e")  return M_E;
            if (peek() != '(') throw runtime_error("Неизвестное имя: " + name);
            take(); double arg = parseExpr();
            if (peek() != ')') throw runtime_error("Ожидалась ')' после аргумента");
            take(); return applyFn(name, arg);
        }
        throw runtime_error(string("Неожиданный символ: '") + c + "'");
    }

    double applyFn(const string& name, double arg) {
        if (name == "sin")  return sin(arg);
        if (name == "cos")  return cos(arg);
        if (name == "tan")  return tan(arg);
        if (name == "sqrt") return sqrt(arg);
        if (name == "log")  return log10(arg);
        if (name == "ln")   return log(arg);
        if (name == "exp")  return exp(arg);
        if (name == "abs")  return fabs(arg);
        throw runtime_error("Неизвестная функция: " + name);
    }
};
