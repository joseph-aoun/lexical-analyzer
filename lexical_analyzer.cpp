#include <bits/stdc++.h>
#include "lexer.h"
#include "tasks.h"
using namespace std;
#define all(x) x.begin(), x.end()
#define dbg(x) cerr << x << '\n';

void syntax_error()
{
	cout << "SNYATX EORRR !!!\n";
	exit(1);
}
LexicalAnalyzer lexer;
Token t;
int i = 0;
bool lef = 1;
string output = "";
bool ok = 1;
set<int> line_nums, errors;
vector<string> scalars, arrays;
vector<TokenType> order = {PLUS, MINUS, MULT, DIV, LPAREN, RPAREN, LBRAC, DOT, RBRAC, NUM, ID, END_OF_FILE};
map<TokenType, string> mp = {{PLUS, "+"}, {MINUS, "-"}, {MULT, "*"}, {DIV, "/"}, {LPAREN, "("}, {RPAREN, ")"}, {LBRAC, "["}, {DOT, "."}, {RBRAC, "]"}, {NUM, "E"}, {ID, "ID"}, {END_OF_FILE, "$"}}; // for printing
vector<vector<string>> precedence =
	{{">", ">", "<", "<", "<", ">", "<", "err", ">", "<", "<", ">"},
	 {">", ">", "<", "<", "<", ">", "<", "err", ">", "<", "<", ">"},
	 {">", ">", ">", ">", "<", ">", "<", "err", ">", "<", "<", ">"},
	 {">", ">", ">", ">", "<", ">", "<", "err", ">", "<", "<", ">"},
	 {"<", "<", "<", "<", "<", "=", "<", "err", "<", "<", "<", "err"},
	 {">", ">", ">", ">", "err", ">", ">", "err", ">", "err", "err", ">"},
	 {"<", "<", "<", "<", "<", "<", "<", "=", "=", "<", "<", "err"},
	 {"err", "err", "err", "err", "err", "err", "err", "err", "=", "err", "err", "err"},
	 {">", ">", ">", ">", "err", ">", ">", "err", ">", "err", "err", ">"},
	 {">", ">", ">", ">", "err", ">", ">", "err", ">", "err", "err", ">"},
	 {">", ">", ">", ">", "err", ">", ">", "err", ">", "err", "err", ">"},
	 {"<", "<", "<", "<", "<", "err", "<", "err", "err", "<", "<", "acc"}};
vector<string> RHS = {"E+E", "E-E", "E*E", "E/E", "(E)", "E[.]", "E[E]", "E"};
enum typee
{
	SCALARS,
	ARRAYS,
	ERRORS,
	ARRAYDECL
};
string get_precedence(TokenType t1, TokenType t2)
{
	if (find(all(order), t1) == order.end() || find(all(order), t2) == order.end())
		syntax_error();
	int i1 = find(all(order), t1) - order.begin();
	int i2 = find(all(order), t2) - order.begin();
	return precedence[i1][i2];
}
enum snodeType
{
	EXPR,
	TERM
};
struct stackNode
{
	snodeType type;
	struct exprNode *expr;
	Token term;
	stackNode() {}
};
enum operatorr
{
	ID_OPER,
	PLUS_OPER,
	MINUS__OPER,
	DIV_OPER,
	ARRAY_ELEM_OPER,
	WHOLE_ARRAY_OPER,
	MULT_OPER,
	ASSIGN_OPER,
	NUM_OPER,
	ARRAY_DOT_OPER
};
struct exprNode
{
	operatorr oper;
	typee type;
	struct
	{
		string varName;
		int line_no;
	} id;
	struct
	{
		stackNode *left;
		stackNode *right;
	} child;
	struct
	{
		exprNode *arrayexpr;
		int line_no;
	} array;
};
struct myStack
{
	stack<stackNode *> st;
	myStack()
	{
		stackNode *s = new stackNode();
		s->type = TERM;
		s->term.token_type = END_OF_FILE;
		s->term.lexeme = "$";
		st.push(s);
	}
	void push(myStack &st2, stackNode *e)
	{
		st2.st.push(e);
	}
	void push(myStack &st2, Token t)
	{
		stackNode *x = new stackNode();
		x->type = TERM;
		x->term = t;
		st2.st.push(x);
	}
	stackNode *pop(myStack &st2)
	{
		auto x = st2.st.top();
		st2.st.pop();
		return x;
	}
	stackNode *peek(myStack &st2)
	{
		stack<stackNode *> st3;
		while (st2.st.top()->type == EXPR)
		{
			st3.push(st2.st.top());
			st2.st.pop();
		}
		auto x = st2.st.top();
		while (!st3.empty())
		{
			st2.st.push(st3.top());
			st3.pop();
		}
		return x;
	}
	stackNode *top(myStack &st2)
	{
		return st2.st.top();
	}
	void reduce(myStack &st2, Token t);
	void last_reduce(myStack &st2);
	void make_reduction(string rhs, stack<stackNode *> stt, myStack &st2);
};

