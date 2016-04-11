#ifndef __FRONTEND__H__
#define __FRONTEND__H__

/*************************************************************************
*	Name:			Mark Harman
*	Date:			11/11/15
*	Assignment:		Project 5
*	Description:	
*
**************************************************************************/
#pragma warning(disable: 4996)

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>


extern "C" {
#include "compiler.h"
}

using namespace std;

/* --------------- FORWARD DECLARATIONS --------------- */

struct StatementNode*		MYPARSER();
struct StatementNode*		body();
struct StatementNode*		stmt();
struct StatementNode*		case_list();
struct PrintStatement*		print_stmt();
struct AssignmentStatement* assign_stmt();
struct IfStatement*			if_stmt(StatementNode*);
struct IfStatement*			while_stmt(StatementNode*,StatementNode*);
struct ValueNode*			condition();
struct ValueNode*			primary();


/* --------------- GLOBAL DECLARATIONS --------------- */

vector<ValueNode*> variables;

/* --------------- PARSING FUNCTIONS ---------------*/

void syntax_error(const char* msg)
{
	cout << "Syntax error while parsing: " << msg << "\n" << endl;
	//printf("Syntax error while parsing %s line %d\n", msg);
	exit(1);
}

ValueNode* getID(char* id)
{
	for (int i = 0; i < variables.size(); i++)
	{
		char* one = id;
		char* two = variables[i]->name;
		if (strcmp(one, two) == 0)
		{
			return variables[i];
		}			
	}
	syntax_error("getID. id not found");
}

ValueNode* primary()
{
	ValueNode* prim;
	
	t_type = getToken();
	if (t_type == ID)
	{
		prim = new ValueNode;
		prim = getID(token);
		return prim;
	}
	else if (t_type == NUM)
	{
		prim = new ValueNode;
		prim->value = atoi(token);
		return prim;
	}
	else
	{
		syntax_error("primary. ID or NUM expected");
	}
}

ValueNode* condition()
{
	ValueNode* cond;

	t_type = getToken();
	if (t_type == ID || t_type == NUM)
	{
		ungetToken();
		cond = new ValueNode;
		cond = primary();
		return cond;
	}
}

void setNextNodeToNoop(StatementNode* true_branch, StatementNode* noopNode)
{
	if (true_branch->next == NULL)
		true_branch->next = noopNode;
	else
		setNextNodeToNoop(true_branch->next, noopNode);
}

void setNextNodeToGoTo(StatementNode* true_branch, GotoStatement* goat)
{
	if (true_branch->next == NULL)
		true_branch->next = goat->target;
	else
		setNextNodeToGoTo(true_branch->next, goat);
}

IfStatement* case_list(ValueNode* valNode, StatementNode* noopNode)
{
	IfStatement* iffer = new IfStatement;
	ValueNode* cond2 = new ValueNode;

	t_type = getToken();
	if (t_type == CASE)
	{
		t_type = getToken();
		if (t_type == NUM)
		{
			ungetToken();
			cond2 = primary();
			iffer->condition_operand1 = valNode;
			iffer->condition_operand2 = cond2;
			iffer->condition_op = CONDITION_NOTEQUAL;

			t_type = getToken();
			if (t_type == COLON)
			{
				iffer->false_branch = body();
				setNextNodeToNoop(iffer->false_branch, noopNode);

				t_type = getToken();
				if (t_type == CASE || t_type == DEFAULT)
				{
					ungetToken();
					iffer->true_branch = new StatementNode;
					iffer->true_branch->type = IF_STMT;
					iffer->true_branch->if_stmt = case_list(valNode, noopNode);

					return iffer;
				}
				else if (t_type == RBRACE)
				{
					ungetToken();
					iffer->true_branch = noopNode;
					return iffer;
				}
			}
		}
	}
	if (t_type == DEFAULT)
	{
		t_type = getToken();
		if (t_type == COLON)
		{
			cond2->value = 0;
			iffer->condition_operand1 = valNode;
			iffer->condition_operand2 = cond2;
			iffer->condition_op = CONDITION_NOTEQUAL;
			
			iffer->true_branch = iffer->false_branch;
			iffer->false_branch = body();
			setNextNodeToNoop(iffer->false_branch, noopNode);
			iffer->true_branch = iffer->false_branch;

			t_type = getToken();
			if (t_type == RBRACE)
			{
				ungetToken();
				return iffer;
			}
		}
	}
}

StatementNode* switch_stmt(StatementNode* noopNode)
{
	StatementNode* stm = new StatementNode;
	ValueNode* valNode = new ValueNode;

	t_type = getToken();
	if (t_type = SWITCH)
	{
		t_type = getToken();
		if (t_type == ID)
		{
			ungetToken();
			valNode = primary();

			t_type = getToken();
			if (t_type == LBRACE)
			{
				stm->if_stmt = case_list(valNode, noopNode);

				t_type = getToken();
				if (t_type == RBRACE)
				{
					return stm;
				}
			}
		}
	}
}

