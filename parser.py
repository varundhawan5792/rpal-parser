# Python 3 port of the C++ code by ChatGPT
# Usage:  python parser.py <source.tiny>

from __future__ import annotations

import sys
from pathlib import Path
from typing import List, Optional

# ---------------------------------------------------------------------------
# Token table (unchanged from C++ code, order matters for grammar)
# ---------------------------------------------------------------------------
TOKENS = [
    "(", ")", "*", "+", ",", "-", ".", "..", "/", ":", ":=", ":=:", ";", "<",
    "<=", "<>", "<char>", "<identifier>", "<integer>", "<string>",
    "=", ">", ">=", "and", "begin", "case", "chr", "const", "do", "else", "end",
    "eof", "exit", "for", "function", "if", "loop", "mod", "not", "of",
    "or", "ord", "otherwise", "output", "pool", "pred", "program", "read", "repeat",
    "return", "succ", "then", "type", "until", "var", "while",
]
TOKEN_SET = set(TOKENS)

# ---------------------------------------------------------------------------
# AST node
# ---------------------------------------------------------------------------
class Node:
    __slots__ = ("name", "count", "child")

    def __init__(self, name: str, count: int = 0, child: Optional[List["Node"]] = None):
        self.name = name
        self.count = count
        self.child: List[Node] = child or []

    def dump(self, level: int = 0):
        print(". " * level + f"{self.name}({self.count})")
        for c in self.child:
            c.dump(level + 1)

# ---------------------------------------------------------------------------
# Character stream with put‑back
# ---------------------------------------------------------------------------
class CharStream:
    def __init__(self, text: str):
        self.text = text
        self.pos = 0
        self.buf: List[str] = []

    def get(self) -> str:
        if self.buf:
            return self.buf.pop()
        if self.pos >= len(self.text):
            return ""  # EOF marker
        ch = self.text[self.pos]
        self.pos += 1
        return ch

    def unget(self, ch: str):
        if ch:
            self.buf.append(ch)

    def peek(self) -> str:
        ch = self.get()
        self.unget(ch)
        return ch

# ---------------------------------------------------------------------------
# Global parser state
# ---------------------------------------------------------------------------
stack: List[Node] = []
stream: CharStream
next_token: str = ""

# ---------------------------------------------------------------------------
# Utility helpers
# ---------------------------------------------------------------------------

def is_valid_token(tok: str) -> bool:
    return tok in TOKEN_SET

def build_tree(name: str, count: int):
    node = Node(name, count)
    node.child = [stack.pop() for _ in range(count)][::-1]
    stack.append(node)

# ---------------------------------------------------------------------------
# Low‑level character/lexical routines
# ---------------------------------------------------------------------------

def _skip_ws_and_comments():
    while True:
        ch = stream.peek()
        if ch.isspace():
            stream.get()
            continue
        if ch == '{':
            stream.get()
            while (c := stream.get()) and c != '}':
                pass
            continue
        if ch == '#':
            while (c := stream.get()) and c != '\n':
                pass
            continue
        break

def expect(literal: str) -> bool:
    global next_token
    _skip_ws_and_comments()
    if not literal:
        return False
    if len(literal) == 1:
        ch = stream.get()
        if ch == literal:
            next_token = literal
            return True
        stream.unget(ch)
        return False
    consumed = ""
    for ch_expect in literal:
        ch = stream.get()
        if ch != ch_expect:
            # push back
            stream.unget(ch)
            for c in reversed(consumed):
                stream.unget(c)
            return False
        consumed += ch
    next_token = literal
    return True

def put(text: str):
    for ch in reversed(text):
        stream.unget(ch)

# Id / int / char / string scanners


def _scan_identifier(push: bool) -> bool:
    _skip_ws_and_comments()
    ch = stream.get()
    if not (ch == '_' or ch.isalpha()):
        stream.unget(ch)
        return False
    ident = ch
    while True:
        ch = stream.get()
        if ch.isalnum() or ch == '_':
            ident += ch
        else:
            stream.unget(ch)
            break
    if is_valid_token(ident):
        put(ident)
        return False
    global next_token
    next_token = ident
    if push:
        stack.append(Node(ident))
        build_tree("<identifier>", 1)
    return True

