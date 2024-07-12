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
 * Evaluate �Լ��� �־��� ����(expression)�� ���Ͽ� ����� ��ȯ�մϴ�. 
 * ���Ŀ��� ������ �����ڰ� ���Ե� �� ������, ������ ���� ���޵� ���� ��(variables)���� ã�� ����մϴ�. 
 * ������ ���� ǥ������� �ۼ��Ǹ�, ������ �켱������ ����Ͽ� �򰡵˴ϴ�.

 * �ֿ� �ܰ�:
 * 1. ������ ��ū���� �и��մϴ�(Tokenize).
 * 2. ��ū�� ���������� �����鼭 ���� ���ÿ� �����ϰ�, �����ڴ� ������ ���ÿ� �����մϴ�.
 * 3. ��ȣ�� �����ڸ� ó���Ͽ� ������ ���� ������ ����մϴ�.
 * 4. ��� ��ū�� ó���� ��, �����ִ� �����ڸ� ó���Ͽ� ���� ����� ��ȯ�մϴ�.

 * ���� �ڷᱸ��:
 * - std::stack<double>: �ǿ����� ���� �����ϴ� �����Դϴ�.
 * - std::stack<char>: �����ڸ� �����ϴ� �����Դϴ�.

 *  ���� ���� �Լ�:
 * - Tokenize: ������ ���ڿ� ��ū���� ��ȯ�մϴ�.
 * - Precedence: �������� �켱������ ��ȯ�մϴ�.
 * - applyOperator: ���ÿ��� ���� ���� ������ �����ϰ� ����� �ٽ� ���ÿ� �����մϴ�.
 */
double ExpressionEvaluator::Evaluate(const std::string& expression, const std::unordered_map<std::string, double>& variables)
{
    std::istringstream tokens(Tokenize(expression));
    std::stack<double> values;
    std::stack<char> operators;

    // applyOperator ���� �Լ��� �־��� ������(op)�� ������ ���� �� ���� �����Ͽ� ����� �ٽ� ���ÿ� �����մϴ�.
    // - values: �ǿ����� ���� �����ϴ� �����Դϴ�.
    // - op: ������ �������Դϴ�.
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
     * ��ū ó�� ������ �־��� ������ ��ū ������ �о�鿩 ������ ó���մϴ�.
     * - tokens: ��ū ��Ʈ���Դϴ�.
     * - token: ���� ó�� ���� ��ū�Դϴ�.
     *
     * �ֿ� �۾�:
     * 1. ���� �� �Ҽ��� ��ū ó��:
     *    - ��ū�� �����̰ų� �Ҽ����̸� ��ü ���ڸ� �о�鿩 values ���ÿ� �����մϴ�.
     * 2. ���� ��ū ó��:
     *    - ��ū�� ���ĺ��̸� ��ü ������ �о�鿩 ���� �ʿ��� ���� ã�� values ���ÿ� �����մϴ�.
     *    - ������ ã�� �� ������ ���ܸ� �����ϴ�.
     * 3. ���� ��ȣ '(' ��ū ó��:
     *    - ���� ��ȣ�� operators ���ÿ� �����մϴ�.
     * 4. �ݴ� ��ȣ ')' ��ū ó��:
     *    - �ݴ� ��ȣ�� ������ ���� ��ȣ�� ���� ������ operators ������ �����ڸ� ó���մϴ�.
     *    - ���� ��ȣ�� operators ���ÿ��� �����մϴ�.
     * 5. ������ ��ū ó��:
     *    - �����ڸ� ������ operators ������ ���� �����ڿ� ���� �������� �켱������ ���Ͽ�, ���� �������� �켱������ ���ų� ������ ���� �����ڸ� ó���մϴ�.
     *    - ���� ���� �����ڸ� operators ���ÿ� �����մϴ�.
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
 * Tokenize �Լ��� �־��� ������ ���� ��ū���� �и��մϴ�.
 * - expression: �Է� ���� ���ڿ��Դϴ�.
 *
 * �ֿ� �۾�:
 * 1. ������ �� ���ڸ� ��ȸ�մϴ�.
 * 2. ���� ���ڴ� �����մϴ�.
 * 3. ������, ��ȣ, ����, �Ҽ��� �Ǵ� ���ĺ� ������ ��� result ���ڿ��� �߰��մϴ�.
 * 4. ���ĺ� ������ ���, ���� �̸��� �����ϱ� ���� �ڿ� ������ �߰��մϴ�.
 *
 * ���������, �������� ���е� ��ū ���ڿ��� ��ȯ�մϴ�.
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