IfStatement* while_stmt(StatementNode* stm, StatementNode* noopNode)
{
	IfStatement* whilst;
	ValueNode* cond1;
	ValueNode* cond2;

	GotoStatement* goat = new GotoStatement;

	t_type = getToken();
	if (t_type == WHILE)
	{
		whilst = new IfStatement;
		cond1 = new ValueNode;

		t_type = getToken();
		if (t_type == ID || t_type == NUM)
		{
			ungetToken();
			cond1 = primary();
			whilst->condition_operand1 = cond1;

			t_type = getToken();
			if (t_type == GREATER)
			{
				whilst->condition_op = CONDITION_GREATER;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					cond2 = primary();
					whilst->condition_operand2 = cond2;

					whilst->true_branch = body();
					goat->target = stm;
					setNextNodeToGoTo(whilst->true_branch, goat);
					whilst->false_branch = noopNode;

					return whilst;
				}

			}
			else if (t_type == LESS)
			{
				whilst->condition_op = CONDITION_LESS;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					cond2 = primary();
					whilst->condition_operand2 = cond2;

					whilst->true_branch = body();
					goat->target = stm;
					setNextNodeToGoTo(whilst->true_branch, goat);
					whilst->false_branch = noopNode;

					return whilst;
				}
			}
			else if (t_type == NOTEQUAL)
			{
				whilst->condition_op = CONDITION_NOTEQUAL;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					cond2 = primary();
					whilst->condition_operand2 = cond2;

					whilst->true_branch = body();
					goat->target = stm;
					setNextNodeToGoTo(whilst->true_branch, goat);
					whilst->false_branch = noopNode;

					return whilst;
				}
			}
		}
	}
}

IfStatement* if_stmt(StatementNode* noopNode)
{
	IfStatement* iffer;
	ValueNode* cond1;
	ValueNode* cond2;

	t_type = getToken();
	if (t_type == IF)
	{
		iffer = new IfStatement;
		cond1 = new ValueNode;

		t_type = getToken();
		if (t_type == ID || t_type == NUM)
		{
			ungetToken();
			cond1 = primary();
			iffer->condition_operand1 = cond1;

			t_type = getToken();
			if (t_type == GREATER)
			{
				iffer->condition_op = CONDITION_GREATER;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					cond2 = primary();
					iffer->condition_operand2 = cond2;

					iffer->true_branch = body();
					setNextNodeToNoop(iffer->true_branch, noopNode);
					iffer->false_branch = noopNode;

					return iffer;
				}
			}
			else if (t_type == LESS)
			{
				iffer->condition_op = CONDITION_LESS;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					cond2 = primary();
					iffer->condition_operand2 = cond2;

					iffer->true_branch = body();
					setNextNodeToNoop(iffer->true_branch, noopNode);
					iffer->false_branch = noopNode;

					return iffer;
				}
			}
			else if (t_type == NOTEQUAL)
			{
				iffer->condition_op = CONDITION_NOTEQUAL;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					cond2 = primary();
					iffer->condition_operand2 = cond2;

					iffer->true_branch = body();
					setNextNodeToNoop(iffer->true_branch, noopNode);
					iffer->false_branch = noopNode;

					return iffer;
				}
			}
		}
	}
	assert(false);
	return NULL;
}

AssignmentStatement* assign_stmt()
{
	AssignmentStatement* assigner;
	ValueNode* leftNode;
	ValueNode* op1;
	ValueNode* op2;

	t_type = getToken();
	if (t_type == ID)
	{
		assigner = new AssignmentStatement;
		leftNode = new ValueNode;

		leftNode = getID(token);
		assigner->left_hand_side = leftNode;

		t_type = getToken();
		if (t_type == EQUAL)
		{
			op1 = new ValueNode;
			op1 = primary();
			assigner->operand1 = op1;

			t_type = getToken();
			if (t_type == PLUS)
			{
				assigner->op = OP_PLUS;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					op2 = new ValueNode;
					op2 = primary();
					assigner->operand2 = op2;

					t_type = getToken();
					if (t_type == SEMICOLON)
					{
						return assigner;
					}
				}			
			}
			else if (t_type == MINUS)
			{
				assigner->op = OP_MINUS;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					op2 = new ValueNode;
					op2 = primary();
					assigner->operand2 = op2;

					t_type = getToken();
					if (t_type == SEMICOLON)
					{
						return assigner;
					}
				}
			}
			else if (t_type == MULT)
			{
				assigner->op = OP_MULT;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					op2 = new ValueNode;
					op2 = primary();
					assigner->operand2 = op2;

					t_type = getToken();
					if (t_type == SEMICOLON)
					{
						return assigner;
					}
				}
			}
			else if (t_type == DIV)
			{
				assigner->op = OP_DIV;
				t_type = getToken();
				if (t_type == ID || t_type == NUM)
				{
					ungetToken();
					op2 = new ValueNode;
					op2 = primary();
					assigner->operand2 = op2;

					t_type = getToken();
					if (t_type == SEMICOLON)
					{
						return assigner;
					}
				}
			}
			else if (t_type == SEMICOLON)
			{
				assigner->op = OP_NOOP;
				return assigner;
			}

		}
	}
	assert(false);
	return NULL;
}