bool is_scalars(string s)
{
	return find(all(scalars), s) != scalars.end();
}
bool is_arrays(string s)
{
	return find(all(arrays), s) != arrays.end();
}

myStack st, st1, st2, st3;

void myStack::last_reduce(myStack &st2)
{
	Token x;
	x.lexeme = "$";
	x.token_type = END_OF_FILE;
	st2.reduce(st2, x);
}
void myStack::make_reduction(string rhs, stack<stackNode *> stt, myStack &st5)
{
	stackNode *x = new stackNode();
	queue<stackNode *> st4;
	while (!stt.empty())
	{
		st4.push(stt.top());
		stt.pop();
	}
	while (!st4.empty())
	{
		stt.push(st4.front());
		st4.pop();
	}
	// now we have reversed the stack
	x->type = EXPR;
	x->expr = new exprNode();
	if (rhs == "E+E")
	{
		x->expr->oper = PLUS_OPER;
		x->expr->child.right = stt.top();
		auto x1 = stt.top();
		stt.pop();
		stt.pop();
		x->expr->child.left = stt.top();
		x->expr->id.line_no = stt.top()->expr->id.line_no;
		auto x2 = stt.top();
		stt.pop();
		if (x1->expr->type == SCALARS && x2->expr->type == SCALARS)
			x->expr->type = SCALARS;
		else if (x1->expr->type == ARRAYS && x2->expr->type == ARRAYS)
			x->expr->type = ARRAYS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(x->expr->id.line_no);
		}
	}
	else if (rhs == "E-E")
	{
		x->expr->oper = MINUS__OPER;
		x->expr->child.right = stt.top();
		auto x1 = stt.top();
		stt.pop();
		stt.pop();
		x->expr->child.left = stt.top();
		x->expr->id.line_no = stt.top()->expr->id.line_no;
		auto x2 = stt.top();
		stt.pop();
		if (x1->expr->type == SCALARS && x2->expr->type == SCALARS)
			x->expr->type = SCALARS;
		else if (x1->expr->type == ARRAYS && x2->expr->type == ARRAYS)
			x->expr->type = ARRAYS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(x->expr->id.line_no);
		}
	}
	else if (rhs == "E*E")
	{
		x->expr->oper = MULT_OPER;
		x->expr->child.right = stt.top();
		auto x1 = stt.top();
		stt.pop();
		stt.pop();
		x->expr->child.left = stt.top();
		x->expr->id.line_no = stt.top()->expr->id.line_no;
		auto x2 = stt.top();
		stt.pop();
		if (x1->expr->type == SCALARS && x2->expr->type == SCALARS)
			x->expr->type = SCALARS;
		else if (x1->expr->type == ARRAYS && x2->expr->type == ARRAYS)
			x->expr->type = SCALARS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(x->expr->id.line_no);
		}
	}
	else if (rhs == "E/E")
	{
		x->expr->oper = DIV_OPER;
		x->expr->child.right = stt.top();
		auto x1 = stt.top();
		stt.pop();
		stt.pop();
		x->expr->child.left = stt.top();
		x->expr->id.line_no = stt.top()->expr->id.line_no;
		auto x2 = stt.top();
		stt.pop();
		if (x1->expr->type == SCALARS && x2->expr->type == SCALARS)
			x->expr->type = SCALARS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(x->expr->id.line_no);
		}
	}
	else if (rhs == "(E)")
	{
		stt.pop();
		x->expr = stt.top()->expr;
		stt.pop();
		stt.pop();
	}
	else if (rhs == "E[.]")
	{
		x->expr->oper = ARRAY_DOT_OPER;
		stt.pop();
		stt.pop();
		stt.pop();
		x->expr->child.left = stt.top();
		x->expr->id.line_no = stt.top()->expr->id.line_no;
		x->expr->child.right = NULL;
		if (is_arrays(stt.top()->expr->id.varName))
			x->expr->type = ARRAYS;
		else if (!lef && stt.top()->expr->type == SCALARS)
			x->expr->type = ARRAYS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(x->expr->id.line_no);
		}
		stt.pop();
	}
	else if (rhs == "E[E]")
	{
		x->expr->oper = ARRAY_ELEM_OPER;
		stt.pop();
		x->expr->array.arrayexpr = stt.top()->expr;
		x->expr->child.right = stt.top();
		auto x1 = stt.top();
		stt.pop();
		stt.pop();
		x->expr->child.left = stt.top();
		auto x2 = stt.top();
		x->expr->id.line_no = stt.top()->expr->id.line_no;
		swap(x1, x2);
		if (is_arrays(x1->expr->id.varName) && x2->expr->type == SCALARS)
			x->expr->type = SCALARS;
		else if (x1->expr->type == ARRAYS && x2->expr->type == SCALARS)
			x->expr->type = SCALARS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(x->expr->id.line_no);
		}
		stt.pop();
	}
	else if (rhs == "E")
	{
		if (is_scalars(stt.top()->term.lexeme))
			x->expr->type = SCALARS;
		else if (is_arrays(stt.top()->term.lexeme))
			x->expr->type = ARRAYDECL;
		else if (stt.top()->term.token_type == NUM)
			x->expr->type = SCALARS;
		else
		{
			x->expr->type = ERRORS;
			line_nums.insert(stt.top()->term.line_no);
		}
		if (stt.top()->term.token_type == NUM)
			x->expr->oper = NUM_OPER;
		x->expr->id.varName = stt.top()->term.lexeme;
		x->expr->id.line_no = stt.top()->term.line_no;
		stt.pop();
	}
	x->type = EXPR;
	st5.push(st5, x);
}