def identifier():
    return _scan_identifier(True)

def expect_identifier():
    return _scan_identifier(False)


def _scan_integer(push: bool) -> bool:
    _skip_ws_and_comments()
    ch = stream.get()
    if not ch.isdigit():
        stream.unget(ch)
        return False
    num = ch
    while True:
        ch = stream.get()
        if ch.isdigit():
            num += ch
        else:
            stream.unget(ch)
            break
    global next_token
    next_token = num
    if push:
        stack.append(Node(num))
        build_tree("<integer>", 1)
    return True

def integer():
    return _scan_integer(True)

def expect_integer():
    return _scan_integer(False)


def _scan_char(push: bool = False) -> bool:
    _skip_ws_and_comments()
    first = stream.get()
    if first != "'":  # doesn’t start a char literal
        stream.unget(first)
        return False

    second = stream.get()
    if second == "'":  # empty char literal → backtrack
        stream.unget(second)
        stream.unget("'")
        return False

    closing = stream.get()
    if closing != "'":  # unterminated – backtrack fully
        stream.unget(closing)
        stream.unget(second)
        stream.unget("'")
        return False

    global next_token
    next_token = second

    if push:
        stack.append(Node(f"'{second}'"))
        build_tree("<char>", 1)
    return True

def character():
    return _scan_char(True)

def expect_character():
    return _scan_char(False)


def stringnode():
    _skip_ws_and_comments()
    first = stream.get()
    if first != '"':
        stream.unget(first)
        return False

    content = ""
    while (ch := stream.get()) and ch != '"':
        content += ch

    if ch != '"':  # unterminated
        stream.unget(ch)
        for c in reversed(content):
            stream.unget(c)
        stream.unget('"')
        return False

    stack.append(Node(content))
    build_tree("<string>", 1)
    return True

# ---------------------------------------------------------------------------
# Error helper
# ---------------------------------------------------------------------------

def error(msg: str):
    raise SyntaxError(msg)

# ---------------------------------------------------------------------------
# Grammar routines (direct translation of C++ code)
# ---------------------------------------------------------------------------

def Tiny():
    if expect("program"):
        Name()
    if expect(":"):
        Consts()
        Types()
        Dclns()
        SubProgs()
        Body()
        Name()
    if not expect("."):
        error("Program dot missing")
    build_tree("program", 7)

# consts ::= 'const' const {',' const} | ε

def Consts():
    if expect("const"):
        n = 1
        Const()
        while expect(","):
            Const()
            n += 1
        build_tree("consts", n)
    else:
        build_tree("consts", 0)

def Const():
    Name()
    if not expect("="):
        error("Const: '=' expected")
    ConstValue()
    build_tree("const", 2)

def Types():
    if expect("type"):
        n = 1
        Type()
        if not expect(";"):
            error("Types: ';' expected")
        while expect_identifier():
            Type()
            if not expect(";"):
                error("Types: ';' expected")
            n += 1
        build_tree("types", n)
    else:
        build_tree("types", 0)

def Type():
    Name()
    if not expect("="):
        error("Type: '=' expected")
    LitList()
    build_tree("type", 2)

def LitList():
    if not expect("("):
        error("LitList: '(' expected")
    n = 1
    Name()
    while expect(","):
        Name()
        n += 1
    if not expect(")"):
        error("LitList: ')' expected")
    build_tree("lit", n)

def SubProgs():
    n = 0
    while expect("function"):
        put("function")  # re‑inject for Fcn
        Fcn()
        n += 1
    build_tree("subprogs", n)

