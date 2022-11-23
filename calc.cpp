#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

double GetG(char** str);
double GetE(char** str);
double GetT(char** str);
double GetD(char** str);
double GetP(char** str);
double GetN(char** str);

int main(void)
{
	char* str = (char*) calloc(100, sizeof(char));
	fgets(str, 100, stdin);
	
	char* end = strchr(str, '\n');
	*end = '\0';


	double val = GetG(&str);

	printf("%lg\n", val);
	
	return 0;
}

double GetG(char** str)
{
	double val = GetE(str);

	assert(**str == '\0');

	return val;
}

double GetE(char** str)
{
	double val = GetT(str);

	while (**str == '+' || **str == '-')
	{
		char op = **str;
		(*str)++;

		double val2 = GetT(str);

		if (op == '+')
			val += val2;
		else
			val -= val2;
	}

	return val;
}

double GetT(char** str)
{
	double val = GetD(str);

	while (**str == '*' || **str == '/')
	{
		char op = **str;
		(*str)++;

		double val2 = GetD(str);

		if (op == '*')
			val *= val2;
		else
			val /= val2;
	}

	return val;
}

double GetD(char** str)
{
	double val = GetP(str);

	while (**str == '^') 
	{
		(*str)++;

		double val2 = GetP(str);
		
		val = pow(val, val2);
	}

	return val;
}

double GetP(char** str)
{
	double val = 0;

	if (**str == '(')
	{
		(*str)++;
		val = GetE(str);
		
		assert(**str == ')');

		(*str)++;
	}
	else
		val = GetN(str);

	return val;
}

double GetN(char** str)
{
	double   val   = 0;
	double isNeg   = 0; 

	char* oldStr = *str;
	
	if (**str == '-')
	{
		isNeg = 1;
		(*str)++;
	}

	while ('0' <= **str && **str <= '9')
	{
		val = val * 10 + **str - '0';
		(*str)++;
	}

	assert(*oldStr != **str);

	return (isNeg) ? -val : val;
}




















