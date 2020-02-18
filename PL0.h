#include <stdio.h>

#define NRW        25     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       17     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     50     // maximum number of symbols

#define STACKSIZE  1000   // maximum storage

enum symtype
{
	SYM_NULL,//0
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
	SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_OR,//或
	SYM_AND,//与
	SYM_NOT,//非
	SYM_bitAND,//按位与
	SYM_bitOR,//按位或
	SYM_bitXOR,//按位异或
	SYM_MODULO,//取余
	SYM_ELSE,
	SYM_ELIF,
	SYM_EXIT,
	SYM_RETURN,
	SYM_FOR,
	SYM_BREAK,
	SYM_GOTO,
	SYM_SWITCH,
	SYM_CASE,
	SYM_COLON,
	SYM_CONTINUE,
	SYM_DEFAULT,
	SYM_RANDOM,
	SYM_PRINT,
	SYM_CALLSTACK,
	SYM_LARRAY,//数组左括号
	SYM_RARRAY,//数字右括号
	SYM_DPLUS,// ++
	SYM_DMINUS,// --
	SYM_LSHIFT,//left shift
	SYM_RSHIFT,//right shift
	SYM_bLPAREN,//{
	SYM_bRPAREN,//}
	SYM_QMARK   //?
};

enum idtype
{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE,ID_ARRAY
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JZ, JNZ, JGE, JL, JG, JLE, JE, JNE, LEV,ARR_LOD, ARR_STO,CMP,POP,RAN,PTR,CST
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ, OPR_bitAND, OPR_bitOR, OPR_bitXOR, OPR_MOD, OPR_AND, OPR_OR,OPR_NOT
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;
typedef struct backlist1
{
	int position;
	char s[11];
	struct backlist1* next;
}*listforgoto;
typedef struct
{
	int cx;
	char s[11];
}tableforgoto;
typedef struct
{
	int num;
	char s;
}tableforcase;
typedef struct backlist
{
	int position;
	struct backlist* next;
	struct backlist* tail;
}*list,alist;
typedef struct
{
	int  num;
	int  cx;
	int defaultflag;
} switchtable1;
tableforcase casetable[100];
int casecount=0;
switchtable1 switchtable[100];
listforgoto gotohead=  NULL , gototail =  NULL ;
tableforgoto gototable[100];
list truelist[100];
list falselist[100];
list nextlist[100];
list breaklist[100];
list continuelist[100];
list exitlist=NULL;
int truecount = 1, falsecount = 1, nextcount = 1, breakcount = 0, continuecount = 0, gototablecount=  0 ,switchcount = 0;
int  lastsym, currentsym;
int caseflag = 0,caseflag1=0;
int orflag = 0, switchnum[100] = { 0 },count=0;
int casestack[100];
//////////////////////////////////////////////////////////////////////
char* err_msg[] =
		{
			/*  0 */    "",
			/*  1 */    "Found ':=' when expecting '='.",
			/*  2 */    "There must be a number to follow '='.",
			/*  3 */    "There must be an '=' to follow the identifier.",
			/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
			/*  5 */    "Missing ',' or ';'.",
			/*  6 */    "Incorrect procedure name.",
			/*  7 */    "Statement expected.",
			/*  8 */    "Follow the statement is an incorrect symbol.",
			/*  9 */    "'.' expected.",
			/* 10 */    "';' expected.",
			/* 11 */    "Undeclared identifier.",
			/* 12 */    "Illegal assignment.",
			/* 13 */    "':=' expected.",
			/* 14 */    "invalid identifier",
			/* 15 */    "A constant or variable can not be called.",
			/* 16 */    "'LPAREN' expected.",
			/* 17 */    "';' or 'end' expected.",
			/* 18 */    "'do' expected.",
			/* 19 */    "Incorrect symbol.",
			/* 20 */    "Relative operators expected.",
			/* 21 */    "Procedure identifier can not be in an expression.",
			/* 22 */    "Missing ')'.",
			/* 23 */    "The symbol can not be followed by a factor.",
			/* 24 */    "The symbol can not be as the beginning of an expression.",
			/* 25 */    "The number is too great.",
			/* 26 */    "'RPAREN' expected.",
			/* 27 */    "unknown size of array",
			/* 28 */    "overflow",
			/* 29 */    "dim fault",
			/* 30 */    "need (",
			/* 31 */    "invalid break",
			/* 32 */    "There are too many levels.",
			/* 33 */    "while expected",
			/*34*/  "symbol repeat",
			/*35*/  "not found symbol"
		};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;
int elseflag = 0;
int loopflag = 0;
int switchflag = 0;
char line[500];


instruction code[CXMAX];

char* word[NRW + 1] =
		{
				"", /* place holder */
				"begin", "call", "const", "do", "end","if",
				"odd", "procedure", "then", "var", "while", "else", "elif", "exit", "return", "for", "break", "continue", "goto", "switch", "case", "default","random","print"
				,"callstack"
		};

int wsym[NRW + 1] =
		{
				SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
				SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
				SYM_ELSE, SYM_ELIF, SYM_EXIT, SYM_RETURN, SYM_FOR, SYM_BREAK, SYM_CONTINUE, SYM_GOTO, SYM_SWITCH, SYM_CASE, SYM_DEFAULT,
				SYM_RANDOM, SYM_PRINT,SYM_CALLSTACK
		};

int ssym[NSYM + 1] =
		{
				SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
				SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD,
				SYM_SEMICOLON, SYM_NOT, SYM_bitXOR,SYM_MODULO,SYM_COLON,SYM_bLPAREN,SYM_bRPAREN,SYM_QMARK
		};

char csym[NSYM + 1] =
		{
				' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';', '!', '^', '%', ':','{','}','?'
		};

#define MAXINS   23
char* mnemonic[MAXINS] =
		{
				"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JZ", "JNZ", "JGE", "JL", "JG", "JLE", "JE", "JNE", "LEV", "ARR_LOD", "ARR_STO", "CMP", "POP","RAN","PTR","CST"
		};

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	int nt;
	int b;
	int blank;
	struct mask1* next;
} comtab;

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	int   blank;
	int nt;
	int b;
	short level;
	short address;
	struct mask1* next;
} mask;

typedef struct arraymask1
{
	char  name[MAXIDLEN + 1];
	int   kind;
	int dim;//维度
	int nt;
	short level;
	short address;
	struct mask1* next;
} arraymask;
typedef struct mask1
{
	int number;//维度大小
	int size;//不变部分
	struct mask1* next;
} mask2;
FILE* infile;

// EOF PL0.h