def Fcn():
    if not expect("function"):
        return
    Name()
    if not expect("("):
        error("Fcn: '(' expected")
    Params()
    if not expect(")"):
        error("Fcn: ')' expected")
    if not expect(":"):
        error("Fcn: ':' expected")
    Name()
    if not expect(";"):
        error("Fcn: ';' expected")
    Consts()
    Types()
    Dclns()
    Body()
    Name()
    if not expect(";"):
        error("Fcn: trailing ';' expected")
    build_tree("fcn", 8)

def Params():
    Dcln()
    n = 1
    while expect(";"):
        Dcln()
        n += 1
    build_tree("params", n)

def Dclns():
    if expect("var"):
        Dcln()
        if not expect(";"):
            error("Dclns: ';' expected")
        n = 1
        while expect_identifier():
            Dcln()
            if not expect(";"):
                error("Dclns: ';' expected")
            n += 1
        build_tree("dclns", n)
    else:
        build_tree("dclns", 0)

def Dcln():
    n = 1
    Name()
    while expect(","):
        Name()
        n += 1
    if not expect(":"):
        error("Dcln: ':' expected")
    Name()
    build_tree("var", n + 1)

def Body():
    if not expect("begin"):
        error("BEGIN missing")
    n = 0
    Statement()
    n += 1
    while expect(";"):
        Statement()
        n += 1
    if not expect("end"):
        error("END missing")
    build_tree("block", n)

def Statement():
    if expect("output"):
        if not expect("("):
            error("output: '(' expected")
        n = 1
        OutExp()
        while expect(","):
            OutExp()
            n += 1
        if not expect(")"):
            error("output: ')' expected")
        build_tree("output", n)
        return

    if expect("if"):
        Expression()
        if not expect("then"):
            error("if: 'then' expected")
        Statement()
        n = 2
        if expect("else"):
            Statement()
            n += 1
        build_tree("if", n)
        return

    if expect("while"):
        Expression()
        if not expect("do"):
            error("while: 'do' expected")
        Statement()
        build_tree("while", 2)
        return

    if expect("repeat"):
        n = 1
        Statement()
        while expect(";"):
            Statement()
            n += 1
        if not expect("until"):
            error("repeat: 'until' expected")
        Expression()
        build_tree("repeat", n + 1)
        return

    if expect("for"):
        if not expect("("):
            error("for: '(' expected")
        ForStat()
        if not expect(";"):
            error("for: ';' expected")
        ForExp()
        if not expect(";"):
            error("for: ';' expected")
        ForStat()
        if not expect(")"):
            error("for: ')' expected")
        Statement()
        build_tree("for", 4)
        return

    if expect("loop"):
        n = 1
        Statement()
        while expect(";"):
            Statement()
            n += 1
        if not expect("pool"):
            error("loop: 'pool' expected")
        build_tree("loop", n)
        return

    if expect("case"):
        Expression()
        if not expect("of"):
            error("case: 'of' expected")
        n = Caseclauses() + OtherwiseClause() + 1
        if not expect("end"):
            error("case: 'end' expected")
        build_tree("case", n)
        return

    if expect("read"):
        if not expect("("):
            error("read: '(' expected")
        n = 1
        Name()
        while expect(","):
            Name()
            n += 1
        if not expect(")"):
            error("read: ')' expected")
        build_tree("read", n)
        return

    if expect("exit"):
        build_tree("exit", 0)
        return

    if expect("return"):
        Expression()
        build_tree("return", 1)
        return

    if expect("begin"):
        put("begin")
        Body()
        return

    if expect_identifier():
        put(next_token)  # re‑inject
        Assignment()
        return

    # empty statement
    build_tree("<null>", 0)


def OutExp():
    if stringnode():
        build_tree("string", 1)
    else:
        Expression()
        build_tree("integer", 1)


def StringNode():
    if not stringnode():
        error("string literal expected")


def ConstValue():
    if integer() or character():
        return
    Name()


def Caseclauses():
    count = 1
    Caseclause()
    if expect(";"):
        pass
    while True:
        if expect_integer() or expect_character() or expect_identifier():
            Caseclause()
            count += 1
            expect(";")
        else:
            break
    return count