void myStack::reduce(myStack &st2, Token t)
{
	string prec = get_precedence(st2.peek(st2)->term.token_type, t.token_type);
	if (prec == "acc")
	{
		return;
	}
	else if (prec == "err")
		syntax_error();
	else if (prec != ">")
		st2.push(st2, t);
	else if (prec == ">")
	{
		while (get_precedence(st2.peek(st2)->term.token_type, t.token_type) == ">")
		{
			string rhs = "";
			stack<stackNode *> new_stack;
			stackNode *last_popped_terminal = NULL;
			do
			{
				stackNode *y = st2.pop(st2);
				new_stack.push(y);
				last_popped_terminal = (y->type == TERM ? y : last_popped_terminal);
				rhs += ((y->term.token_type != ID && y->type == TERM && y->term.token_type != NUM) ? mp[y->term.token_type] : "E");
			} while (!(st2.top(st2)->type == TERM && last_popped_terminal && get_precedence(st2.peek(st2)->term.token_type, last_popped_terminal->term.token_type) == "<"));
			reverse(all(rhs));
			if (find(all(RHS), rhs) != RHS.end())
				make_reduction(rhs, new_stack, st2);
			else
				syntax_error();
		}
		st2.push(st2, t);
	}
	else
		syntax_error();
}
Token expect(TokenType expected_type)
{
	Token t = lexer.GetToken();
	if (t.token_type != expected_type)
		syntax_error();
	return t;
}

/*----------------------------------------------------------------------*/

void bfs(myStack st, myStack st1)
{
	if (!output.empty())
		return;
	queue<stackNode *> q;
	q.push(st.top(st));
	q.push(st1.top(st1));
	while (!q.empty())
	{
		auto x = q.front();
		q.pop();
		if (x->expr->oper == ARRAY_ELEM_OPER)
			output += "[] ";
		else if (x->expr->oper == PLUS_OPER)
			output += "+ ";
		else if (x->expr->oper == MINUS__OPER)
			output += "- ";
		else if (x->expr->oper == MULT_OPER)
			output += "* ";
		else if (x->expr->oper == DIV_OPER)
			output += "/ ";
		else if (x->expr->oper == ID_OPER)
			output += "ID \"" + x->expr->id.varName + "\" ";
		else if (x->expr->oper == NUM_OPER)
			output += "NUM \"" + x->expr->id.varName + "\" ";
		else if (x->expr->oper == ARRAY_DOT_OPER)
			output += "[.] ";
		if (x->expr->child.left != NULL)
			q.push(x->expr->child.left);
		if (x->expr->child.right != NULL)
			q.push(x->expr->child.right);
	}
}

void parse_expr(myStack &stt)
{
	if (lexer.peek(1).token_type == END_OF_FILE)
		return;
	if (lexer.peek(1).token_type == SEMICOLON || lexer.peek(1).token_type == EQUAL || lexer.peek(1).token_type == RBRAC || lexer.peek(1).token_type == RPAREN)
		return;
	t = lexer.peek(1);
	if (t.token_type == LPAREN)
	{
		t = expect(LPAREN);
		stt.reduce(stt, t);
		parse_expr(stt);
		t = expect(RPAREN);
		stt.reduce(stt, t);
		parse_expr(stt);
	}
	else if (t.token_type == LBRAC)
	{
		t = expect(LBRAC);
		stt.reduce(stt, t);
		parse_expr(stt);
		t = expect(RBRAC);
		stt.reduce(stt, t);
		parse_expr(stt);
	}
	else
	{
		t = lexer.GetToken();
		stt.reduce(stt, t);
		parse_expr(stt);
	}
}

