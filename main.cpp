/* 
 * File:   main.cpp
 * Author: Varun
 *
 */

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include <stack>
#include "procs.h"

using namespace std;

string tokens[] = {"(",")","*","+",",","-",".","..","/",":",":=",":=:",";","<",
                    "<=","<>","<char>","<identifier>","<integer>","<string>",
                    "=",">",">=","and","begin","case","chr","const","do","else","end",
                    "eof","exit","for","function","if","loop","mod","not","of",
                    "or","ord","otherwise","output","pool","pred","program","read","repeat",
                    "return","succ","then","type","until","var","while"};

stack<Node*> s;
ifstream in;
string nextToken;

int isValidToken(string t){
    for(int i = 0; i < LENGTH(tokens); i++)
        if(tokens[i] == t)
            return i;
    return -1;
}
void getNext(){
    string temp = "";
    while(isValidToken(temp) < 0 && in.good()){
        if(in.get() == ' ')
            continue;
        temp += in.get();
    }
    nextToken = temp;
}
void consume(char till){
    char ch;
    while((ch = in.get()) != till);
}
int read(string t){
    if(expect(t)){
        Node* temp = new Node;
        temp->name = t;
        temp->count = 0;
        temp->child = NULL;
        s.push(temp);
        return 1;
    }
    else{
        return 0;
    }
}
int expect(string t){
    int c = 0;
    char ch;
    string token;
    const char *str = t.c_str();
    while(isspace(ch = in.peek()))
        in.get();
    if((ch = in.get()) != str[c++]){
        in.putback(ch);
        return 0;
    }
    token += ch;
    while((ch = in.get()) == str[c++]){
        token += ch;
    }
    in.putback(ch);
    if(token == t){
        nextToken = token;
        return 1;
    }
    else{
        put(token);
        return 0;
    }
}
void put(string t){
    int l = t.size();
    const char* ch = t.c_str();
    for(int i = l-1; i >= 0; i--){
        in.putback(ch[i]);
    }
}
void buildTree(string name, int count){
    
    Node* n = new Node;
    n->name = name;
    n->count = count;
    n->child = new Node*[count];
    
    for(int i = count-1; i >= 0; i--){
        Node* temp = s.top();
        s.pop();
        n->child[i] = temp;
    }
    s.push(n);
    //stack_disp();
    //disp_tree(s.top());
}
void error(string msg){
    cout<<"Error: "<<msg<<endl;
    exit(1);
}
int identifier(){
    int ch;
    string id;
    while(isspace(ch = in.peek()))
        in.get();
    if (!((ch = in.get()) == '_' || isalpha(ch))){
        in.putback(ch);
        return 0;
    }
    do { 
        id += ch;
        ch = in.get();
    } while (isalpha(ch) || isdigit(ch) || ch == '_');
    in.putback(ch);
    if(isValidToken(id) >= 0){
        put(id);
        return 0;
    }
    nextToken = id;
    Node* temp = new Node;
    temp->name = id;
    temp->count = 0;
    temp->child = NULL;
    s.push(temp);
    buildTree("<identifier>", 1);
    return 1;
}
int expectIdentifier(){
    int ch;
    string id;
    while(isspace(ch = in.peek()))
        in.get();
    if (!((ch = in.get()) == '_' || isalpha(ch))){
        in.putback(ch);
        return 0;
    }
    do { 
        id += ch;
        ch = in.get();
    } while (isalpha(ch) || isdigit(ch) || ch == '_');
    in.putback(ch);
    put(id);
    if(isValidToken(id) == -1)
        return 1;
    else return 0;
}
int integer(){
    int ch;
    string id;
    while(isspace(ch = in.peek()))
        in.get();
    ch = in.get();
    if (!isdigit(ch)){
        in.putback(ch);
        return 0;
    }
    do { 
        id += ch;
        ch = in.get();
    } while (isdigit(ch));
    in.putback(ch);
    nextToken = id;
    Node* temp = new Node;
    temp->name = id;
    temp->count = 0;
    temp->child = NULL;
    s.push(temp);
    buildTree("<integer>", 1);
    return 1;
}
int expectInteger(){
    int ch;
    string id;
    while(isspace(ch = in.peek()))
        in.get();
    ch = in.get();
    if (!isdigit(ch)){
        in.putback(ch);
        return 0;
    }
    do { 
        id += ch;
        ch = in.get();
    } while (isdigit(ch));
    in.putback(ch);
    put(id);
    nextToken = id;
    return 1;
}
int character(){
    int ch;
    string id;
    while(isspace(ch = in.peek()))
        in.get();
    if (!((ch = in.get()) == '\'')){
        in.putback(ch);
        return 0;
    }
    if ((ch = in.get()) == '\''){
        in.putback(ch);
        return 0;
    }
    else{
        id += ch;
    }
    if (!((ch = in.get()) == '\'')){
        in.putback(ch);
        in.putback(id.at(0));
        in.putback('\'');
        return 0;
    }
    nextToken = id;
    Node* temp = new Node;
    temp->name = "'" + id + "'";
    temp->count = 0;
    temp->child = NULL;
    s.push(temp);
    buildTree("<char>", 1);
    return 1;
}
int expectCharacter(){
    int ch;
    string id;
    while(isspace(ch = in.peek()))
        in.get();
    if (!((ch = in.get()) == '\'')){
        in.putback(ch);
        return 0;
    }
    if ((ch = in.get()) == '\''){
        in.putback(ch);
        return 0;
    }
    else{
        id += ch;
    }
    if (!((ch = in.get()) == '\'')){
        in.putback(ch);
        in.putback(id.at(0));
        in.putback('\'');
        return 0;
    }
    in.putback('\'');
    in.putback(id.at(0));
    in.putback('\'');
    nextToken = id;
    return 1;
}
int stringnode(){
    int ch;
    string id;
    if (!((ch = in.get()) == '\"')){
        in.putback(ch);
        return 0;
    }
    while ((ch = in.get()) != '\"') { 
        id += ch;
    }
    in.putback(ch);
    if (!((ch = in.get()) == '\"')){
        in.putback(ch);
        in.putback(id.at(0));
        in.putback('\'');
        return 0;
    }
    Node* temp = new Node;
    temp->name = id;
    temp->count = 0;
    temp->child = NULL;
    s.push(temp);
    buildTree("<string>", 1);
    return 1;
}
void Tiny(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    if(expect("program")){
        Name();
    }
    if(expect(":")){
        Consts();
        Types();
        Dclns();
        SubProgs();
        Body();
        Name();
    }
    if(expect(".")){
        buildTree("program", 7);
    }
}
void Consts(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    if(expect("const")){
        int n = 1;
        Const();
        while(expect(",")){
            Const();
            n++;
        }
        buildTree("consts", n);
    }
    else{
        buildTree("consts", 0);
    }
}
void Const(){
    Name();
    if(!expect("=")) error("Const: Invalid Syntax");
    ConstValue();
    buildTree("const", 2);
}
void Types(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    if(expect("type")){
        int n = 1;
        Type();
        if(!expect(";")) error("Types: Invalid Syntax");
        while(expectIdentifier()){
            Type();
            if(!expect(";")) error("Types: Invalid Syntax");
            n++;
        }
        buildTree("types", n);
    }
    else{
        buildTree("types", 0);
    }
}
void Type(){
    Name();
    if(!expect("=")) error("Type: Invalid Syntax");
    LitList();
    buildTree("type", 2);
}
void LitList(){
    if(!expect("(")) error("LitList: Invalid Syntax");
    int n = 1;
    Name();
    while(expect(",")){
        Name();
        n++;
    }
    if(!expect(")")) error("LitList: Invalid Syntax");
    buildTree("lit", n);
}
void SubProgs(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    int n = 0;
    while(expect("function")){
        put("function");
        Fcn();
        n++;
    }
    buildTree("subprogs", n);
}
void Fcn(){
    if(expect("function")){
        Name();
        if(!expect("(")) error("Function: Invalid Syntax");
        Params();
        if(!expect(")")) error("Function: Invalid Syntax");
        if(!expect(":")) error("Function: Invalid Syntax");
        Name();
        if(!expect(";")) error("Function: Invalid Syntax"); 
        Consts();
        Types();
        Dclns();
        Body();
        Name();
        if(!expect(";")) error("Function: Invalid Syntax");
        buildTree("fcn", 8);
    }
}
void Params(){
    int n = 1;
    Dcln();
    while(expect(";")){
        Dcln();
        n++;
    }
    buildTree("params", n);
}
void Dclns(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    if(expect("var")){
        Dcln();
        if(!expect(";")) error("Dclns: Invalid Syntax");
        int n = 1;
        while(expectIdentifier()){
            Dcln();
            if(!expect(";")) error("Dclns: Invalid Syntax");
            n++;
        }
        buildTree("dclns", n);
    }
    else{
        buildTree("dclns",0);
    }
}
void Dcln(){
    int n = 1;
    Name();
    while(expect(",")){
        Name();
        n++;
    }
    if(!expect(":")) error("Declaration: Invalid Syntax");
    Name();
    buildTree("var", n+1);
}
void Body(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    if(!expect("begin")) error("BEGIN Missing");
    int n = 1;
    if(expect("{"))
        consume('}');
    Statement();
    if(expect("#"))
        consume('\n');
    while(expect(";")){
        Statement();
        if(expect("#"))
            consume('\n');
        n++;
    }
    if(!expect("end")) error("END Missing");
    buildTree("block", n);
}
void Statement(){
    int n;
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    if(expect("output")){
        if(!expect("(")) error("Statement: Invalid Syntax");
        n = 1;
        OutExp();
        while(expect(",")){
            OutExp();
            n++;
        }
        if(!expect(")")) error("Statement: Invalid Syntax");
        buildTree("output", n);
    }
    else if(expect("if")){
        n = 2;
        Expression();
        if(!expect("then"))
            error("IF: Invalid Syntax");
        Statement();
        if(expect("else")){
            Statement();
            n++;
        }
        buildTree("if", n);
    }
    else if(expect("while")){
        Expression();
        if(!expect("do")) error("WHILE: Invalid Syntax");
        Statement();
        buildTree("while", 2);
    }
    else if(expect("repeat")){
        n = 1;
        Statement();
        while(expect(";")){
            Statement();
            n++;
        }
        if(!expect("until")) error("REPEAT: Invalid Syntax");
        Expression();
        buildTree("repeat", n+1);
    }
    else if(expect("for")){
        if(!expect("(")) error("FOR: Invalid Syntax");
        ForStat();
        if(!expect(";")) error("FOR: Invalid Syntax");
        ForExp();
        if(!expect(";")) error("FOR: Invalid Syntax");
        ForStat();
        if(!expect(")")) error("FOR: Invalid Syntax");
        Statement();
        buildTree("for", 4);
    }
    else if(expect("loop")){
        n = 1;
        Statement();
        while(expect(";")){
            Statement();
            n++;
        }
        if(!expect("pool")) error("LOOP: Invalid Syntax");
        buildTree("loop", n);
    }
    else if(expect("case")){
        int n = 1;
        Expression();
        if(!expect("of")) error("CASE: Invalid Syntax");
        n += Caseclauses();
        n += OtherwiseClause();
        if(!expect("end")) error("CASE: Invalid Syntax");
        buildTree("case", n);
    }
    else if(expect("read")){
        n = 1;
        if(!expect("(")) error("READ: Invalid Syntax");
        Name();
        while(expect(",")){
            Name();
            n++;
        }
        if(!expect(")")) error("READ: Invalid Syntax");
        buildTree("read", n);
    }
    else if(expect("exit")){
        buildTree("exit", 0);
    }
    else if(expect("return")){
        Expression();
        buildTree("return", 1);
    }
    else if(expect("begin")){
        put("begin");
        Body();
    }
    else if(expectIdentifier()){
        Assignment();
    }
    else{
        if(expect("{")){
            consume('}');
        }
        if(expect("#")){
            consume('\n');
        }
        buildTree("<null>", 0);
    }
}
void OutExp(){
    if(stringnode()){
        buildTree("string", 1);
    }
    else{
        Expression();
        buildTree("integer", 1);
    }
}
void StringNode(){
    if(stringnode()){
        ;
    }
    else{
        error("Invalid Syntax");
    }
}
void ConstValue(){
    if(integer()){
        ;
    }
    else if(character()){
        ;
    }
    else{
        Name();
    }
}
int Caseclauses(){
    int n = 1;
    Caseclause();
    if(expect(";")){
        ;
    }
    while(expectInteger() || expectCharacter() || expectIdentifier()){
        Caseclause();
        if(expect(";")){
            n++;
        }
    }
    return n;
}
void Caseclause(){
    CaseExpression();
    int n = 1;
    while(expect(",")){
        CaseExpression();
        n++;
    }
    if(expect(":")){
        Statement();
        buildTree("case_clause", n + 1);
    }
    else{
        error("Caseclause: Invalid Syntax");
    }
}
void CaseExpression(){
    ConstValue();
    if(expect("..")){
        ConstValue();
        buildTree("..", 2);
    }
}
int OtherwiseClause(){
    if(expect("otherwise")){
        Statement();
        buildTree("otherwise", 1);
        return 1;
    }
    return 0;
}
void ForStat(){
    if(expectIdentifier()){
        Assignment();
    }
    else{
        buildTree("<null>", 0);
    }
}
void Assignment(){
    if(expect("{"))
        consume('}');
    if(expect("#"))
        consume('\n');
    Name();
    if(expect(":=")){
        Expression();
        buildTree("assign", 2);
    }
    else if(expect(":=:")){
        Name();
        buildTree("swap", 2);
    }
}
void ForExp(){
    
    if(expect(";")){
        put(";");
        buildTree("true",0);
    }
    else
       Expression(); 
}
void Expression(){
    Term();
    if(expect("<=")){
        Term();
        buildTree("<=", 2);
    }
    else if(expect("<>")){
        Term();
        buildTree("<>", 2);
    }
    else if(expect("<")){
        Term();
        buildTree("<", 2);
    }
    else if(expect(">=")){
        Term();
        buildTree(">=", 2);
    }
    else if(expect(">")){
        Term();
        buildTree(">", 2);
    }
    else if(expect("=")){
        Term();
        buildTree("=", 2);
    }
}
void Term(){
    Factor();
    while(expect("+") || expect("-") || expect("or")){
        if(nextToken == "+"){
            Factor();
            buildTree("+", 2);
        }
        else if(nextToken == "-"){
            Factor();
            buildTree("-", 2);
        }
        else if(nextToken == "or"){
            Factor();
            buildTree("or", 2);
        }
    }
}
void Factor(){
    Primary();
    while(expect("*") || expect("/") || expect("and") || expect("mod")){
        if(nextToken == "*"){
            Primary();
            buildTree("*", 2);
        }
        else if(nextToken == "/"){
            Primary();
            buildTree("/", 2);
        }
        else if(nextToken == "and"){
            Primary();
            buildTree("and", 2);
        }
        else if(nextToken == "mod"){
            Primary();
            buildTree("mod", 2);
        }
    }
    
}
void Primary(){
    if(expect("-")){
        Primary();
        buildTree("-", 1);
    }
    else if(expect("+")){
        Primary();
    }
    else if(expect("not")){
        Primary();
        buildTree("not", 1);
    }
    else if(expect("eof")){
        buildTree("eof", 0);
    }
    else if(integer()){
        ;
    }
    else if(character()){
        ;
    }
    else if(expect("succ")){
        if(!expect("(")) error("Primary: Invalid Syntax");
        Expression();
        if(!expect(")")) error("Primary: Invalid Syntax");
        buildTree("succ", 1);
    }
    else if(expect("pred")){
        if(!expect("(")) error("Primary: Invalid Syntax");
        Expression();
        if(!expect(")")) error("Primary: Invalid Syntax");
        buildTree("pred", 1);
    }
    else if(expect("ord")){
        if(!expect("(")) error("Primary: Invalid Syntax");
        Expression();
        if(!expect(")")) error("Primary: Invalid Syntax");
        buildTree("ord", 1);
    }
    else if(expect("chr")){
        if(!expect("(")) error("Primary: Invalid Syntax");
        Expression();
        if(!expect(")")) error("Primary: Invalid Syntax");
        buildTree("chr", 1);
    }
    else if(expect("(")){
        Expression();
        expect(")");
    }
    else{
        Name();
        int n = 1;
        if(expect("(")){
            Expression();
            n++;
            while(expect(",")){
                Expression();
                n++;
            }
            expect(")");
            buildTree("call", n);
        }
    }
    
}
void Name(){
    if(!identifier()){
        char ch = in.get();
        stringstream ss;
        string s;
        ss << ch;
        ss >> s;
        s = "Unexpected token '" + s + "'";
        error(s);
    }
}
void stack_disp(){
    cout<<endl<<"Stack\n------\n";
    stack<Node*> temp;
    Node *n;
    while(!s.empty()){
        n = s.top();
        cout<<n->name<<"("<<n->count<<")"<<endl;
        temp.push(n);
        s.pop();
    }
    cout<<"------"<<endl;
    while(!temp.empty()){
        n = temp.top();
        s.push(n);
        temp.pop();
    }
}
void disp_tree(Node *node){
    disp_tree(node, 0);
    cout<<endl;
}
void disp_tree(Node *node, int level){
    if(node == NULL)
        return;
    for(int i = 1; i <= level; i++)
        cout<<". ";
    cout<<node->name<<"("<<node->count<<")"<<endl;

    if(node->child == NULL)
        return;
    
    for(int i = 0; i < node->count; i++){
        disp_tree(node->child[i], level+1);
    }
    
}
void clean(){
    string data;
    char ch;
    while(in.good()){
        ch = in.get();
        if(ch == '{'){
            string temp;
            temp += ch;
            while((ch = in.get()) != '}' && in.good()){
                temp += ch;
            }
            if(ch != '}')
                put(temp);
        }
        else if(ch == '#'){
            while((ch = in.get()) != '\n' && in.good());
        }
        else{
            if(isascii(ch))
                data += ch;
            //in.putback(ch);
        }
    }
    //cout<<data;
    put(data);
}
int main(int argc, char** argv) {
    string file, line; 
    char buffer[2];
    file = argv[1];
    in.open(file.c_str());
    
    //clean();
    
    Tiny();
    
    disp_tree(s.top());
    
    return 0;
}

