/* 
 * File:   procs.h
 * Author: Varun
 *
 * Created on April 2, 2013, 3:41 AM
 */

#ifndef PROCS_H
#define	PROCS_H
#define LENGTH(X) (sizeof(X)/sizeof(X[0]))

typedef struct nodeType{
    std::string name;
    struct nodeType** child;
    int count;
} Node;
void disp_tree(Node*);
void disp_tree(Node*, int);
void stack_disp();
int expect(std::string);
int read(std::string);
void error(std::string);
void put(std::string);

void Tiny();
void Consts();
void Const();
void Types();
void Type();
void LitList();
void SubProgs();
void Fcn();
void Params();
void Dclns();
void Dcln();
void Body();
void OutExp();
void StringNode();
int Caseclauses();
void Caseclause();
void ConstValue();
void CaseExpression();
void Statement();
int OtherwiseClause();
void ForStat();
void Assignment();
void ForExp();
void Expression();
void Factor();
void Term();
void Primary();
void Name();

#endif	/* PROCS_H */

