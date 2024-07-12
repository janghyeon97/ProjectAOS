// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <unordered_map>
#include <stdexcept>

/**
 * 
 */
class PROJECT_AOS_API ExpressionEvaluator
{
public:
	ExpressionEvaluator();
	~ExpressionEvaluator();

public:
	double Evaluate(const std::string& expression, const std::unordered_map<std::string, double>& variables);

private:
	std::string Tokenize(const std::string& expression);
	int Precedence(char op);
};
