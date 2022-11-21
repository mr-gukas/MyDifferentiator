#include "diff.h"

#ifdef LOG_MODE
    FILE* LogFile = startLog(LogFile);
#endif

int main(void)
{
	Tree_t* tree = DataDownload();

	FILE* texFile = fopen("obj/texfile.tex", "w+");
	StartTexPrint(texFile);

	Tree_t diffTree = {};
	TreeCtor(&diffTree);
	free(diffTree.root);

	TreeTexPrint(tree, texFile);

	diffTree.root = DiffTree(tree->root);
	TreeUpdate(&diffTree, diffTree.root);
	TreeDump(&diffTree);		

	TreeSimplifyConst(&diffTree, diffTree.root);
	TreeDump(&diffTree);		

	TreeSimplifyNeutral(&diffTree, diffTree.root);
	TreeDump(&diffTree);		

	TreeTexPrint(&diffTree, texFile);
	
	EndTexPrint(texFile);

	fclose(texFile);
	
	TreeDtor(&diffTree);

	system ("pdflatex obj/texfile.tex");
	system ("xdg-open texfile.pdf");

	TreeDtor(tree);
	free(tree);

#ifdef LOG_MODE
    endLog(LogFile);
#endif

	return 0;
}

Tree_t* DataDownload(void)
{
	FILE* loadData = fopen("obj/expression", "r");
	if (loadData == NULL)
	{
		fprintf(stderr, "..can not connect to the data\n");
		return NULL;
	}
 
	Tree_t* tree = (Tree_t*) calloc(1, sizeof(Tree_t));
	TreeCtor(tree);
	
	TEXT data = {};
	textCtor(&data, loadData);
	fclose(loadData);
	
	ReadTree(tree, data.lines[0].lineStart, data.lines[0].lineLen);

	textDtor(&data);

	return tree;
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
					 "\\usepackage{textcomp}\n"
				 	 "\\setlength{\\oddsidemargin}{-0.4in}\n"
					 "\\setlength{\\evensidemargin}{-0.4in}\n"
					 "\\setlength{\\textwidth}{7in}\n"
					 "\\setlength{\\parindent}{0ex}\n"
					 "\\setlength{\\parskip}{1ex}\n"
					 "\\newcommand\\round[1]{\left[#1\right]}\n"
					 "\\begin{document}\n"
					 "\\maketitle\n"
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
		case Type_NUM: fprintf(texFile, "%lg", node->numVal); break;
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
			return MAKE_NUM(0); 
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
	
	if (node->left)
		newNode->left = TrNodeCopy(node->left);
	
	if (node->right)
		newNode->right = TrNodeCopy(node->right);

	return newNode;
}

int TreeSimplifyConst(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
		
	int oldSize = 0;

	if (node == tree->root)
		oldSize = tree->size;

	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = SimplifyConst(node, tree);

	if (!isSimpled && node && node->left)
		TreeSimplifyConst(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyConst(tree, node->right);

	if (node == tree->root)
	{
		TreeUpdate(tree, tree->root);
		if (tree->size < oldSize)
			return 1;
		
		return 0;
	}

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

		if (node->parent && node->parent->left && node->parent->left == node)                                               
			node->parent->left = newNode;                                            
		else if (node->parent && node->parent->right && node->parent->right == node)       
			node->parent->right = newNode;                                        
		else if (node == tree->root)
			tree->root = newNode;

		TrNodeRemove(tree, node);
			
		isSimpled = 1;
	}
	
	return isSimpled;
}

int TreeSimplifyNeutral(Tree_t* tree, TreeNode_t* node)
{
	if (node == NULL || tree == NULL) return WRONG_DATA;
	
	int oldSize = 0;

	if (node == tree->root)
		oldSize = tree->size;

	if (node->left == NULL && node->right == NULL) return 0;
	
	int isSimpled = SimplifyNeutral(node, tree);

	if (!isSimpled && node && node->left)
		TreeSimplifyNeutral(tree, node->left);

	if (!isSimpled && node && node->right)
		TreeSimplifyNeutral(tree, node->right);

	if (node == tree->root)
	{
		TreeUpdate(tree, tree->root);
		if (tree->size < oldSize)
			return 1;
		
		return 0;
	}

	return 0;
}

int SimplifyNeutral(TreeNode_t* node, Tree_t* tree)
{
	if (node == NULL) return WRONG_DATA;
	
	int isSimpled = 0;
	
	int isRight = -1;
	int isLeft  = -1;

	if (node->right->type == Type_NUM)
	{
		if (node->right->numVal == 0)
			isRight = 0;
		else if (node->right ->numVal == 1)
			isRight = 1;
	}
	
	if (node->left->type == Type_NUM)
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

	if (node->parent && node->parent->left && node->parent->left == node)                                               
			node->parent->left = newNode;                                            
	else if (node->parent && node->parent->right && node->parent->right == node)       
			node->parent->right = newNode;                                        
	else if (node == tree->root)
				tree->root = newNode;
					
	TrNodeRemove(tree, node);
		
	isSimpled = 1;
	
	return isSimpled;
}