def Caseclause():
    n = 1
    CaseExpression()
    while expect(","):
        CaseExpression()
        n += 1
    if not expect(":" ):
        error("case clause ':' expected")
    Statement()
    build_tree("case_clause", n + 1)


def CaseExpression():
    ConstValue()
    if expect(".."):
        ConstValue()
        build_tree("..", 2)


def OtherwiseClause():
    if expect("otherwise"):
        Statement()
        build_tree("otherwise", 1)
        return 1
    return 0

def ForStat():
    if expect_identifier():
        put(next_token)
        Assignment()
    else:
        build_tree("<null>", 0)


def Assignment():
    Name()
    if expect(":="):
        Expression()
        build_tree("assign", 2)
    elif expect(":=:"):
        Name()
        build_tree("swap", 2)
    else:
        error("assignment operator expected")


def ForExp():
    if expect(";"):
        put(";")
        build_tree("true", 0)
    else:
        Expression()


def Expression():
    Term()
    if expect("<="):
        Term(); build_tree("<=", 2)
    elif expect("<>"):
        Term(); build_tree("<>", 2)
    elif expect("<"):
        Term(); build_tree("<", 2)
    elif expect(">="):
        Term(); build_tree(">=", 2)
    elif expect(">"):
        Term(); build_tree(">", 2)
    elif expect("="):
        Term(); build_tree("=", 2)


def Term():
    Factor()
    while True:
        if expect("+"):
            Factor(); build_tree("+", 2)
        elif expect("-"):
            Factor(); build_tree("-", 2)
        elif expect("or"):
            Factor(); build_tree("or", 2)
        else:
            break


def Factor():
    Primary()
    while True:
        if expect("*"):
            Primary(); build_tree("*", 2)
        elif expect("/"):
            Primary(); build_tree("/", 2)
        elif expect("and"):
            Primary(); build_tree("and", 2)
        elif expect("mod"):
            Primary(); build_tree("mod", 2)
        else:
            break


def Primary():
    if expect("-"):
        Primary(); build_tree("-", 1); return
    if expect("+"):
        Primary(); return
    if expect("not"):
        Primary(); build_tree("not", 1); return
    if expect("eof"):
        build_tree("eof", 0); return
    if integer() or character():
        return
    if expect("succ"):
        if not expect("("):
            error("succ: '(' expected")
        Expression()
        if not expect(")"):
            error("succ: ')' expected")
        build_tree("succ", 1); return
    if expect("pred"):
        if not expect("("):
            error("pred: '(' expected")
        Expression()
        if not expect(")"):
            error("pred: ')' expected")
        build_tree("pred", 1); return
    if expect("ord"):
        if not expect("("):
            error("ord: '(' expected")
        Expression()
        if not expect(")"):
            error("ord: ')' expected")
        build_tree("ord", 1); return
    if expect("chr"):
        if not expect("("):
            error("chr: '(' expected")
        Expression()
        if not expect(")"):
            error("chr: ')' expected")
        build_tree("chr", 1); return
    if expect("("):
        Expression()
        if not expect(")"):
            error("primary: ')' expected")
        return
    # identifier (possibly call)
    Name()
    n = 1
    if expect("("):
        Expression(); n += 1
        while expect(","):
            Expression(); n += 1
        if not expect(")"):
            error("call: ')' expected")
        build_tree("call", n)


def Name():
    if not identifier():
        ch = stream.get()
        error(f"Unexpected token '{ch}'")

# ---------------------------------------------------------------------------
# Driver
# ---------------------------------------------------------------------------

def parse_file(path: str) -> Node:
    global stream, stack
    stack.clear()
    text = Path(path).read_text()
    stream = CharStream(text)
    Tiny()
    if not stack:
        raise RuntimeError("Empty AST: parse failed")
    return stack[-1]


def main():
    if len(sys.argv) != 2:
        print("Usage: python parser.py <source.tiny>")
        sys.exit(1)
    root = parse_file(sys.argv[1])
    root.dump()

if __name__ == "__main__":
    main()
