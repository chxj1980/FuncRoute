#ifndef __C_KEYWORD_H__
#define __C_KEYWORD_H__


//C�ؼ���
char c_keywords[][25] =
{
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "inline", //(C99 ��)
    "int",
    "long",
    "register",
    "restrict", //(C99 ��)
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    "_Alignas", //(C11 ��)
    "_Alignof", //(C11 ��)
    "_Atomic", //(C11 ��)
    "_Bool", //(C99 ��)
    "_Complex", //(C99 ��)
    "_Generic", //(C11 ��)
    "_Imaginary", //(C99 ��)
    "_Noreturn", //(C11 ��)
    "_Static_assert", //(C11 ��)
    "_Thread_local", //(C11 ��)
};


//CԤ��������
char c_preprocessors[][25] =
{
    "if",
    "elif",
    "else",
    "endif",
    "defined",
    "ifdef",
    "ifndef",
    "define",
    "undef",
    "include",
    "line",
    "error",
    "pragma",
};

#endif //__C_KEYWORD_H__
