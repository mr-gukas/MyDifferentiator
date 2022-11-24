#include "diff.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif

int main(void)
{
	Expression_t exp = {};

	DataDownload(&exp);
	
	FILE* texFile = fopen("obj/texfile.tex", "w+");
	StartTexPrint(texFile);

	TreeTexPrint(exp.tree, texFile);

	Tree_t* diffTree = DiffExpression(exp.tree, exp.derOrd);
	TreeTexPrint(diffTree, texFile);
	
	double val = CalcValue(diffTree, exp.point);
	fprintf(texFile, "\n$%lg$\n", val);
	
	Maclaurin(&exp, texFile);
	TangentEquation(&exp, texFile);
	BuildGraph(&exp, texFile);

	EndTexPrint(texFile);

	fclose(texFile);

	system ("pdflatex obj/texfile.tex");
	system ("xdg-open texfile.pdf");
	
	TreeDtor(diffTree);
	free(diffTree);
	TreeDtor(exp.tree);
	free(exp.tree);

#ifdef LOG_MODE
    endLog(LogFile);
#endif

	return 0;
}

int DataDownload(Expression_t* exp)
{
	FILE* loadData = fopen("obj/expression", "r");
	if (loadData == NULL)
	{
		fprintf(stderr, "..can not connect to the data\n");
		return NULL;
	}
 
	exp->tree = (Tree_t*) calloc(1, sizeof(Tree_t));
	TreeCtor(exp->tree);
	
	TEXT data = {};
	textCtor(&data, loadData);
	fclose(loadData);
	
	ReadTree(exp->tree, data.lines[0].lineStart, data.lines[0].lineLen);
	
	if ((sscanf(data.lines[1].lineStart, "%lu", &exp->derOrd) != 1) ||
		(sscanf(data.lines[2].lineStart, "%lf", &exp->point)  != 1) ||
		(sscanf(data.lines[3].lineStart, "%lu", &exp->macOrd) != 1))
	{
		return WRONG_DATA;
	}

	textDtor(&data);

	return STATUS_OK;
}

int ReadTree(Tree_t* tree, const char* expression, size_t size)
{
	if (expression == NULL) return WRONG_DATA;

	TreeNode_t* curNode = tree->root;
	

	for (size_t index = 0; index < size; ++index)
	{

		if (*(expression + index) == ')')
		{
			curNode = curNode->parent;
			continue;
		}
		else if (*(expression + index) == '(')
		{
			if (curNode->left != NULL)
			{
				TrNodeInsert(tree, curNode, RIGHT);
				curNode = curNode->right;
				continue;
			}
			else
			{
				TrNodeInsert(tree, curNode, LEFT);
				curNode = curNode->left;
				continue;
			}
		}
		else 
		{
			index += ReadNodeValue(curNode, expression + index);	

		}	
	
		if (curNode->parent->opVal == Op_Sin ||
			curNode->parent->opVal == Op_Cos ||
			curNode->parent->opVal == Op_Ln)
		{
			curNode->parent->right = curNode;
			curNode->parent->left  = NULL;
		}
	}

	tree->root = tree->root->left;
	free(tree->root->parent);

	tree->root->parent = NULL;
	tree->size--;
	
	return STATUS_OK; 
}

