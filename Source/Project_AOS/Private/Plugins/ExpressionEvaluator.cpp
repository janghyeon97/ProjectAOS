// Fill out your copyright notice in the Description page of Project Settings.


#include "Plugins/ExpressionEvaluator.h"
#include <stack>
#include <sstream>
#include <cmath>
#include <algorithm>

ExpressionEvaluator::ExpressionEvaluator()
{
}

ExpressionEvaluator::~ExpressionEvaluator()
{
}

/*
 * Evaluate 함수는 주어진 수식(expression)을 평가하여 결과를 반환합니다. 
 * 수식에는 변수와 연산자가 포함될 수 있으며, 변수의 값은 전달된 변수 맵(variables)에서 찾아 사용합니다. 
 * 수식은 중위 표기법으로 작성되며, 연산자 우선순위를 고려하여 평가됩니다.

 * 주요 단계:
 * 1. 수식을 토큰으로 분리합니다(Tokenize).
 * 2. 토큰을 순차적으로 읽으면서 값을 스택에 저장하고, 연산자는 별도의 스택에 저장합니다.
 * 3. 괄호와 연산자를 처리하여 스택의 값을 적절히 계산합니다.
 * 4. 모든 토큰을 처리한 후, 남아있는 연산자를 처리하여 최종 결과를 반환합니다.

 * 사용된 자료구조:
 * - std::stack<double>: 피연산자 값을 저장하는 스택입니다.
 * - std::stack<char>: 연산자를 저장하는 스택입니다.

 *  사용된 보조 함수:
 * - Tokenize: 수식을 문자열 토큰으로 변환합니다.
 * - Precedence: 연산자의 우선순위를 반환합니다.
 * - applyOperator: 스택에서 값을 꺼내 연산을 적용하고 결과를 다시 스택에 저장합니다.
 */
double ExpressionEvaluator::Evaluate(const std::string& expression, const std::unordered_map<std::string, double>& variables)
{
    std::istringstream tokens(Tokenize(expression));
    std::stack<double> values;
    std::stack<char> operators;

    // applyOperator 람다 함수는 주어진 연산자(op)를 스택의 상위 두 값에 적용하여 결과를 다시 스택에 저장합니다.
    // - values: 피연산자 값을 저장하는 스택입니다.
    // - op: 적용할 연산자입니다.
    auto applyOperator = [&](char op)
        {
            double right = values.top(); values.pop();
            double left = values.top(); values.pop();
            switch (op)
            {
            case '+': values.push(left + right); break;
            case '-': values.push(left - right); break;
            case '*': values.push(left * right); break;
            case '/': values.push(left / right); break;
            case '^': values.push(std::pow(left, right)); break;
            default: throw std::runtime_error("Unknown operator");
            }
        };


    /**
     * 토큰 처리 루프는 주어진 수식을 토큰 단위로 읽어들여 적절히 처리합니다.
     * - tokens: 토큰 스트림입니다.
     * - token: 현재 처리 중인 토큰입니다.
     *
     * 주요 작업:
     * 1. 숫자 및 소수점 토큰 처리:
     *    - 토큰이 숫자이거나 소수점이면 전체 숫자를 읽어들여 values 스택에 저장합니다.
     * 2. 변수 토큰 처리:
     *    - 토큰이 알파벳이면 전체 변수를 읽어들여 변수 맵에서 값을 찾아 values 스택에 저장합니다.
     *    - 변수를 찾을 수 없으면 예외를 던집니다.
     * 3. 여는 괄호 '(' 토큰 처리:
     *    - 여는 괄호를 operators 스택에 저장합니다.
     * 4. 닫는 괄호 ')' 토큰 처리:
     *    - 닫는 괄호를 만나면 여는 괄호를 만날 때까지 operators 스택의 연산자를 처리합니다.
     *    - 여는 괄호를 operators 스택에서 제거합니다.
     * 5. 연산자 토큰 처리:
     *    - 연산자를 만나면 operators 스택의 상위 연산자와 현재 연산자의 우선순위를 비교하여, 상위 연산자의 우선순위가 높거나 같으면 상위 연산자를 처리합니다.
     *    - 이후 현재 연산자를 operators 스택에 저장합니다.
     */
    char token;
    while (tokens >> token)
    {
        if (std::isdigit(token) || token == '.')
        {
            tokens.putback(token);
            double value;
            tokens >> value;
            values.push(value);
        }
        else if (std::isalpha(token))
        {
            tokens.putback(token);
            std::string var;
            tokens >> var;
            auto it = variables.find(var);
            if (it == variables.end())
            {
                throw std::runtime_error("Unknown variable: " + var);
            }
            values.push(it->second);
        }
        else if (token == '(')
        {
            operators.push(token);
        }
        else if (token == ')')
        {
            while (!operators.empty() && operators.top() != '(')
            {
                applyOperator(operators.top());
                operators.pop();
            }
            operators.pop();
        }
        else if (std::strchr("+-*/^", token))
        {
            while (!operators.empty() && Precedence(operators.top()) >= Precedence(token))
            {
                applyOperator(operators.top());
                operators.pop();
            }
            operators.push(token);
        }
    }

    while (!operators.empty())
    {
        applyOperator(operators.top());
        operators.pop();
    }

    return values.top();
}

/**
 * Tokenize 함수는 주어진 수식을 개별 토큰으로 분리합니다.
 * - expression: 입력 수식 문자열입니다.
 *
 * 주요 작업:
 * 1. 수식의 각 문자를 순회합니다.
 * 2. 공백 문자는 무시합니다.
 * 3. 연산자, 괄호, 숫자, 소수점 또는 알파벳 문자인 경우 result 문자열에 추가합니다.
 * 4. 알파벳 문자의 경우, 변수 이름을 구분하기 위해 뒤에 공백을 추가합니다.
 *
 * 결과적으로, 공백으로 구분된 토큰 문자열을 반환합니다.
 */
std::string ExpressionEvaluator::Tokenize(const std::string& expression)
{
    std::string result;
    for (char ch : expression)
    {
        if (std::isspace(ch)) continue;
        if (std::strchr("+-*/^()0123456789.", ch) || std::isalpha(ch))
        {
            result.push_back(ch);
            if (std::isalpha(ch))
            {
                result.push_back(' ');
            }
        }
    }
    return result;
}

int ExpressionEvaluator::Precedence(char op)
{
    switch (op)
    {
    case '+':
    case '-': return 1;
    case '*':
    case '/': return 2;
    case '^': return 3;
    default: return 0;
    }
}