PrintStatement* print_stmt()
{
	PrintStatement* stmt;
	ValueNode* valNode;

	t_type = getToken();
	if (t_type == PRINT)
	{
		stmt = new PrintStatement;
		valNode = new ValueNode;

		t_type = getToken();
		if (t_type == ID)
		{
			valNode = getID(token);
			stmt->id = valNode;
			t_type = getToken();
			if (t_type == SEMICOLON)
			{
				return stmt;
			}
		}
	}
	assert(false);
	return NULL;
}

StatementNode* stmt()
{
	StatementNode* stm;

	t_type = getToken();
	if (t_type == ID) // assign stmt
	{
		stm = new StatementNode;
		ungetToken();

		StatementNode* noopNode = new StatementNode;
		noopNode->type = NOOP_STMT;
		stm->next = noopNode;

		stm->assign_stmt = assign_stmt();
		stm->type = ASSIGN_STMT;
		
		t_type = getToken();
		if (t_type == ID || t_type == PRINT || t_type == IF ||
			t_type == SWITCH || t_type == WHILE)
		{
			ungetToken();
			noopNode->next = stmt();
			return stm;
		}
		else
		{
			ungetToken();
			noopNode->next = NULL;
			return stm;
		}
	}
	else if (t_type == PRINT)
	{
		stm = new StatementNode;
		ungetToken();

		StatementNode* noopNode = new StatementNode;
		noopNode->type = NOOP_STMT;
		stm->next = noopNode;

		stm->print_stmt = print_stmt();
		stm->type = PRINT_STMT;

		t_type = getToken();
		if (t_type == ID || t_type == PRINT || t_type == IF ||
			t_type == SWITCH || t_type == WHILE)
		{
			ungetToken();
			noopNode->next = stmt();
			return stm;
		}
		else
		{
			ungetToken();
			noopNode->next = NULL;
			return stm;
		}
	}
	else if (t_type == IF)
	{
		stm = new StatementNode;
		ungetToken();

		struct StatementNode* noopNode;
		noopNode = new StatementNode;
		noopNode->type = NOOP_STMT;
		stm->next = noopNode;

		stm->if_stmt = if_stmt(noopNode);
		stm->type = IF_STMT;

		t_type = getToken();
		if (t_type == ID || t_type == PRINT || t_type == IF ||
			t_type == SWITCH || t_type == WHILE)
		{

			ungetToken();
			noopNode->next = stmt();
			return stm;
		}
		else
		{
			ungetToken();
			noopNode->next = NULL;
			return stm;
		}
	}
	else if (t_type == WHILE)
	{
		stm = new StatementNode;
		ungetToken();

		struct StatementNode* noopNode;
		noopNode = new StatementNode;
		noopNode->type = NOOP_STMT;
		stm->next = noopNode;

		stm->type = IF_STMT;
		stm->if_stmt = while_stmt(stm,noopNode);

		t_type = getToken();
		if (t_type == ID || t_type == PRINT || t_type == IF ||
			t_type == SWITCH || t_type == WHILE)
		{
			ungetToken();
			noopNode->next = stmt();
			return stm;
		}
		else
		{
			ungetToken();
			noopNode->next = NULL;
			return stm;
		}
	}
	else if (t_type == SWITCH)
	{
		stm = new StatementNode;
		ungetToken();

		struct StatementNode* noopNode;
		noopNode = new StatementNode;
		noopNode->type = NOOP_STMT;
		stm->next = noopNode;

		stm = switch_stmt(noopNode);
		stm->type = IF_STMT;

		t_type = getToken();
		if (t_type == ID || t_type == PRINT || t_type == IF ||
			t_type == SWITCH || t_type == WHILE)
		{
			ungetToken();
			noopNode->next = stmt();
			return stm;
		}
		else
		{
			ungetToken();
			noopNode->next = NULL;
			return stm;
		}

	}
	assert(false);
	return NULL;
//	return stm;
}

StatementNode* body()
{
	StatementNode* bod;

	t_type = getToken();
	if (t_type == LBRACE) // PARSE BODY
	{
		bod = new StatementNode;
		bod->next = stmt();
		bod->type = NOOP_STMT;

		t_type = getToken();
		if (t_type == RBRACE)
		{
			return bod;
		}
	}
	assert(false);
	return NULL;

}

StatementNode* MYPARSER()
{
	ValueNode* newNode;
	StatementNode* stmt;

	stmt = new StatementNode;

	t_type = getToken();
	while(t_type != SEMICOLON) // PARSE VARIABLES
	{
		if(t_type == ID)
		{
			newNode = new ValueNode;
			newNode->name = strdup(token);
			newNode->value = 0;
			variables.push_back(newNode);
		}
		
		t_type = getToken();
	}

	t_type = getToken();
	if (t_type == LBRACE)
	{
		ungetToken();
		stmt->type = NOOP_STMT;
		stmt->next = body();
		return stmt;
	}	
}

#endif //__FRONTEND__H__