static int ReadNodeValue(TreeNode_t* curNode, const char* expression)
{
	int    charCount = 0;
	double dblVal    = 0;

	if (sscanf(expression, "%lf%n", &dblVal, &charCount) == 1)
	{
		curNode->type   = Type_NUM;
		curNode->numVal = dblVal;
	
		return charCount - 1;
	}

	char*  value     = (char*) calloc(STR_MAX_SIZE, sizeof(char)); 

	if (sscanf(expression, "%[^()]%n", value, &charCount))
	{
		if (strchr(value, '+'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Add;
		}
		else if (strchr(value, '-'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Sub;
		}
		else if (strchr(value, '*'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Mul;
		}
		else if (strchr(value, '/'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Div;
		}
		else if (strchr(value, '^'))
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Pow;
		}
		else if (strcmp(value, "sin") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Sin;
		}
		else if (strcmp(value, "cos") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Cos;
		}
		else if (strcmp(value, "ln") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Ln;
		}
		else if (strcmp(value, "log") == 0)
		{
			curNode->type  = Type_OP;
			curNode->opVal = Op_Log;
		}
		else
		{
			curNode->type     = Type_VAR;
			strcpy(curNode->varVal, value);
		}
		
		free(value);
		
		return charCount - 1;
	}
	
	return charCount;
}

int StartTexPrint(FILE* texFile)
{
	if (texFile == NULL) return WRONG_DATA;
	
	fprintf(texFile, "\\documentclass{article}\n"
				     "\\usepackage[utf8]{inputenc}\n"
					 "\\title{Моя жизнь на кафедре вышмата, или как армяне в нарды играли на физтехе}\n"
					 "\\author{Владимир Гукасян aka mr.gukas}\n"
					 "\\date{Очереднойбрь 1984}\n"
					 "\\usepackage[T2A]{fontenc}\n"
					 "\\usepackage[utf8]{inputenc}\n"
					 "\\usepackage[english,russian]{babel}\n"
					 "\\usepackage{amsthm}\n"
					 "\\usepackage{amsmath}\n"
					 "\\usepackage{amssymb}\n"
					 "\\usepackage{tikz}\n"
					 "\\usepackage{graphicx}\n"
					 "\\graphicspath{ {./images/} }\n"
					 "\\usepackage{textcomp}\n"
				 	 "\\setlength{\\oddsidemargin}{-0.4in}\n"
					 "\\setlength{\\evensidemargin}{-0.4in}\n"
					 "\\setlength{\\textwidth}{7in}\n"
					 "\\setlength{\\parindent}{0ex}\n"
					 "\\setlength{\\parskip}{1ex}\n"
					 "\\newcommand\\round[1]{\left[#1\right]}\n"
					 "\\begin{document}\n"
					 "\\maketitle\n"
					 //"\\includegraphics[scale=0.3]{obj/math.jpeg}\n"
					 "\\newpage\n");
	
	return STATUS_OK;
}

int EndTexPrint(FILE* texFile)
{
	if (texFile == NULL) return WRONG_DATA;

	fprintf(texFile, "\\end{document}\n");

	return STATUS_OK;
}

int TreeTexPrint(const Tree_t* tree, FILE* texFile)
{
	if (tree    == NULL) return NULL_TREE;
	if (texFile == NULL) return WRONG_DATA;
	
	fprintf(texFile, "\n$");

	TrNodesPrint(tree->root, texFile);

	fprintf(texFile, "$\n");

	return STATUS_OK;
}

int TrNodesPrint(const TreeNode_t* node, FILE* texFile)
{
	if (node == NULL || texFile == NULL) return WRONG_DATA;
	
	if ((node->left != NULL || node->right != NULL) &&
		!(node->opVal >= 4 && node->opVal <= 10)    &&
		node->parent != NULL                        &&
		!(node->opVal - node->parent->opVal > 1))

		fprintf(texFile, "(");

#define OperPrint_(str)               \
	fprintf(texFile, "{");			  \
	TrNodesPrint(node->left, texFile); \
	fprintf(texFile, "}" #str "{");       \
	TrNodesPrint(node->right, texFile);  \
	fprintf(texFile, "}");

	switch (node->type)
	{
		case Type_VAR: fprintf(texFile, "%s", node->varVal);  break; 
		case Type_NUM: 
		{
			if (node->numVal < 0)
				fprintf(texFile, "(");
			fprintf(texFile, "%lg", node->numVal); 
			if (node->numVal < 0)
				fprintf(texFile, ")");

			break;
		}
		case Type_OP:  
		{
			switch (node->opVal)
			{
				case Op_Add:
					OperPrint_(+);
					break;
				case Op_Sub: 
					OperPrint_(-);
					break;
			
				case Op_Mul:
					OperPrint_(\\cdot);
					break;
				case Op_Div:
					fprintf(texFile, "\\frac{");
					TrNodesPrint(node->left, texFile);
					fprintf(texFile, "}{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Log:
					fprintf(texFile, "\\log_{");
					TrNodesPrint(node->left, texFile);
					fprintf(texFile, "}{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Ln: 
					fprintf(texFile, "\\ln{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Sin:
					fprintf(texFile, "\\sin{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Cos: 
					fprintf(texFile, "\\cos{");
					TrNodesPrint(node->right, texFile);
					fprintf(texFile, "}");
					break;

				case Op_Pow:
					OperPrint_(^);
					break;

				default: fprintf(texFile, "???");
			}

			break;
		}

		default: fprintf(texFile, "???");
	}

	if ((node->left != NULL || node->right != NULL) &&
		!(node->opVal >= 4 && node->opVal <= 10)    &&
		node->parent != NULL                        &&
		!(node->opVal - node->parent->opVal > 1))

		fprintf(texFile, ")");

#undef OperPrint_

	return STATUS_OK;
}

TreeNode_t* DiffTree(TreeNode_t* node)
{
	if (node == NULL) return NULL;
	
	switch (node->type)
	{
		case Type_NUM:
		{
			return MAKE_NUM(0); 
		}
		case Type_VAR:
		{
			if (strcmp(node->varVal, "x") == 0)
				return MAKE_NUM(1);
			else
				return MAKE_NUM(0);
		}
		case Type_OP:
		{
			switch(node->opVal)
			{
				case Op_Add: return ADD(DL, DR); 
				case Op_Sub: return SUB(DL, DR);
				case Op_Mul: return ADD(MUL(DL, CR), MUL(CL, DR));				
				case Op_Div: return DIV(SUB(MUL(DL,CR), MUL(CL, DR)), MUL(CR, CR));
				case Op_Sin: return MUL(DR, COS(CR));
				case Op_Cos: return MUL(DR, MUL(MAKE_NUM(-1), SIN(CR)));
				case Op_Ln : return MUL(DR, DIV(MAKE_NUM(1), CR));
				case Op_Log: return	DIV(MUL(DR, DIV(MAKE_NUM(1), CR)), MUL(DL, DIV(MAKE_NUM(1), CL)));
				case Op_Pow:
				{
					int inLeft  = IsInTree(node->left, "x");
					int inRight = IsInTree(node->right, "x");
					
					if (inLeft && inRight)
					{
						return MUL(POW(CL, CR), ADD(MUL(DR, LN(CR)), MUL(CR, MUL(DL, DIV(MAKE_NUM(1), CL))))); 
					}
					else if (inLeft)
					{
						return MUL(MUL(CR, POW(CL, SUB(CR, MAKE_NUM(1)))), DL);
					}
					else if (inRight)
					{
						return MUL(MUL(LN(CL), POW(CL, CR)), DR);
					}
					else
						return MAKE_NUM(1); 

					break;
				
				}
				
				default: break;
			}
		}
	}
}


int IsInTree(TreeNode_t* node, const char* value)
{
	if (node == NULL || value == NULL) return WRONG_DATA;
	
	if (node->type == Type_VAR && strcmp(node->varVal, value) == 0)
		return 1;
	
	else if (node->left == NULL && node->right == NULL)
		return 0;

	if (IsInTree(node->left, value))
		return 1;
	if (IsInTree(node->right, value))
		return 1;

	return 0;
}

TreeNode_t* TrNodeCopy(TreeNode_t* node)
{
	if (node == NULL) return NULL;

	TreeNode_t* newNode = (TreeNode_t*) calloc(1, sizeof(TreeNode_t));
	if (newNode == NULL)
		return NULL;

	newNode->type   = node->type;
	newNode->opVal  = node->opVal;
	newNode->numVal = node->numVal;
	strcpy(newNode->varVal, node->varVal);
	
	if (node->left != nullptr)
		newNode->left = TrNodeCopy(node->left);
	
	if (node->right != nullptr)
		newNode->right = TrNodeCopy(node->right);

	return newNode;
}

int TreeSimplify(Tree_t* tree)
{
	if (tree == NULL) return NULL_TREE;
		
	int     isSimpled = 0;
	size_t	oldSize   = tree->size;
	do 
	{
		isSimpled = 0;
		
		TreeSimplifyConst(tree, tree->root);
		if (tree->size < oldSize)
		{
			isSimpled += 1;
			oldSize   = tree->size;
		}

		TreeSimplifyNeutral(tree, tree->root);
		if (tree->size < oldSize)
		{
			isSimpled += 1;
			oldSize    = tree->size;
		}
	} while (isSimpled);
	
	return STATUS_OK;
}


int TreeSimplifyConst(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
		
	int oldSize = 0;
	
	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = SimplifyConst(node, tree);
	TreeUpdate(tree, tree->root);

	if (!isSimpled && node && node->left)
		TreeSimplifyConst(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyConst(tree, node->right);

	TreeUpdate(tree, tree->root);
	return 0;
}

int SimplifyConst(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;

	if (node->right->type == Type_NUM) 
	{                                              
		double newNum = 0;

		if (node->left != NULL && node->left->type == Type_NUM)				
		{
			switch (node->opVal)
			{
				case Op_Add: newNum = node->left->numVal + node->right->numVal;		break;
				case Op_Sub: newNum = node->left->numVal - node->right->numVal;		break;
				case Op_Mul: newNum = node->left->numVal * node->right->numVal;		break;
				case Op_Div: newNum = node->left->numVal / node->right->numVal;		break;
				case Op_Pow: newNum = pow(node->left->numVal, node->right->numVal); break;

				default: break;
			}
		}
		else if (node->left == NULL)	
		{
			switch (node->opVal)
			{
				case Op_Sin: newNum = sin(node->right->numVal);	break;
				case Op_Cos: newNum = cos(node->right->numVal);	break;
				case Op_Ln:	 newNum = log(node->right->numVal);	break;

				default: break;
			}

		}
		
		else 
			return isSimpled;

		TreeNode_t* newNode = MakeNode(Type_NUM, Op_Null, newNum, NULL, NULL, NULL);  

		if (node == tree->root)
			tree->root = newNode;
		if (node->parent && node->parent->left && node->parent->left == node)                                               
				node->parent->left = newNode;                                            
		else if (node->parent && node->parent->right && node->parent->right == node)       
				node->parent->right = newNode;                                        
		
		TrNodeRemove(tree, node);
		isSimpled = 1;
	}
	
	return isSimpled;
}

int TreeSimplifyNeutral(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
	
	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = SimplifyNeutral(node, tree);
	TreeUpdate(tree, tree->root);

	if (!isSimpled && node && node->left)
		TreeSimplifyNeutral(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyNeutral(tree, node->right);

	TreeUpdate(tree, tree->root);

	return 0;
}

int SimplifyNeutral(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;
	
	int isRight = -1;
	int isLeft  = -1;

	if (node->right && node->right->type == Type_NUM)
	{
		if (node->right->numVal == 0)
			isRight = 0;
		else if (node->right ->numVal == 1)
			isRight = 1;
	}
	
	if (node->left && node->left->type == Type_NUM)
	{
		if (node->left->numVal == 0)
			isLeft = 0;
		else if (node->left->numVal == 1)
			isLeft = 1;
	}

	if (isRight == -1 && isLeft == -1)
		return isSimpled;
	
	TreeNode_t* newNode = NULL;
	
	switch (node->opVal)
	{
		case Op_Add:
		{
			if (isLeft == 0)
				newNode = TrNodeCopy(node->right);
			break;
		}
		case Op_Sub:
		{
			if (isRight == 0)
				newNode = TrNodeCopy(node->left);
			break;
		}
		case Op_Mul:
		{
			if (isLeft == 0)
				newNode = TrNodeCopy(node->left);
			else if (isLeft == 1)
				newNode = TrNodeCopy(node->right);
			break;
		}
		case Op_Div:
		{
			if (isLeft == 0 || isRight == 1)
				newNode = TrNodeCopy(node->left);
			break;
		}
		case Op_Pow:
		{
			if (isLeft == 1 || isRight == 1)
				newNode = TrNodeCopy(node->left);
			else if (isRight == 0)
				newNode = MAKE_NUM(1);

			break;
		}
		case Op_Ln:
		{
			if (isRight == 1)
				newNode = MAKE_NUM(0);
			break;
		}

		default: break;
	}
	
	if (newNode == NULL)
		return isSimpled;
	
	if (node == tree->root)
		tree->root = newNode;
	if (node->parent && node->parent->left && node->parent->left == node)                                               
			node->parent->left = newNode;                                            
	else if (node->parent && node->parent->right && node->parent->right == node)       
			node->parent->right = newNode;                                        
					
	TrNodeRemove(tree, node);
		
	isSimpled = 1;
	
	return isSimpled;
}

Tree_t* DiffExpression(Tree_t* tree, size_t derOrd)
{
	if (tree == NULL || tree->root == NULL) return NULL;
	
	Tree_t*	diffTree = (Tree_t*) calloc(1, sizeof(Tree_t));
	Tree_t* curTree = (Tree_t*) calloc(1, sizeof(Tree_t));

	curTree->root = TrNodeCopy(tree->root);
	TreeUpdate(curTree, curTree->root);
	
	for (size_t index = 0; index < derOrd; index++)
	{
		diffTree->root = DiffTree(curTree->root);
		TreeUpdate(diffTree, diffTree->root);
		
		TreeSimplify(diffTree);

		TrNodeRemove(curTree, curTree->root);
		curTree->root = TrNodeCopy(diffTree->root);
		TreeUpdate(curTree, curTree->root);

		TrNodeRemove(diffTree, diffTree->root);

	}

	free(diffTree);

	return curTree;
}

double CalcValue(Tree_t* tree, double point)
{
	if (tree == NULL || tree->root == NULL) return WRONG_DATA;


	Tree_t* cpTree = (Tree_t*) calloc(1, sizeof(Tree_t));
	cpTree->root = TrNodeCopy(tree->root);
	TreeUpdate(cpTree, cpTree->root);

	PutValueInPoint(cpTree->root, point);
	TreeSimplify(cpTree);

	double val = cpTree->root->numVal;
		
	TreeDtor(cpTree);
	free(cpTree);

	return val;
}

int PutValueInPoint(TreeNode_t* node, double point)
{
	if (node == NULL) return WRONG_DATA;
	
	if (node->type == Type_VAR && strcmp(node->varVal, "x") == 0)
	{
		node->type   = Type_NUM;
		node->numVal = point;
	}
	
	if (node->left)
		PutValueInPoint(node->left, point);

	if(node->right)
		PutValueInPoint(node->right, point);

	return STATUS_OK;
}

int Maclaurin(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || exp->tree == NULL || exp->tree->root == NULL) return WRONG_DATA;
	
	TreeTexPrint(exp->tree, texFile);

	fprintf(texFile, "$=");
	
	Tree_t* curTree = NULL;

	double funcVal = CalcValue(exp->tree, 0);
	fprintf(texFile, "%lg +", funcVal);
	
	for (size_t index = 1; index <= exp->macOrd; ++index)
	{
		curTree = DiffExpression(exp->tree, index);
			
		double derivVal = CalcValue(curTree, 0);
		fprintf(texFile, "\\frac{%lg\\cdot x^{%lu}}{%lu}", derivVal, index, factorial(index)); 
		
		if (index != exp->macOrd)
			fprintf(texFile, "+");
		
		TreeDtor(curTree);
		free(curTree);
	}
	
	fprintf(texFile, "+ o(x^{%lu})$\n", exp->macOrd);

	return STATUS_OK;
}

size_t factorial(size_t num)
{
	if (num <= 1) return 1;

	return num * factorial(num - 1);
}

int TangentEquation(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || exp->tree == NULL || exp->tree->root == NULL) return WRONG_DATA;
	
	fprintf(texFile, "\\\\Уравнение касательной к графику в точке %lg примет следующий вид: $y = ", exp->point);
	
	Tree_t* curTree = NULL;

	double funcVal = CalcValue(exp->tree, exp->point);
	
	curTree = DiffExpression(exp->tree, 1);
	double derivVal = CalcValue(curTree, exp->point);

	fprintf(texFile, "%lg\\cdot(x - %lg)+%lg$\n", derivVal, exp->point, funcVal);
		
	TreeDtor(curTree);
	free(curTree);

	return STATUS_OK;
}

int BuildGraph(Expression_t* exp, FILE* texFile)
{
	if (exp == NULL || exp->tree == NULL || exp->tree->root == NULL) return WRONG_DATA;
	
	FILE * gnuplotPipe = popen("gnuplot -persistent", "w");

	fprintf(gnuplotPipe, "set terminal png size 800, 600\n"
						 "set output \"graph.png\"\n"
						 "set xlabel \"X\"\n" 
						 "set ylabel \"Y\"\n"
						 "set grid\n"
						 "set title \"А вы знали, что первыми художниками были армяне?\" font \"Helvetica Bold, 20\"\n"
						 "plot '-' lt 3 linecolor 1 notitle\n");

	for (double x = -20.0; x <= 20.1; x = x + 0.001)
	{
		fprintf(gnuplotPipe, "%lf\t %lf\n", x, CalcValue(exp->tree, x));
	}
	
	fprintf(gnuplotPipe, "e");

	fclose(gnuplotPipe);

	fprintf(texFile,"\\\\\\\\\\includegraphics[scale=0.3]{graph.png}\n");



	return STATUS_OK;
}












	