void parse_assignment(myStack &st11, myStack &st22)
{
	lef = 1;
	t = lexer.peek(1);
	if (t.token_type == RBRACE || t.token_type == END_OF_FILE)
		return;
	expect(ID);
	st11.reduce(st11, t);
	Token t1 = lexer.peek(1), t2 = lexer.peek(2);
	if (t1.token_type == LBRAC && t2.token_type == DOT)
	{
		t = expect(LBRAC);
		st11.reduce(st11, t);
		t = expect(DOT);
		st11.reduce(st11, t);
		t = expect(RBRAC);
		st11.reduce(st11, t);
	}
	else if (t1.token_type == LBRAC)
	{
		t = expect(LBRAC);
		st11.reduce(st11, t);
		parse_expr(st11);
		t = expect(RBRAC);
		st11.reduce(st11, t);
	}
	expect(EQUAL);
	st11.last_reduce(st11);
	st11.pop(st11);
	lef = 0;
	if(st11.top(st11)->expr->type == ARRAYDECL)
		line_nums.insert(st11.top(st11)->expr->id.line_no);
	parse_expr(st22);
	st22.last_reduce(st22);
	st22.pop(st22); // removing the $ which is useless
	if (!(st11.top(st11)->expr->type == ARRAYS || st22.top(st22)->expr->type == SCALARS))
		errors.insert(st11.top(st11)->expr->id.line_no);
	expect(SEMICOLON);
}

void parse_statement_list()
{
	if (lexer.peek(1).token_type == OUTPUT)
	{
		expect(OUTPUT);
		expect(ID);
		if (lexer.peek(1).token_type == LBRAC && lexer.peek(2).token_type == DOT)
		{
			expect(LBRAC);
			expect(DOT);
			expect(RBRAC);
		}
		expect(SEMICOLON);
	}
	else if (i == 0 && lexer.peek(1).token_type == ID)
	{
		parse_assignment(st, st1);
		bfs(st, st1);
	}
	else if (i > 0 && lexer.peek(1).token_type == ID)
		parse_assignment(st2, st3);
	else
		syntax_error();
	st2 = myStack();
	st3 = myStack();
	st = myStack();
	st1 = myStack();
	i++;
	t = lexer.peek(1);
	if (t.token_type == RBRACE)
		return;
	if (t.token_type == END_OF_FILE)
		return;
	parse_statement_list();
}

void parse_block()
{
	parse_statement_list();
}

void parse_dec_sect()
{
	t = lexer.peek(1);
	if (t.token_type == SCALAR)
	{
		expect(SCALAR);
		while (lexer.peek(1).token_type != ARRAY && lexer.peek(1).token_type != LBRACE && lexer.peek(1).token_type == ID)
		{
			scalars.push_back(lexer.peek(1).lexeme);
			t = expect(ID);
		}
	}
	t = lexer.peek(1);
	if (t.token_type == ARRAY)
	{
		expect(ARRAY);
		while (lexer.peek(1).token_type != LBRACE && lexer.peek(1).token_type == ID)
		{
			arrays.push_back(lexer.peek(1).lexeme);
			t = expect(ID);
		}
	}
}

void parse_and_generate_AST()
{
	parse_dec_sect();
	expect(LBRACE);
	parse_block();
	expect(RBRACE);
	expect(END_OF_FILE);
	if (ok)
		cout << "= " << output;
}

void parse_and_type_check()
{
	ok = 0;
	parse_and_generate_AST();
	if (!line_nums.empty())
	{
		cout << "Disappointing expression type error :(\n\n";
		for (auto x : line_nums)
			cout << "Line " << x << '\n';
	}
	else if (!errors.empty())
	{
		cout << "The following assignment(s) is/are invalid :(\n\n";
		for (auto x : errors)
			cout << "Line " << x << '\n';
	}
	else
	{
		cout << "Amazing! No type errors here :)\n";
	}
}
{
	ok = 0;
	parse_and_generate_AST();
	if (!line_nums.empty())
	{
		cout << "Disappointing expression type error :(\n\n";
		for (auto x : line_nums)
			cout << "Line " << x << '\n';
	}
	else if (!errors.empty())
	{
		cout << "The following assignment(s) is/are invalid :(\n\n";
		for (auto x : errors)
			cout << "Line " << x << '\n';
	}
	else
	{
		cout << "Amazing! No type errors here :)\n";
	}
}