// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
/////////////////////////////////短路计算需要的函数

void backpatch(list *p, int* number)//回填，情况1和2对应网站上的要求，p中position从小到大排列
{
	
	list last = *p, k = *p, DEL, n, x = *p;
	if (*p != NULL)
	{
		while (x->next)
		{
			last = x;
			x = x->next;
		}
		if (x->position == *number - 1 && code[x->position].f == JMP)//情况1
		{
			(*number)--;
			if (x == k)
			{
				last = NULL;
				k = NULL;
				free(x);
			}
			else
			{
				last->next = NULL;
				free(x);
			}
		}
		else last = x;
		list q1 = k, q = last;
		if (q&&code[q->position].f > 6 && code[q->position].f < 14 && q->position == *number - 2 && code[q->position + 1].f == JMP)//情况2
			(*number)--;

		while (q1)
		{
			if (code[q1->position].f > 6 && code[q1->position].f <= 14 && q1->position == *number - 1 && code[q1->position + 1].f == JMP)
			{
				if (code[q1->position].f % 2 == 0) code[q1->position].f--;
				else code[q1->position].f++;
				if (code[q1->position + 1].a == 0)
				{
					if (code[q1->position + 1].l == 0)
					{
						n = falselist[falsecount - 1];
						while (n)
						{
							if (n->position == *number)
							{
								n->position--;
								break;
							}
							n = n->next;
						}
					}
					else if (code[q1->position + 1].l == 1)
					{
						breaklist[breakcount]->tail->position--;;
					}
					else if (code[q1->position + 1].l == 2)
					{
						continuelist[continuecount]->tail->position--;;
					}
					else gototail ->position--;
				}
				else code[q1->position].a = code[q1->position + 1].a;
			}
				else code[q1->position].a = *number;
				DEL = q1;
				q1 = q1->next;
				free(DEL);

		}
		*p = NULL;
	}
}
list merge(list list1, list* list2)//合并
{
	list p = list1;
	if (list1 != NULL)
	{
		while (p->next)
		{
			p = p->next;
		}
		p->next = *list2;
		*list2 = NULL;
		return list1;
	}
	else 
	{ p = *list2;
	 *list2 = NULL;
	 return p; }
}
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch; 
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i,j, k,gotoflag=0;
	char a[MAXIDLEN + 1];
	listforgoto q,last=gotohead,p;
	while (ch == ' '||ch == '\t')
		getch();

	if (ch == '[') { sym = SYM_LARRAY; getch(); }
	else if (ch == ']') { sym = SYM_RARRAY; getch(); }
	else if (ch == '{') { sym = SYM_BEGIN; getch(); }
	else if (ch == '}') { sym = SYM_END; getch(); }
	else if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch)||ch=='_');
		while (ch == ' ' || ch == '\t')
			getch();
		if (ch == ':'&&caseflag==0)
		{
			gotoflag = 1;
			getch();
		}
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;// num of resevred words
		while (strcmp(id, word[i--]));// try to match a resevred word
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else if (gotoflag)
		{
			for (j = 0; j < gototablecount; j++)
			{
				if (!strcmp(id, gototable[i].s))
				{
					error(34);
					break;
				}
			}
			strcpy(gototable[gototablecount].s, id);
		gototable[gototablecount].cx = cx;
		gototablecount++;
		////////////////////////////////////////////////////////////////////////
		
		q = gotohead;
		last = gotohead;
		while (q)
		{
			if (!strcmp(q->s, id))
			{
				code[q->position].a = cx;
				if (gotohead == gototail)
				{
					free(gotohead);
					gotohead = gototail = NULL;
					break;
				}
				else if (q == gotohead )
				{
					gotohead  = gotohead ->next;
					last = gotohead ;
					free(q);
					q = gotohead ;
				}
				else {
					free(last->next);
					last->next = q->next;
					q = last->next;
				}
			}
			else{
				last = q;
				q = q->next;
			}
		}getsym();
		}
		else
			sym = SYM_IDENTIFIER;// symbol is an identifier
		if (sym == SYM_DEFAULT) { cc-=2; getch(); }
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';// convert to digit
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great. 
	}
	else if (ch == ':')
	{
		sym=SYM_COLON;
		getch();
	}
	else if (ch == '|')
	{
		getch();
		if (ch == '|')
		{
			sym = SYM_OR;
			getch();
		}
		else sym = SYM_bitOR;
	}
	else if (ch == '&')
	{
		getch();
		if (ch == '&')
		{
			sym = SYM_AND;
			getch();
		}
		else sym = SYM_bitAND;
	}
	else if (ch == '/')//////////////////////////////////////////////////////////////////////
	{
		getch();
		if (ch == '/')//行注释
		{
			cc = ll;
			getch();
			getsym();		
		}
		else if (ch == '*')//块注释
		{
			while (!(ch == '*'&&line[cc+1] == '/'))
			{
					getch(); 
			}
			getch();
			getch();
			getsym();
		}
		else sym = SYM_SLASH;
	}
	// 11-20
	else if(ch == '+')
    {
        getch();
        if(ch == '+')
        {
            sym = SYM_DPLUS;//构成++号
            getch();
        }
        else
        {
            sym = SYM_PLUS;
        }
    }
    else if(ch == '-')
    {
        getch();
        if(ch == '-')
        {
            sym = SYM_DMINUS;//构成--号
            getch();
        }
        else
        {
            sym = SYM_MINUS;
        }
    }

	else if (ch == '=')
	{
		getch();
		if (ch != '=')
		{
			sym = SYM_BECOMES; // :=
		}
		else
		{
			sym = SYM_EQU;
			getch();
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else if(ch == '>')
		{
			sym = SYM_RSHIFT;	//>>
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else if(ch == '<')
		{
			sym = SYM_LSHIFT;	//<<
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();	//skip
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index
int var_flag = 0;
int var_num  = 0;
int var_table[100] = {0};

int var_arr[100][100];
int var_arr_flag = 0;
int arr_index = 0;
int arr_i[100];
arraymask* arr_table[100];
// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;
	arraymask* mk1;
	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	table[tx].nt = 1;
	table[tx-1].nt = tx;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
	    table[tx].next = NULL;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		mk->next = NULL;
		break;
	case ID_ARRAY:
		mk1 = (arraymask*)&table[tx];
		mk1->level = level;
		mk1->address = dx++;
		mk1->next = NULL;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = cx;
		mk->next = NULL;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
mask* var_mk[100];

////var get number
void enter_become(int kind)
{
	mask* mk;
	arraymask* mk1;
	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_VARIABLE:
		var_flag = 1;
		var_table[var_num] = num;
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		mk->next = NULL;
		var_mk[var_num++] = mk;

		break;
	default: error(19);
	} // switch
}




// locates identifier in symbol table.
int position(char* id)
{
	int i = tx;	
	mask* mk;
	mk = (mask*) &table[tx];
	if(table[tx+1].kind == -1)
		i  = 1;
	else
		while(mk->level != 0){
			mk = (mask*)&table[--i];
		}
	int k = i; 
	while(1){
		if(strcmp(table[i].name,id) == 0)
			return i;
		else	
			i  = table[i].nt;
		if (i == k)
			return 0;
	}
} // position


//////////////////////////////////////////////////////////////////////
void constdeclaration()//常量赋值
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_BECOMES)
		{
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration


//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	int size=1,q;
	int arraydim = 0;
	mask2 *temp=NULL,*temp1=NULL;
	arraymask* mk1=NULL;
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_LARRAY)
				{
					enter(ID_ARRAY);
					while (sym == SYM_LARRAY)//数组定义,产生一个链表存储维度和每个维度的大小，标识符只适用于const型
					{
						getsym();
						if (sym == SYM_NUMBER || sym == SYM_IDENTIFIER)
						{
							if (sym == SYM_IDENTIFIER)
							{
								if ((q = position(id)) == 0)
								{
									error(11); // Undeclared identifier.
								}
								else
								{
									switch (table[q].kind)
									{
									case ID_CONSTANT:
										num = table[q].value;
										break;
									default:error(19);
									}
								}
							}// num  
							temp = (mask2*)malloc(sizeof(mask1));
							temp->number = num;
							temp->next  = NULL;
							mk1 = (arraymask*)&table[tx];
							if (mk1->next == NULL)
								mk1->next = temp;
							else temp1->next = temp;
							temp1 = temp;
							arraydim++;
							size *= num;
							getsym();
							if (sym == SYM_RARRAY)
								getsym();
							else error(26);
						}
						else error(27);
					}
					mk1->dim = arraydim;
					int dimsize=size;
					for (temp1 = mk1->next; temp1 != NULL; temp1 = temp1->next)
					{
						dimsize /= temp1->number;
						temp1->size = dimsize;
					}
					dx += size - 1;
					if(sym == SYM_BECOMES)// =
						{	
							getsym();// {
							getsym();// number
							var_arr_flag = 1;
							arr_i[arr_index] = 0;
						
							arr_table[arr_index] = mk1;

							while(sym == SYM_NUMBER)
							{

								var_arr[arr_index][arr_i[arr_index]++] = num;
																
								getsym();// comma
								getsym();// number
							}

							arr_index++;
						}

					
				}
		else if (sym == SYM_BECOMES)
		{
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter_become(ID_VARIABLE);
			}	
			getsym();
		}
		else
		{
			enter(ID_VARIABLE);

		} 	
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i, position1 = 0, dim = 0 , q, flag = 0;
	int address = 0, sizeofarray = 0;
	symset set;
	arraymask* mk1;
	mask2* temp;
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.
	if (inset(sym, facbegsys))
	{
	if(sym == SYM_RANDOM){
			getsym();
			getsym();
			if(sym == SYM_RPAREN)
				gen(RAN,0,-1);
			else 
			    gen(RAN,0,num);
				getsym();
			getsym();
	}
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
					mask* mk;
				case ID_CONSTANT:
					if (!caseflag1)		gen(LIT, 0, table[i].value);
					else {
						casetable[casecount].s = 'a';
						casetable[casecount++].num = table[i].value;
					}
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;

				case ID_ARRAY:
					mk1 = (arraymask*)&table[i];
					flag = 1;
					getsym();
					temp = mk1->next;
					while (sym == SYM_LARRAY)//数组定义
					{
						getsym();
						if (sym == SYM_NUMBER || sym == SYM_IDENTIFIER)
						{
							if (sym == SYM_IDENTIFIER)
							{
								if ((q = position(id)) == 0)
								{
								error(11); // Undeclared identifier.
								}
								else
								{
								switch (table[q].kind)
								{
								case ID_CONSTANT:
									address += (table[q].value*temp->size);
									break;
								case ID_VARIABLE:
									mk = (mask*)&table[q];
									gen(LIT, 0, temp->size);
									gen(LOD, level - mk->level, mk->address);
									gen(OPR, 0, OPR_MUL);
									if (sizeofarray!=0)
										gen(OPR, 0, OPR_ADD);
									sizeofarray++;
									break;
								default:error(19);
								}
								}
							}
							else {
								address += (num*temp->size);
							}
							dim++;
							getsym();
							if (sym == SYM_RARRAY)
								getsym();
							else error(26);
					}
						else error(27);
						temp = temp->next;
					}
					gen(LIT, 0, address);
					if (sizeofarray != 0)
						gen(OPR, 0, OPR_ADD);
					if (dim !=mk1->dim) error(29);
					else 
						gen(ARR_LOD, level - mk1->level, mk1->address);
					break;
				case ID_PROCEDURE:
				int k,count;
				k = i;
				count = 0;
				getsym();
				if(sym == SYM_LPAREN){
					getsym();
				}
				else{
					error(0);
				}
				while(sym != SYM_RPAREN)
				{
				count++;
				expression(fsys);					
				if(sym == SYM_COMMA)
				{
					getsym();
				}
				}
				level++;
				mk = (mask*)&table[k];
				gen(LIT,0,count);
				gen(CAL,1, mk->address);	
				level--;
				break; 
					
				}// switch 
			} 
			if(!flag) getsym();
			//getsym();
			// 11-20
			if(sym == SYM_DPLUS)
            {
            	mask* mk;
            	mk = (mask*) &table[i];
                gen(LIT,0,1);//将值为入栈
                gen(OPR,0,2);//+1,栈顶加次栈顶
                gen(STO,level-mk->level,mk->address);//出栈取值到内存
                gen(LOD,level-mk->level,mk->address);//取值到栈顶
                gen(LIT,0,1);
                gen(OPR,0,3);//栈顶值减
                getsym();
        } 
            else if(sym == SYM_DMINUS)
            {
            	mask* mk;
				mk = (mask*) &table[i];
                gen(LIT,level-mk->level,1);//将值为入栈
                gen(OPR,0,3);//-1,栈顶加次栈顶
                gen(STO,level-mk->level,mk->address);//出栈取值到内存
                gen(LOD,level-mk->level,mk->address);//取值到栈顶
                gen(LIT,0,1);
                gen(OPR,0,2);//栈顶值加
                getsym();                       
            }
            else if(sym == SYM_LSHIFT)
            {
            	mask* mk;
            	mk = (mask*) &table[i];
            	getsym();
            	if(sym == SYM_NUMBER) 
            	{
            		for(int j = 0; j < num;j++)
            		{
            			gen(LIT,0,2);
            			gen(OPR,0,4);
            			gen(STO,level-mk->level,mk->address);
            			gen(LOD,level-mk->level,mk->address);

            		}
            	}
            	getsym();
            }
            else if(sym == SYM_RSHIFT)
            {
            	mask* mk;
            	mk = (mask*) &table[i];
            	getsym();
            	if(sym == SYM_NUMBER) 
            	{
            		for(int j = 0; j < num;j++)
            		{
            			gen(LIT,0,2);
            			gen(OPR,0,5);
            			gen(STO,level-mk->level,mk->address);
            			gen(LOD,level-mk->level,mk->address);

            		}
            	}
            	getsym();
            }
            // 12-10
            else if(sym == SYM_BECOMES)
            {	
            	mask* mk;
				mk = (mask*) &table[i];
            	gen(STO,level-mk->level,mk->address);
            	getsym();
            	if(sym == SYM_NUMBER){
            		gen(LIT,0,num);
            		gen(STO,level-mk->level,mk->address);
            		gen(LOD,level-mk->level,mk->address);
            		getsym();
            	}
            	else{
            		factor(fsys);
            		gen(STO,level-mk->level,mk->address);
            		gen(LOD,level-mk->level,mk->address);
            	}
            }
			else if (sym == SYM_GTR&&!orflag)		// ? : function 
			{
				mask* mk;
				getsym();
				i = position(id);
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);// second var
				getsym();//?
				gen(JLE, 0, cx + 4);// POP + First LOD
				gen(POP, 0, 2);
				getsym();// first
				i = position(id);
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);
				gen(JMP, 0, cx + 2);
				getsym();//
				i = position(id);
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);
				getsym();// ;

			}
			else if (sym == SYM_LES&&!orflag)		// ? : function
			{
				mask* mk;
				getsym();
				i = position(id);
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);// second var
				getsym();//?
				gen(JGE, 0, cx + 4);// POP + First LOD
				gen(POP, 0, 2);
				getsym();
				i = position(id);
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);
				gen(JMP, 0, cx + 2);
				getsym();//
				i = position(id);
				mk = (mask*)&table[i];
				gen(LOD, level - mk->level, mk->address);
				getsym();// ;

			}

		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			if (!caseflag1)		gen(LIT, 0, num);
			else {
				casetable[casecount].s = 'a';
				casetable[casecount++].num = num;
			}
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
		{  
			 getsym();
			 factor(fsys);
			 gen(OPR, 0, OPR_NEG);
		}

		 // 11-20
        else if(sym == SYM_DPLUS)
        {
            getsym();
            mask* mk;
            
            if(sym == SYM_IDENTIFIER)
            {
                getsym();
                if (! (i = position(id)))
                {
                error(11); // Undeclared identifier.
                }
                else
                {
                    if(table[i].kind == ID_VARIABLE)
                    {
                        mk = (mask*) &table[i];
                        gen(LOD,level-mk->level,mk->address);//找到变量地址，将其值入栈
                        gen(LIT,0,1);//将常数1取到栈顶
                        gen(OPR,0,2);     //执行加操作
                        gen(STO,level-mk->level,mk->address);//出栈取值到内存
                        gen(LOD,level-mk->level,mk->address);//取值到栈顶
                    }
                }
            }   
        }
        else if(sym == SYM_DMINUS)
        {
            getsym();
            mask* mk;
            if(sym == SYM_IDENTIFIER)
            {
                getsym();
                
                if (! (i = position(id)))
                {
                error(11); // Undeclared identifier.
                }
                else
                {
                    if(table[i].kind == ID_VARIABLE)
                    {
                        mk = (mask*) &table[i];
                        gen(LOD,level-mk->level,mk->address);//找到变量地址，将其值入栈
                        gen(LIT,0,1);//将常数1取到栈顶
                        gen(OPR,0,3);     //执行减操作
                        gen(STO,level-mk->level,mk->address);//出栈取值到内存
                        gen(LOD,level-mk->level,mk->address);//取值到栈顶
                    }
                }
            }   
        }
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_MODULO,SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH||sym==SYM_MODULO)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			if (!caseflag1)	gen(OPR, 0, OPR_MUL);
			else {
				casetable[casecount++].s = '*';
			}
		}
		else if (mulop == SYM_SLASH)
		{
			if (!caseflag1)		gen(OPR, 0, OPR_DIV);
			else {
				casetable[casecount++].s = '/';
			}
		}
		else if (mulop == SYM_MODULO)
		{
			if (!caseflag1)	 gen(OPR, 0, OPR_MOD);
			else {
				casetable[casecount++].s = '%';
			}
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression1(symset fsys)
{
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_bitAND,SYM_bitOR,SYM_bitXOR,SYM_PLUS, SYM_MINUS, SYM_NULL));//////////////////////////////

	term(set);
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
		if(!caseflag1) gen(OPR, 0, OPR_ADD);
		else {
				casetable[casecount++].s = '+';
			}
		}
		else
		{
			if (!caseflag1)	gen(OPR, 0, OPR_MIN);
			else
			{
				casetable[casecount++].s = '-';
			}
		}
	} // while
	destroyset(set);
} // expression1
void bit_and(symset fsys)//按位与
{
	symset set;

	set = uniteset(fsys, createset(SYM_bitAND, SYM_NULL));//////////////////////////////

	expression1(set);
	while (sym == SYM_bitAND)
	{
		getsym();
		expression1(set);
		if(!caseflag1)gen(OPR, 0, OPR_bitAND);
		else { 
			casetable[casecount++].s = '&';
			
		}
	}
	destroyset(set);
}
void bitxor(symset fsys)//按位异或
{
	symset set;

	set = uniteset(fsys, createset(SYM_bitXOR, SYM_NULL));//////////////////////////////

	bit_and(set);
	while (sym == SYM_bitXOR)
	{
		getsym();
		bit_and(set);
		if(!caseflag1) gen(OPR, 0, OPR_bitXOR);
		else {
			casetable[casecount++].s = '^';
		}
	}
	destroyset(set);
}
void expression(symset fsys)//按位或
{
	symset set;

	set = uniteset(fsys, createset(SYM_bitOR, SYM_NULL));//////////////////////////////

	bitxor(set);
	while (sym == SYM_bitOR)
	{
		getsym();
		bitxor(set);
		if(!caseflag1)gen(OPR, 0, OPR_bitOR);
		else {
			casetable[casecount++].s = '|';
		}
	}
	destroyset(set);
}
//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set,set1;
	void or_(symset fsys);
	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else if (sym == SYM_LPAREN)
	{
		getsym();
		set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
		or_(set);
		destroyset(set);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(22); // Missing ')'.
		}
	}
	else if (sym != SYM_RPAREN)
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		 if (! inset(sym, relset))
		{
			set1 = createset(SYM_RPAREN,SYM_OR, SYM_AND, SYM_NULL);
			if (inset(sym, set1))
			{
				truelist[truecount] = (list)malloc(sizeof(alist));
				falselist[falsecount] = (list)malloc(sizeof(alist));
				truelist[truecount]->position = cx;
				truelist[truecount]->next = NULL;
				gen(JNZ, 0, 0);
				falselist[falsecount]->position = cx;
				falselist[falsecount]->next = NULL;
				gen(JMP, 0, 0);
				truecount++;
				falsecount++;
			}
			else error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			truelist[truecount] = (list)malloc(sizeof(alist));
			falselist[falsecount] = (list)malloc(sizeof(alist));
			truelist[truecount]->position = cx;
			truelist[truecount]->next = NULL;
			switch (relop)
			{
			case SYM_EQU:
				gen(JE, 0, 0);			
				gen(JMP, 0, 0);
				break;
			case SYM_NEQ:
				gen(JNE, 0, 0);
				gen(JMP, 0, 0);
				break;
			case SYM_LES:	
				gen(JL, 0, 0);
				gen(JMP, 0, 0);
				break;
			case SYM_GEQ:
				gen(JGE, 0, 0);
				gen(JMP, 0, 0);
				break;
			case SYM_GTR:
				gen(JG, 0, 0);
				gen(JMP, 0, 0);				
				break;
			case SYM_LEQ:
				gen(JLE, 0, 0);
				gen(JMP, 0, 0);
				break;
			} // switch
			falselist[falsecount]->position = cx - 1;
			falselist[falsecount]->next = NULL;
			truecount++;
			falsecount++;
		} // else
	} // else

} // condition
void not_(symset fsys)//逻辑非
{
	symset set;
	list q;
	set = uniteset(fsys, createset(SYM_NOT, SYM_NULL));//////////////////////////////
	if (sym == SYM_NOT)
		{
			getsym();
			not_(set);
			q=truelist[truecount-1];
			truelist[truecount-1] = falselist[falsecount-1];
			falselist[falsecount-1] = q;		
		}
	else condition(set);
	destroyset(set);
}
void and_(symset fsys)//逻辑与
{
	symset set;

	set = uniteset(fsys, createset(SYM_AND, SYM_NULL));//////////////////////////////

	not_(set);
	while (sym == SYM_AND)
	{
		getsym();
		backpatch(&truelist[--truecount], &cx);
		not_(set);
		falselist[falsecount - 2]=merge(falselist[falsecount - 2], &falselist[falsecount - 1]);
		falsecount--;
	}
	destroyset(set);
}
void or_(symset fsys)//逻辑或
{
	symset set;

	set = uniteset(fsys, createset(SYM_OR, SYM_NULL));//////////////////////////////
	orflag++;
	and_(set);
	while (sym == SYM_OR)
	{
			getsym();
			backpatch(&falselist[--falsecount], &cx);
			and_(set);
			truelist[truecount - 2] = merge(truelist[truecount - 2], &truelist[truecount - 1]);
			truecount--;
	}
	orflag--;
	destroyset(set);
}

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)//负责语句部分
{
	int i, cx1, cx2,cx3, flag = 0, q1, position1=0, dim = 0,j,lastcx,arrayflag=0;
	int address = 0, sizeofarray = 0;
	symset set1, set;
	list q=NULL,p=NULL,p1=NULL,p2=NULL;
	mask2* temp=NULL;
	mask* mk = NULL;
	arraymask* mk1=NULL;
	instruction* code1=NULL;
	listforgoto goto1 = NULL, goto2 = NULL;
	int z = 0, change[100];
	if(sym==SYM_CALLSTACK){
		gen(CST,0,0);
		getsym();
	}
	if(sym==SYM_RANDOM){
			getsym();
			getsym();
			if(sym == SYM_RPAREN)
				gen(RAN,0,-1);
			else 
			    gen(RAN,0,num);
				getsym();
			getsym();
		}
		if(sym ==SYM_PRINT){
			getsym();
			getsym();
			int count=0;			
			while(1){				
			if(sym == SYM_RPAREN){
				if(!count)
					gen(PTR,1,0);				
				break;
			}
			else {
			    expression(fsys);
				gen(PTR,0,0);
				count++;
				if(sym == SYM_COMMA)
				getsym();
				else
				break;		
				}
			}
			getsym();
		}
	if(var_flag)
	{	int var_i;
		for(var_i=0;var_i<var_num;var_i++)
		{
			mask* var_t;
			var_t = var_mk[var_i];
			gen(LIT,0,var_table[var_i]);
			gen(STO, level - var_t->level, var_t->address);
		}
		var_flag = 0;
	}
	if(var_arr_flag)
	{
		int arr_j,arr_k;
		for(arr_j=0;arr_j<arr_index;arr_j++)// ID
		{

			arraymask* arr_t;
			arr_t = arr_table[arr_j];
			temp = arr_t->next;
			for(arr_k=0;arr_k<temp->number;arr_k++)// offset
			{

				gen(LIT,0,var_arr[arr_j][arr_k]);//value 
				gen(LIT,0,arr_k*temp->size);//address
				gen(ARR_STO, level - arr_t->level, arr_t->address);
				
			}
		}
	 var_arr_flag = 0;
	}
	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		mask* mk = NULL;
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE && table[i].kind != ID_ARRAY&&table[i].kind == ID_PROCEDURE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		if (table[i].kind == ID_PROCEDURE)
			{	
				int k,count;
				k = i;
				count = 0 ;				
				getsym();
				if(sym == SYM_LPAREN){
					getsym();
				}
				else{
					error(0);
				}
				while(sym != SYM_RPAREN)
				{
					count ++;
					if(table[k+count].kind == ID_VARIABLE)
					expression(fsys);
//					else if (table[k+count].level == 2){
//						i = position(id);
//						mask *mk1;
//						mk1 = (mask*)&table[i];
//						gen(LIT,0,mk1->address);	
//					}
					else if (table[k+count].kind == ID_PROCEDURE){
						
					}
					else error(0);																								
				getsym();
				if(sym == SYM_COMMA)
				{
					getsym();
				}
			}//void()
				level++;
				mk = (mask*)&table[k];
				gen(LIT,0,count);
				gen(CAL,1, mk->address);
				level--;		
			}
		if (table[i].kind == ID_ARRAY)
		{
			mk1 = (arraymask*)&table[i];
			arrayflag = 1;
			temp = mk1->next;
			flag = 1;
			getsym();
			cx1 = cx;
			while (sym == SYM_LARRAY)//数组定义
			{
				getsym();
				if (sym == SYM_NUMBER || sym == SYM_IDENTIFIER)
				{
					if (sym == SYM_IDENTIFIER)
					{
						if ((q1 = position(id)) == 0)
						{
							error(11); // Undeclared identifier.
						}
						else
						{
							switch (table[q1].kind)
							{
							case ID_CONSTANT:
								address += (table[q1].value*temp->size);
								break;
							case ID_VARIABLE:
								mk = (mask*)&table[q1] ;
								gen(LIT, 0, temp->size);
								gen(LOD, level - mk->level, mk->address);
								gen(OPR, 0, OPR_MUL);// stack[top] = var * size
								if (sizeofarray != 0)
									gen(OPR, 0, OPR_ADD);
								sizeofarray++;
								break;
							default:error(19);
							}
						}
					}
					else {// number
						address += (num*temp->size);
						 }
					dim++;			
					getsym();
					if (sym == SYM_RARRAY)
						getsym();
					else error(26);
				}
				else error(27);
				temp = temp->next;
			}
			gen(LIT, 0, address);
			if (sizeofarray != 0)
				gen(OPR, 0, OPR_ADD);
			cx2 = cx;
			code1 = (instruction*)malloc((cx2 - cx1)*sizeof(instruction));
			for (j = 0;  j< cx2 - cx1; j++)
			{
				code1[j] = code[cx1 + j];
			}
			cx = cx1;
		}
		else mk = (mask*)&table[i];
		if (!flag) { getsym(); }
		if (sym == SYM_BECOMES)
		{
			getsym();
			expression(fsys);
			//  up???
		}

		// 11-20
		else if(sym == SYM_DPLUS) // 后++运算 
		{
		    getsym();
		    mk = (mask*) &table[i];
		    gen(LOD,level-mk->level,mk->address);//找到变量地址，将其值入栈
		    gen(LIT,0,1);//将常数1取到栈顶
		    if(i!=0)
		    {
		        gen(OPR,0,2);     //执行加操作
		        //gen(STO,level-mk->level,mk->address);
		    }
		}
		else if(sym == SYM_DMINUS) // 后--运算 
		{
		    getsym();
		    mk = (mask*) &table[i];
		    gen(LOD,level-mk->level,mk->address);//找到变量地址，将其值入栈
		    gen(LIT,0,1);//将常数1取到栈顶
		    if(i!=0)
		    {
		        gen(OPR,0,3);     //执行减操作
		        //gen(STO,level-mk->level,mk->address);
		    }
		}

		//12-9
		else if(sym == SYM_LSHIFT) // 左移
		{
			//printf("%d\n", sym);
			getsym();
			mk = (mask*) &table[i];
			if(sym == SYM_NUMBER)
			{
				for(j = 0;j < num;j++)
				{
					gen(LOD,level-mk->level,mk->address);//变量入栈
					gen(LIT,0,2);//2入栈
					gen(OPR,0,4);//Mul
					if(j == num - 1)
						continue;
					gen(STO, level - mk->level, mk->address);

				}
				getsym();
			}
			else
			{
				error(12);	
			}
		}
		else if(sym == SYM_RSHIFT) // 右移
		{
			getsym();
			if(sym == SYM_NUMBER)
			{
				for(j = 0;j < num;j++)
				{
					gen(LOD,level-mk->level,mk->address);//变量入栈
					gen(LIT,0,2);//2入栈
					gen(OPR,0,5);//Div
					if(j == num - 1)
						continue;
					gen(STO, level - mk->level, mk->address);

				}
				getsym();
			}
			else
			{
				error(12);
			}
		}		
		else 
		{
			error(13); // ':=' expected.
		}

		if (arrayflag == 0)
		{
			gen(STO, level - mk->level, mk->address);
		}
		else 
		{
			for (j = 0; j < cx2 - cx1; j++)
			{
				code[cx++] = code1[j];
			}
			free(code1);
			gen(ARR_STO, level - mk1->level, mk1->address);
		}
	}	
		// 11-20	
	else if(sym == SYM_DPLUS) // 前++运算 
    {
        getsym();
        if(sym == SYM_IDENTIFIER)
        {
            if (! (i = position(id)))
            {
            error(11); // Undeclared identifier.
            }
            else
            {
                if(table[i].kind != ID_VARIABLE) 
                {
                    error(12);
                    i = 0;
                }
                else    //++后跟的是变量
                {
                    getsym();
                    mask* mk;
                    mk = (mask*) &table[i];
                    gen(LOD,level-mk->level,mk->address);//找到变量地址，将其值入栈
                    gen(LIT,0,1);//将常数1取到栈顶
                    if(i != 0)
                    {
                        gen(OPR,0,2);     //执行加操作
                        gen(STO,level-mk->level,mk->address);
                    }
                }
            }
        }
        else
        {
            error(19);
        }
    }
    else if(sym == SYM_DMINUS) // 前--运算 
    {
        getsym();
        if(sym == SYM_IDENTIFIER)
        {
            if (! (i = position(id)))
            {
            error(11); // Undeclared identifier.
            }
            else
            {
                if(table[i].kind != ID_VARIABLE) 
                {
                    error(12);
                    i=0;
                }
                else  //--后跟的是变量
                {
                    getsym();
                    mask* mk;
                    mk = (mask*) &table[i];
                    gen(LOD,level-mk->level,mk->address);//找到变量地址，将其值入栈
                    gen(LIT,0,1);//将常数1取到栈顶
                    if(i!=0)
                    {
                        gen(OPR,0,3);     //执行减操作
                        gen(STO,level-mk->level,mk->address);
                    }
                }

            }
        }
        else
        {
            error(19);
        }
    }

	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_ELSE,SYM_RPAREN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		if (sym == SYM_LPAREN)
		{
			getsym();
			or_(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(26);
			}
		}
		else error(16);
		backpatch(&truelist[--truecount], &cx);
		statement(set);
		nextcount++;
		if (sym!=SYM_RPAREN) 
		{
			elseflag++;
			lastsym = sym;
			getsym();
		}
		while (sym == SYM_ELIF)//ELIF语句
		{
			elseflag--;
			getsym();
			nextlist[nextcount] = (list)malloc(sizeof(alist));
			nextlist[nextcount]->position = cx;
			nextlist[nextcount]->next = NULL;
			gen(JMP, 0, 0);
			if (nextlist[nextcount - 1] == NULL)
			{
				nextlist[nextcount - 1] = (list)malloc(sizeof(alist));
				nextlist[nextcount - 1] = merge(NULL, &nextlist[nextcount]);
			}
			else nextlist[nextcount - 1] = merge(nextlist[nextcount - 1], &nextlist[nextcount]);
			backpatch(&falselist[--falsecount], &cx);
			if (sym == SYM_LPAREN)
			{
				getsym();
				or_(set);
				if (sym == SYM_RPAREN)
				{
					getsym();
				}
				else
				{
					error(26);
				}
			}
			else error(16);
			backpatch(&truelist[--truecount], &cx);
			statement(set);
			//backpatch(&falselist[--falsecount], &cx);
			if (nextlist[nextcount - 1] == NULL)
			{
				nextlist[nextcount - 1] = (list)malloc(sizeof(alist));
				nextlist[nextcount - 1] = merge(NULL, &nextlist[nextcount]);
			}
			else nextlist[nextcount - 1] = merge(nextlist[nextcount - 1], &nextlist[nextcount]);
			if (sym != SYM_RPAREN)
			{
				elseflag++;
				lastsym = sym;
				getsym();
			}
		}
		if (sym == SYM_ELSE)//ELSE语句
		{
			elseflag--;
			getsym();
			nextlist[nextcount] = (list)malloc(sizeof(alist));
			nextlist[nextcount]->position = cx;
			nextlist[nextcount]->next = NULL;
			gen(JMP, 0, 0);
			if (nextlist[nextcount-1] == NULL)
			{
				nextlist[nextcount-1] = (list)malloc(sizeof(alist));
				nextlist[nextcount - 1] = merge(NULL, &nextlist[nextcount]);
			}
			else nextlist[nextcount - 1] = merge(nextlist[nextcount-1], &nextlist[nextcount]);
			backpatch(&falselist[--falsecount], &cx);
			statement(set);
			nextlist[nextcount-1] = merge(nextlist[nextcount-1], &nextlist[nextcount]);
		}
		else  
		{
		if (nextlist[nextcount-1] == NULL)
		{
			nextlist[nextcount-1] = (list)malloc(sizeof(alist));
			nextlist[nextcount-1] = merge(NULL, &falselist[--falsecount]);
		}
		else nextlist[nextcount-1] = merge(nextlist[nextcount-1], &falselist[falsecount]);
		}
		backpatch(&nextlist[--nextcount], &cx);
		destroyset(set1);
		destroyset(set);
		if (elseflag)
		{
			currentsym = sym;
			sym = lastsym;
		}
		///////////////////////////////////////
	}
	else if (sym == SYM_BREAK)//break
	{
		if (!loopflag&&!switchflag) error(31);
		else if (breaklist[breakcount] == NULL)
		{
			breaklist[breakcount] = (list)malloc(sizeof(alist));
			breaklist[breakcount]->position = cx;
			breaklist[breakcount]->tail = breaklist[breakcount];
			breaklist[breakcount]->next = NULL;
			gen(JMP,1, 0);
		}
		else {
			q = (list)malloc(sizeof(alist));
			q->position = cx;
			q->next = NULL;
			q->tail = NULL;
			breaklist[breakcount]->tail->next=q;
			breaklist[breakcount]->tail = q;
			gen(JMP,1, 0);
		}
		getsym();
	}
	else if (sym == SYM_CONTINUE)//continue
	{
		if (!loopflag) error(31);
		else if (continuelist[continuecount] == NULL)
		{
			continuelist[continuecount] = (list)malloc(sizeof(alist));
			continuelist[continuecount]->position = cx;
			continuelist[continuecount]->tail = continuelist[continuecount];
			continuelist[continuecount]->next = NULL;
			gen(JMP, 2, 0);
		}
		else {
			q = (list)malloc(sizeof(alist));
			q->position = cx;
			q->next = NULL;
			q->tail = NULL;
			continuelist[continuecount]->tail->next=q;
			continuelist[continuecount]->tail = q;
			gen(JMP, 2, 0);
		}
		getsym();
	}
	else if (sym == SYM_GOTO)//goto语句
	{
		getsym();
		if (sym == SYM_IDENTIFIER)
		{
			for (i = 0; i < gototablecount ; i++)
			{
				if (!strcmp(id, gototable[i].s))
				{
					gen(JMP, 3, gototable[i].cx);
					break;
				}
			}
		}
		else error(19);
		if (i == gototablecount ){
			goto1 = (listforgoto)malloc(sizeof(backlist1));
			goto1->position = cx;
			goto1->next = NULL;
			strcpy(goto1->s, id);
			if (gototail  != NULL)
				gototail ->next = goto1;
				gototail  = goto1;
			gen(JMP, 3, 0);
			if (gotohead  == NULL)
			{
				gotohead  = goto1;
			}
		}
		getsym();
	}
	else if (sym == SYM_CASE)
	{
		if (switchflag != 0)
		{
			set = uniteset(fsys, createset(SYM_COLON, SYM_NULL));
			while (sym == SYM_CASE)
			{
				caseflag++;
				caseflag1++;
				getsym();
				casecount = 0;
				expression(set);
				switchnum[count]++;
				caseflag1--;
				caseflag--;
				z = 0;
				for (j = 0; j < casecount; j++)
				{
					switch (casetable[j].s)
					{
					case '+':
						casestack[z - 2] = casestack[z - 2] + casestack[z - 1];
						z--;
						break;
					case '-':
						casestack[z - 2] = casestack[z - 2] - casestack[z - 1];
						z--;
						break;
					case '*':
						casestack[z - 2] = casestack[z - 2] * casestack[z - 1];
						z--;
						break;
					case '/':
						casestack[z - 2] = casestack[z - 2] / casestack[z - 1];
						z--;
						break;
					case '%':
						casestack[z - 2] = casestack[z - 2] % casestack[z - 1];
						z--;
						break;
					case '&':
						casestack[z - 2] = casestack[z - 2] & casestack[z - 1];
						z--;
						break;
					case '^':
						casestack[z - 2] = casestack[z - 2] ^ casestack[z - 1];
						z--;
						break;
					case '|':
						casestack[z - 2] = casestack[z - 2] | casestack[z - 1];
						z--;
						break;
					default:casestack[z++] = casetable[j].num;
					}
				}
				z--;
				switchtable[switchcount].defaultflag = 0;
				switchtable[switchcount].num = casestack[0];
				switchtable[switchcount++].cx = cx;
				if (sym == SYM_COLON)
				{
					getsym();
					while (sym != SYM_CASE&&sym != SYM_DEFAULT&&sym != SYM_END)
					{
						statement(fsys);
						getsym();
					}

				}
				else error(19);
			}
			destroyset(set);
			if (sym == SYM_DEFAULT)
			{
				getsym();
				switchnum[count]++;
				switchtable[switchcount].defaultflag = 1;
				switchtable[switchcount++].cx = cx;
				if (sym == SYM_COLON)
				{
					getsym();
					while (sym != SYM_END)
					{
						statement(fsys);
						getsym();
					}
				}
				else error(19);
			}
		}
		else error(19);
	}
	else if (sym == SYM_SWITCH)////////switch
	{
		switchflag++;
		breakcount++;
		count++;
		switchnum[count] = 0;
		getsym();
		cx3 = cx;
		gen(JMP, 0, 0);
		cx2 = cx;
		expression(fsys);
		cx1 = cx;
		code1 = (instruction*)malloc((cx1-cx2)*sizeof(instruction));
		for (j = 0; j < cx1 - cx2; j++)
			code1[j] = code[cx2 + j];
		cx = cx2;
		statement(fsys);
			 if (breaklist[breakcount] == NULL)
			{
				breaklist[breakcount] = (list)malloc(sizeof(alist));
				breaklist[breakcount]->position = cx;
				breaklist[breakcount]->next = NULL;
				gen(JMP, 0, 0);
			}
			else {
				q = (list)malloc(sizeof(alist));
				q->position = cx;
				q->next = breaklist[breakcount]->next;
				breaklist[breakcount]->next = q;
				gen(JMP, 0, 0);
			}
		switchflag--;
		code[cx3].a = cx;
		switchcount -= switchnum[count];
		for (j = 0; j < cx1 - cx2; j++)
			code[cx++] = code1[j];
		free(code1);
		for (j = switchcount; j < switchcount + switchnum[count]; j++)
		{
			if (switchtable[j].defaultflag)
				gen(JMP, 0, switchtable[j].cx);
			else
			{
				gen(CMP, 0, switchtable[j].num);
				gen(JZ, 0, switchtable[j].cx);
			}
		}
		backpatch(&breaklist[breakcount--], &cx);
		gen(POP, 0, 1);	
		count--;
		if (sym != SYM_SEMICOLON)  error(19);
	}
	else if (sym == SYM_FOR)//简单的for循环
	{
		loopflag++;
		getsym();
			if (sym == SYM_LPAREN)
			{
				getsym();
				breakcount++;
				continuecount++;
				set1 = createset(SYM_RPAREN,SYM_SEMICOLON, SYM_END, SYM_NULL);
				set = uniteset(set1, fsys);
				if(sym==SYM_IDENTIFIER) statement(set);
				else error(19);
				lastcx = cx;
				if (sym == SYM_SEMICOLON)
				{
					getsym();
					or_(set);
				}
				backpatch(&truelist[--truecount], &cx);
				cx1 = cx;
				if (sym == SYM_SEMICOLON)
					{
						getsym();
						statement(set);
					}
				if (sym == SYM_RPAREN)
				{
				getsym();
				}
				else error(22);
				cx2 = cx;
				code1 = (instruction*)malloc((cx2 - cx1)*sizeof(instruction));
				for (i = 0; i < cx2 - cx1; i++)
				{
					code1[i] = code[cx1 + i];
					if (code[cx1 + i].f >= 6 && code[cx1 + i].f <= 14)
					{
						code1[i].a = code[cx1 + i].a - cx1;
					}
				}
				for (i = 0; i < gototablecount ; i++)//////////////////////////////////////////////////////////////////////////////
				{
					if (gototable[i].cx >= cx1&&gototable[i].cx < cx2)
					{
						change[z++] = i;
					}
				}
				goto1 = gotohead ;
				goto2 = gototail ;
				while (goto1)
				{
					if (goto1->position >= cx1&&goto1->position < cx2)
						break;
					else goto1 = goto1->next;
				}
			if (breaklist[breakcount]!=NULL)//////////////////////////////////////////////////////
				p = breaklist[breakcount]->tail;
				if (continuelist[continuecount] != NULL)
				p1 = continuelist[continuecount]->tail;
				cx = cx1;
				statement(set);
				cx3 = cx;
				for (i = 0; i < z; i++)////////////////////////////////////////////////////////////////////////////////////////////
				{
					gototable[change[i]].cx += (cx - cx1);
				}
				while (goto1)
				{
					if (goto1 == goto2)
					{
						goto1->position += (cx - cx1);
						break;
					}
					goto1->position += (cx - cx1);
					goto1 = goto1->next;
				}
				if (p)
				{
					p2 = breaklist[breakcount];
					while (1)////////////////////////////////////////////////////////xunhuan
					{
						if (p2 == p)
						{
							p2->position += (cx - cx1);
							break;
						}
						else p2->position += (cx - cx1);
						p2 = p2->next;
					}
				}
				if (p1)
				{
					p2 = continuelist[continuecount];
					while (1)
					{
						if (p2 == p1)
						{
							p2->position += (cx - cx1);
							break;
						}
						else p2->position += (cx - cx1);
						p2 = p2->next;
					}
				}
				for (i = 0; i < cx2 - cx1; i++)
				{
					code[cx]=code1[i];
					if (code1[i].f >= 6 && code1[i].f <= 14)
					{
						code[cx].a = code1[i].a + cx3;
					}
					cx++;
				}
				free(code1);
				gen(JMP, 0, lastcx);
				backpatch(&continuelist[continuecount--], &cx3);
				backpatch(&falselist[--falsecount], &cx);
				backpatch(&breaklist[breakcount--], &cx);
			}
			else error(16);
	}

	else if (sym == SYM_BEGIN)
	{ // 
		getsym();
		set1 = createset( SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys)||elseflag )
		{
			if (elseflag)
			{
				elseflag--;
				sym = currentsym;
			}
			else if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_DO)
	{
		loopflag++;
		breakcount++;
		continuecount++;
		lastcx = cx;
		cx1 = cx;
		getsym();
		statement(fsys);
		cx2 = cx;
		if (sym == SYM_SEMICOLON)
			getsym();
		else error(10);
		if (sym == SYM_WHILE)
		{
			getsym();
		}
		else error(33);
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		else error(16);
		or_(fsys);
		backpatch(&falselist[--falsecount], &cx);
		cx3 = cx;
		if (sym == SYM_RPAREN)
			getsym();
		else error(22);		
		backpatch(&continuelist[continuecount--], &cx1);
		backpatch(&truelist[--truecount], &cx1);
		backpatch(&breaklist[breakcount--], &cx);
		if (sym != SYM_SEMICOLON)
			error(10);
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		loopflag++;
		breakcount++;
		continuecount++;
		cx1 = cx;
		getsym();
		if (sym == SYM_LPAREN)
		{
			getsym();
		}
		else error(16);
		or_(fsys);
		backpatch(&truelist[--truecount], &cx);
		if (sym == SYM_RPAREN)
			getsym();
		else error(22);
		statement(fsys);
		gen(JMP, 0, cx1);
		backpatch(&continuelist[continuecount--], &cx1);
		backpatch(&breaklist[breakcount--], &cx);
		backpatch(&falselist[--falsecount], &cx);
	}
	else if (sym == SYM_EXIT)//EXIT语句
	{
		q = (list)malloc(sizeof(alist));
		q->next = NULL;
		q->position = cx;
		gen(JMP, 0, 0);
		if (exitlist == NULL)
			exitlist = q;
		else p->next = q;
		p = q;
		getsym();
	}
	else if (sym == SYM_RETURN)
	{
		
		getsym();		
		expression(fsys); 
		gen(LEV,0,0);
	}

	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)//负责定义变量的部分
{
	int cx0; // initial code index
	mask* mk;
	int block_dx,i;
	int savedTx;
	int savedCx;
	symset set1, set;
	listforgoto goto1;
	dx = 3;
	//gotonum++;
	block_dx = dx;
	savedCx = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
			if (sym == SYM_CONST)
			{ // constant declarations
				getsym();
				do
				{
					constdeclaration();
					
					while (sym == SYM_COMMA)
					{
						getsym();
						constdeclaration();
					}
					if (sym == SYM_SEMICOLON)
					{
						getsym();
					}
					else
					{
						error(5); // Missing ',' or ';'.
					}
				} while (sym == SYM_IDENTIFIER);
			} // if

			if (sym == SYM_VAR)
			{ // variable declarations
				getsym();
				do
				{
					vardeclaration();
					while (sym == SYM_COMMA)
					{
						getsym();
						vardeclaration();
					}
					if (sym == SYM_SEMICOLON)
					{
						getsym();
					}
					else
					{
						error(5); // Missing ',' or ';'.
					}
				} while (sym == SYM_IDENTIFIER);
			} // if
			block_dx = dx; //save dx before handling procedure call!
			while (sym == SYM_PROCEDURE)
			{ // procedure declarations
				
				getsym();
				if (sym == SYM_IDENTIFIER)
				{						
					enter(ID_PROCEDURE);
					savedTx=tx;						
					getsym();
				}
				else
				{
					error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
				}
				if(sym == SYM_LPAREN)
				{
					getsym();				
				}
				else {
					error(31);//miss LPAREN
				}
				level++;			
				int q=0;
				while(sym != SYM_RPAREN)
				{
				if(sym == SYM_IDENTIFIER)
				{					
					vardeclaration();
					mk = (mask*)&table[tx];
					mk->address = q;
					if(table[tx].kind == ID_ARRAY)
						mk->level = 2;
					q++;
				}
				else if(sym == SYM_bitAND){
					enter(ID_VARIABLE);
					mk = (mask*)&table[tx];
					mk->address = q;
					mk->level = 2;
					q++;			
				}
				else
				{
					error(8);
							}
				if(sym == SYM_COMMA)
				{
					getsym();	
							}
				else if(sym == SYM_RPAREN){
					int p = tx;
					while(p!=savedTx){
						mk = (mask*)&table[p];
						mk->address -= (q+1);
						p--;
					}
					continue;
				}
				else error(0);
			
				}					
				getsym();
				set1 = createset(SYM_SEMICOLON, SYM_NULL);
				set = uniteset(set1, fsys);
				block(set);
				destroyset(set1);
				destroyset(set);
				table[savedTx].nt = tx+1;
				level--;
			} // while
			dx = block_dx; //restore dx after handling procedure call!
			set1 = createset(SYM_IDENTIFIER, SYM_NULL);
			set = uniteset(statbegsys, set1);
			test(set, declbegsys, 7);
			destroyset(set1);
			destroyset(set);
		
	}
	while (inset(sym, declbegsys));

	code[savedCx].a = cx;
	
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset( SYM_RPAREN,SYM_SEMICOLON,SYM_COMMA, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	table[tx+1].kind = -1;
	if (exitlist) backpatch(&exitlist, &cx);
	
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
	if (gotohead != NULL)
	{
		error(35);
		printf("%s", gotohead->s);
	}
	gototablecount = 0;
	//gotonum--;
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register
	int j;
	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_MOD:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] %= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1]; break;//后加的，可能不对
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1]; break;//后加的，可能不对
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1]; break;//后加的，可能不对
			case OPR_bitAND:
				top--;
				stack[top] &= stack[top + 1];//按位与
				break;
			case OPR_bitOR:
				top--;
				stack[top] |= stack[top + 1];//按位或
				break;
			case OPR_bitXOR:
				top--;
				stack[top] ^= stack[top + 1];//按位异或
				break;
			case OPR_OR:
				top--;
				stack[top] = stack[top + 1] || stack[top];//逻辑或
				break;
			case OPR_AND:
				top--;
				stack[top] = stack[top + 1] && stack[top];//逻辑与
				break;
			case OPR_NOT:
				top--;
				stack[top] = !stack[top];//逻辑非
				break;
			} // switch
			break;
		case POP:top -= i.a; break;
		case CMP:stack[++top] = stack[top - 1] - i.a; break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case ARR_LOD:
			stack[top] = stack[base(stack, b, i.l) + stack[top]+i.a];
			break;
		case ARR_STO:
				stack[base(stack, b, i.l) + stack[top]+i.a] = stack[top - 1];
				top--;
			printf("%d\n", stack[top]);
			top--;
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
//			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = b;
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case LEV:
		//stack[b-1]=count<<0
		stack[b-1-stack[b-1]] = stack[top];
		top = b - 1 -stack[b-1] ;
		pc = stack[b + 2];
		b = stack[b];
		break;
		case CST:
			int n;
		    j = b ;
		    printf("\ncurrent:b=%d,pc=%d\n",b,pc);
			while(j!=1){
				printf("b=%d,pc=%d  ",stack[j],stack[j+2]);
				n = stack[b-1];
				while(n--)
				printf("var:%d ",stack[j-n-2]);
				printf("\n"); 
				j = base(stack,j,1);
			}
			break;
		case RAN:
			if(i.a == -1)
				stack[++top] = rand();
			else
				stack[++top] = rand()%i.a + 1;
			break;
		case PTR:
			if(i.l == 1)
				printf("\n");
			else
				printf("%d ",stack[top--]);
						break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JZ:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case JE:
			if (stack[top] == stack[top-1])
				pc = i.a;
			top=top-2;
			break;
		case JNE:
			if (stack[top] != stack[top - 1])
				pc = i.a;
			top = top - 2;
			break;
		case JG:
			if (stack[top] < stack[top - 1])
				pc = i.a;
			top = top - 2;
			break;
		case JGE:
			if (stack[top] <= stack[top - 1])
				pc = i.a;
			top = top - 2;
			break;

		case JLE:
			if (stack[top] >= stack[top - 1])
				pc = i.a;
			top = top - 2;
			break;
		case JL:
			if (stack[top] > stack[top - 1])
				pc = i.a;
			top = top - 2;
			break;
		} // switch
	}
	while (pc);

	printf("\nEnd executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
int main ()
{
	FILE* hbin;
	char s[80] = { "switch.txt" };
	int i;
	symset set, set1, set2;

	//printf("Please input source file name: "); // get file name to be compiled
	//scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_DMINUS, SYM_DPLUS, SYM_LSHIFT, SYM_RSHIFT, SYM_CASE, SYM_DEFAULT,SYM_FOR, SYM_DO, SYM_SWITCH, SYM_NULL);
	facbegsys = createset(SYM_RANDOM,SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_DPLUS, SYM_DMINUS, SYM_EQU, SYM_LSHIFT, SYM_RSHIFT, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
    return(0);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
