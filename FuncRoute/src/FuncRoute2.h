#ifndef __FUNC_ROUTE2_H__
#define __FUNC_ROUTE2_H__

#include <string>
#include <vector>


char cpp_keywords[][25] =
{
};

typedef enum _CPP_STEP_
{
	/*
	* C/C++�﷨Ԫ��
	*/
	CPP_STEP0_UNKNOWN = 0,    //δ֪�﷨Ԫ��

	/*
	* ����1����ԭʼ���룬���ÿհ��ַ����ָ�������� ����+��ĸ+�»��� ��ɵĵ���
	*/
	CPP_STEP1_WORD_UNKNOWN,              //�������ͣ�δ֪��������� +,-,*,/,,%,!,?,~,=,==,<,<=,=>,>,<<,>>,&,|,,||,^,//,/*,*/,",',#,;,:,::,[,],...��Щ �� ����+��ĸ+�»��� ����Ϊ����
	CPP_STEP1_WORD_KEYWORD,              //�������ͣ�C/C++�ؼ��� char,int,short,long,float,double,unsigned,if,else,for,do,while,private,protected,public,new,delete,class,struct,...
	CPP_STEP1_WORD_UNKNOWN_STR,          //�������ͣ�δ֪�ַ���

	/*
	* ����2���� CPP_STEP1 �Ѿ�����ĵ��ʽ�һ��������ԣ���ϳɸ���һ���Ĵʷ���Ԫ
	*/
	CPP_STEP2_WORDS_UNKNOWN,                     //�ʷ���Ԫ��δ֪����
	CPP_STEP2_WORDS_COMMENT_SINGLE_LINE,         //�ʷ���Ԫ������ĵ���ע�͹ؼ��� "//"
	CPP_STEP2_WORDS_COMMENT_MULTI_LINE,          //�ʷ���Ԫ������Ķ���ע�͹ؼ��� "/*...*/"
	CPP_STEP2_WORDS_PREPROCESSOR_UNKNOWN,        //�ʷ���Ԫ��δ֪Ԥ�������� �� "#__has_include" ��ЩC++17�����Ԥ�����������Ϊ����
	CPP_STEP2_WORDS_PREPROCESSOR_IF,             //�ʷ���Ԫ��Ԥ�������� "#if"
	CPP_STEP2_WORDS_PREPROCESSOR_ELIF,           //�ʷ���Ԫ��Ԥ�������� "#elif"
	CPP_STEP2_WORDS_PREPROCESSOR_ELSE,           //�ʷ���Ԫ��Ԥ�������� "#else"
	CPP_STEP2_WORDS_PREPROCESSOR_ENDIF,          //�ʷ���Ԫ��Ԥ�������� "#endif"
	CPP_STEP2_WORDS_PREPROCESSOR_IFDEF,          //�ʷ���Ԫ��Ԥ�������� "#ifdef"
	CPP_STEP2_WORDS_PREPROCESSOR_IFNDEF,         //�ʷ���Ԫ��Ԥ�������� "#ifndef"
	CPP_STEP2_WORDS_PREPROCESSOR_DEFINE,         //�ʷ���Ԫ��Ԥ�������� "#define"
	CPP_STEP2_WORDS_PREPROCESSOR_UNDEF,          //�ʷ���Ԫ��Ԥ�������� "#undef"
	CPP_STEP2_WORDS_PREPROCESSOR_INCLUDE,        //�ʷ���Ԫ��Ԥ�������� "#include"
	CPP_STEP2_WORDS_PREPROCESSOR_LINE,           //�ʷ���Ԫ��Ԥ�������� "#line"
	CPP_STEP2_WORDS_PREPROCESSOR_ERROR,          //�ʷ���Ԫ��Ԥ�������� "#error"
	CPP_STEP2_WORDS_PREPROCESSOR_PRAGMA,         //�ʷ���Ԫ��Ԥ�������� "#pragma"
	CPP_STEP2_WORDS_PREPROCESSOR_DEFINED,        //�ʷ���Ԫ��Ԥ�������� "#defined"
	CPP_STEP2_KEYWORD_ALIGNAS,                   //�ʷ���Ԫ�� "alignas", //(C++11 ��)
	CPP_STEP2_KEYWORD_ALIGGNOF,                  //�ʷ���Ԫ�� "alignof", //(C++11 ��)
	CPP_STEP2_KEYWORD_AND,                       //�ʷ���Ԫ�� "and",
	CPP_STEP2_KEYWORD_AND_EQ,                    //�ʷ���Ԫ�� "and_eq",
	CPP_STEP2_KEYWORD_ASM,                       //�ʷ���Ԫ�� "asm",
	CPP_STEP2_KEYWORD_ATOMIC_CANCEL,             //�ʷ���Ԫ�� "atomic_cancel", //(TM TS)
	CPP_STEP2_KEYWORD_ATOMIC_COMMIT,             //�ʷ���Ԫ�� "atomic_commit", //(TM TS)
	CPP_STEP2_KEYWORD_ATOMIC_NOEXCEPT,           //�ʷ���Ԫ�� "atomic_noexcept", //(TM TS)
	CPP_STEP2_KEYWORD_AUTO,                      //�ʷ���Ԫ�� "auto",
	CPP_STEP2_KEYWORD_BITAND,                    //�ʷ���Ԫ�� "bitand",
	CPP_STEP2_KEYWORD_BITOR,                     //�ʷ���Ԫ�� "bitor",
	CPP_STEP2_KEYWORD_BOOL,                      //�ʷ���Ԫ�� "bool",
	CPP_STEP2_KEYWORD_BREAK,                     //�ʷ���Ԫ�� "break",
	CPP_STEP2_KEYWORD_CASE,                      //�ʷ���Ԫ�� "case",
	CPP_STEP2_KEYWORD_CATCH,                     //�ʷ���Ԫ�� "catch",
	CPP_STEP2_KEYWORD_CHAR,                      //�ʷ���Ԫ�� "char",
	CPP_STEP2_KEYWORD_CHAR8_T,                   //�ʷ���Ԫ�� "char8_t", //(C++20 ��)
	CPP_STEP2_KEYWORD_CHAR16_T,                  //�ʷ���Ԫ�� "char16_t", //(C++11 ��)
	CPP_STEP2_KEYWORD_CHAR32_T,                  //�ʷ���Ԫ�� "char32_t", //(C++11 ��)
	CPP_STEP2_KEYWORD_CLASS,                     //�ʷ���Ԫ�� "class",
	CPP_STEP2_KEYWORD_COMPL,                     //�ʷ���Ԫ�� "compl",
	CPP_STEP2_KEYWORD_CONCEPT,                   //�ʷ���Ԫ�� "concept", //(C++20 ��)
	CPP_STEP2_KEYWORD_CONST,                     //�ʷ���Ԫ�� "const",
	CPP_STEP2_KEYWORD_CONSTEVAL,                 //�ʷ���Ԫ�� "consteval", //(C++20 ��)
	CPP_STEP2_KEYWORD_CONSTEXPR,                 //�ʷ���Ԫ�� "constexpr", //(C++11 ��)
	CPP_STEP2_KEYWORD_CONSTINIT,                 //�ʷ���Ԫ�� "constinit", //(C++20 ��)
	CPP_STEP2_KEYWORD_CONST_CAST,                //�ʷ���Ԫ�� "const_cast",
	CPP_STEP2_KEYWORD_CONTINUE,                  //�ʷ���Ԫ�� "continue",
	CPP_STEP2_KEYWORD_CO_AWAIT,                  //�ʷ���Ԫ�� "co_await", //(C++20 ��)
	CPP_STEP2_KEYWORD_CO_RETURN,                 //�ʷ���Ԫ�� "co_return", //(C++20 ��)
	CPP_STEP2_KEYWORD_CO_YIELD,                  //�ʷ���Ԫ�� "co_yield", //(C++20 ��)
	CPP_STEP2_KEYWORD_DECLTYPE,                  //�ʷ���Ԫ�� "decltype", //(C++11 ��)
	CPP_STEP2_KEYWORD_DEFAULT,                   //�ʷ���Ԫ�� "default",
	CPP_STEP2_KEYWORD_DELETE,                    //�ʷ���Ԫ�� "delete",
	CPP_STEP2_KEYWORD_DO,                        //�ʷ���Ԫ�� "do",
	CPP_STEP2_KEYWORD_DOUBLE,                    //�ʷ���Ԫ�� "double",
	CPP_STEP2_KEYWORD_DYNAMIC_CAST,              //�ʷ���Ԫ�� "dynamic_cast",
	CPP_STEP2_KEYWORD_ELSE,                      //�ʷ���Ԫ�� "else",
	CPP_STEP2_KEYWORD_ENUM,                      //�ʷ���Ԫ�� "enum",
	CPP_STEP2_KEYWORD_EXPLICIT,                  //�ʷ���Ԫ�� "explicit",
	CPP_STEP2_KEYWORD_EXPORT,                    //�ʷ���Ԫ�� "export",
	CPP_STEP2_KEYWORD_EXTERN,                    //�ʷ���Ԫ�� "extern",
	CPP_STEP2_KEYWORD_FALSE,                     //�ʷ���Ԫ�� "false",
	CPP_STEP2_KEYWORD_FLOAT,                     //�ʷ���Ԫ�� "float",
	CPP_STEP2_KEYWORD_FOR,                       //�ʷ���Ԫ�� "for",
	CPP_STEP2_KEYWORD_FRIEND,                    //�ʷ���Ԫ�� "friend",
	CPP_STEP2_KEYWORD_GOTO,                      //�ʷ���Ԫ�� "goto",
	CPP_STEP2_KEYWORD_IF,                        //�ʷ���Ԫ�� "if",
	CPP_STEP2_KEYWORD_INLINE,                    //�ʷ���Ԫ�� "inline",
	CPP_STEP2_KEYWORD_INT,                       //�ʷ���Ԫ�� "int",
	CPP_STEP2_KEYWORD_LONG,                      //�ʷ���Ԫ�� "long",
	CPP_STEP2_KEYWORD_MUTABLE,                   //�ʷ���Ԫ�� "mutable",
	CPP_STEP2_KEYWORD_NAMESPACE,                 //�ʷ���Ԫ�� "namespace",
	CPP_STEP2_KEYWORD_NEW,                       //�ʷ���Ԫ�� "new",
	CPP_STEP2_KEYWORD_NOEXCEPT,                  //�ʷ���Ԫ�� "noexcept", //(C++11 ��)
	CPP_STEP2_KEYWORD_NOT,                       //�ʷ���Ԫ�� "not",
	CPP_STEP2_KEYWORD_NOT_EQ,                    //�ʷ���Ԫ�� "not_eq",
	CPP_STEP2_KEYWORD_NULLPTR,                   //�ʷ���Ԫ�� "nullptr", //(C++11 ��)
	CPP_STEP2_KEYWORD_OPERATOR,                  //�ʷ���Ԫ�� "operator",
	CPP_STEP2_KEYWORD_OR,                        //�ʷ���Ԫ�� "or",
	CPP_STEP2_KEYWORD_OR_EQ,                     //�ʷ���Ԫ�� "or_eq",
	CPP_STEP2_KEYWORD_PRIVATE,                   //�ʷ���Ԫ�� "private",
	CPP_STEP2_KEYWORD_PROTECTED,                 //�ʷ���Ԫ�� "protected",
	CPP_STEP2_KEYWORD_PUBLIC,                    //�ʷ���Ԫ�� "public",
	CPP_STEP2_KEYWORD_REFLEXPR,                  //�ʷ���Ԫ�� "reflexpr", //(���� TS)
	CPP_STEP2_KEYWORD_REGISTER,                  //�ʷ���Ԫ�� "register",
	CPP_STEP2_KEYWORD_REINTERPRET_CAST,          //�ʷ���Ԫ�� "reinterpret_cast",
	CPP_STEP2_KEYWORD_REQUIRES,                  //�ʷ���Ԫ�� "requires", //(C++20 ��)
	CPP_STEP2_KEYWORD_RETURN,                    //�ʷ���Ԫ�� "return",
	CPP_STEP2_KEYWORD_SHORT,                     //�ʷ���Ԫ�� "short",
	CPP_STEP2_KEYWORD_SIGNED,                    //�ʷ���Ԫ�� "signed",
	CPP_STEP2_KEYWORD_SIZEOF,                    //�ʷ���Ԫ�� "sizeof",
	CPP_STEP2_KEYWORD_STATIC,                    //�ʷ���Ԫ�� "static",
	CPP_STEP2_KEYWORD_STATIC_ASSERT,             //�ʷ���Ԫ�� "static_assert", //(C++11 ��)
	CPP_STEP2_KEYWORD_STATIC_CAST,               //�ʷ���Ԫ�� "static_cast",
	CPP_STEP2_KEYWORD_STRUCT,                    //�ʷ���Ԫ�� "struct",
	CPP_STEP2_KEYWORD_SWITCH,                    //�ʷ���Ԫ�� "switch",
	CPP_STEP2_KEYWORD_SYNCHRONIZED,              //�ʷ���Ԫ�� "synchronized", //(TM TS)
	CPP_STEP2_KEYWORD_TEMPLATE,                  //�ʷ���Ԫ�� "template",
	CPP_STEP2_KEYWORD_THIS,                      //�ʷ���Ԫ�� "this",
	CPP_STEP2_KEYWORD_THREAD_LOCAL,              //�ʷ���Ԫ�� "thread_local", //(C++11 ��)
	CPP_STEP2_KEYWORD_THROW,                     //�ʷ���Ԫ�� "throw",
	CPP_STEP2_KEYWORD_TRUE,                      //�ʷ���Ԫ�� "true",
	CPP_STEP2_KEYWORD_TRY,                       //�ʷ���Ԫ�� "try",
	CPP_STEP2_KEYWORD_TYPEDEF,                   //�ʷ���Ԫ�� "typedef",
	CPP_STEP2_KEYWORD_TYPEID,                    //�ʷ���Ԫ�� "typeid",
	CPP_STEP2_KEYWORD_TYPENAME,                  //�ʷ���Ԫ�� "typename",
	CPP_STEP2_KEYWORD_UNION,                     //�ʷ���Ԫ�� "union",
	CPP_STEP2_KEYWORD_UNSIGNED,                  //�ʷ���Ԫ�� "unsigned",
	CPP_STEP2_KEYWORD_USING,                     //�ʷ���Ԫ�� "using",
	CPP_STEP2_KEYWORD_VIRTUAL,                   //�ʷ���Ԫ�� "virtual",
	CPP_STEP2_KEYWORD_VOID,                      //�ʷ���Ԫ�� "void",
	CPP_STEP2_KEYWORD_VOLATILE,                  //�ʷ���Ԫ�� "volatile",
	CPP_STEP2_KEYWORD_WCHAR_T,                   //�ʷ���Ԫ�� "wchar_t",
	CPP_STEP2_KEYWORD_WHILE,                     //�ʷ���Ԫ�� "while",
	CPP_STEP2_KEYWORD_XOR,                       //�ʷ���Ԫ�� "xor",
	CPP_STEP2_KEYWORD_XOR_EQ,                    //�ʷ���Ԫ�� "xor_eq",
	CPP_STEP2_WORDS_OPERATOR_PAIR,               //�ʷ���Ԫ������Ե���������һ�飬���� "()","{}","[]","<>",...

	/*
	* �� CPP_STEP2 �Ļ����ϣ�������Ĵʷ���Ԫ�������������һ����������䣬
	* ����ÿ���ʷ���Ԫ���з���
	*/
	CPP_STEP3_STATEMENT_UNKNOWN,                       //�䷨��Ԫ��δ֪
	CPP_STEP3_STATEMENT_END_SYMBOL,                    //�䷨��Ԫ���������������� "int a;" �е�";"
	CPP_STEP3_STATEMENT_DECLARE_TYPE,                  //�䷨��Ԫ���������������ͣ����� "static unsigned long long"
	CPP_STEP3_STATEMENT_CLASS_NAME,                    //�䷨��Ԫ������
	CPP_STEP3_STATEMENT_STRUCT_NAME,                   //�䷨��Ԫ���ṹ����
	CPP_STEP3_STATEMENT_VAR_NAME,                      //�䷨��Ԫ��������
	CPP_STEP3_STATEMENT_FUNCTION_NAME,                 //�䷨��Ԫ��������
	CPP_STEP3_STATEMENT_ENUM_NAME,                     //�䷨��Ԫ��ö����
	CPP_STEP3_STATEMENT_UNION_NAME,                    //�䷨��Ԫ����������
	CPP_STEP3_STATEMENT_ARRAY_NAME,                    //�䷨��Ԫ��������
	CPP_STEP3_STATEMENT_VAR_DECLARE,                   //�䷨��Ԫ���������������� "int a;" �е�"a"
	CPP_STEP3_STATEMENT_CLASS_DECLARE,                 //�䷨��Ԫ��C++������������ "class A : public B {};"
	CPP_STEP3_STATEMENT_STRUCT_DECLARE,                //�䷨��Ԫ��C++�ṹ������������ "typedef struct _A_ {}A;"
	CPP_STEP3_STATEMENT_FUNCTION_DECLARE_RETURN,       //�䷨��Ԫ�����������ķ���ֵ������ "virtual int get() = 0;" �е�"virtual int"
	CPP_STEP3_STATEMENT_FUNCTION_DECLARE_NAME,         //�䷨��Ԫ�����������ĺ����������� "virtual int get() = 0;" �е�"get"
	CPP_STEP3_STATEMENT_FUNCTION_DECLARE_ARGS,         //�䷨��Ԫ�����������Ĳ����б����� "virtual int get() = 0;" �е�"()"
	CPP_STEP3_STATEMENT_FUNCTION_DECLARE_QUALIFIER,    //�䷨��Ԫ�������������޶����η������� "virtual int get() = 0;" �е�"= 0"
	CPP_STEP3_STATEMENT_FUNCTION_DEFINE_RETURN,        //�䷨��Ԫ����������ķ���ֵ������ "int A::get(){ return 0��}" �е�"int"
	CPP_STEP3_STATEMENT_FUNCTION_DEFINE_NAME,          //�䷨��Ԫ����������ĺ����������� "int A::get(){ return 0��}" �е�"A::get"
	CPP_STEP3_STATEMENT_FUNCTION_DEFINE_ARGS,          //�䷨��Ԫ����������Ĳ����б����� "int A::get(){ return 0��}" �е�"()"
	CPP_STEP3_STATEMENT_FUNCTION_DEFINE_BODY,          //�䷨��Ԫ������������壬���� "int A::get(){ return 0��}" �е�"{ return 0��}"
	CPP_STEP3_STATEMENT_IF_DEFINE,                     //�䷨��Ԫ��if�����ж϶��壬���� "if (a == 1){ return 0��}"
	CPP_STEP3_STATEMENT_ELSE_IF_DEFINE,                //�䷨��Ԫ��else if�����ж϶��壬���� "else if (a == 1){ return 0��}"
	CPP_STEP3_STATEMENT_ELSE_DEFINE,                   //�䷨��Ԫ��else�����ж϶��壬���� "else { return 0��}"
	CPP_STEP3_STATEMENT_FOR_DEFINE,                    //�䷨��Ԫ��forѭ�����壬���� "for (int i = 0; i < 3; ++i){ a += i��}"
	CPP_STEP3_STATEMENT_WHILE_DEFINE,                  //�䷨��Ԫ��while����ѭ�����壬���� "while (a < 3){ a += 1��}"
	CPP_STEP3_STATEMENT_DO_WHILE_DEFINE,               //�䷨��Ԫ��do while����ѭ�����壬���� "do { a += 1��} while (a < 3);"
	CPP_STEP3_STATEMENT_SWITCH_DEFINE,                 //�䷨��Ԫ��switch���壬���� "switch (a) { case 1: {break;} case 2: {break;} default: {break;} }"
	CPP_STEP3_STATEMENT_TRY_CATCH,                     //�䷨��Ԫ��try catch�쳣�������� "try { a += 1��} catch (int err){}"
	CPP_STEP3_STATEMENT_MACRO_NAME,                    //�䷨��Ԫ���궨������ƣ����� "#define PI 3.1415926" �е�"PI"
	CPP_STEP3_STATEMENT_MACRO_ARGS,                    //�䷨��Ԫ���궨��Ĳ����б����� "SUB(a, b)  (a - b)" �е�"(a, b)"
	CPP_STEP3_STATEMENT_MACRO_BODY,                    //�䷨��Ԫ���궨����壬���� "#define PI 3.1415926" �е�"3.1415926"

	/*
	* �� CPP_STEP3 �Ļ����ϣ���һ����������䣬���ͳɳ��������
	*/
	CPP_STEP4_SEMANTICS_UNKNOWN,                      //���嵥Ԫ��δ֪
	CPP_STEP4_SEMANTICS_NORMAL_VAR,                   //���嵥Ԫ����ͨ���������� "int a;"
	CPP_STEP4_SEMANTICS_POINTER_VAR,                  //���嵥Ԫ��ָ����������� "int *a = NULL;"
	CPP_STEP4_SEMANTICS_STATIC_VAR,                   //���嵥Ԫ����̬���������� "static int a = 0;"
	CPP_STEP4_SEMANTICS_GLOBAL_VAR,                   //���嵥Ԫ��ȫ�ֱ��������� "int g_a = 0;"
	CPP_STEP4_SEMANTICS_CLASS_MEMBER_VAR,             //���嵥Ԫ��C++���Ա���������� "class A {int m_a;};"
	CPP_STEP4_SEMANTICS_CLASS_MEMBER_FUNCTION,        //���嵥Ԫ��C++���Ա���������� "class A {int get();};"
	CPP_STEP4_SEMANTICS_FUNCTION_BODY_VAR,            //���嵥Ԫ���������ڲ������ı��������� "int get() {int a;}" �е�"a"
	CPP_STEP4_SEMANTICS_FUNCTION_BODY_FUNC_CALLED,    //���嵥Ԫ���������ڲ��ĺ������ã����� "int get() {printf("");}" �е�"printf"
	CPP_STEP4_SEMANTICS_FUNCTION_ARGS_VAR,            //���嵥Ԫ�����������б������ı��������� "int get(int a) {}" �е�"a"
	CPP_STEP4_SEMANTICS_CONDITION,                    //���嵥Ԫ�������жϷ�֧������ "if(){}else{}"
	CPP_STEP4_SEMANTICS_LOOP,                         //���嵥Ԫ��ѭ�������� "for(){}"
}CPP_STEP;


typedef struct _MY_STRING2_
{
	char str[256]; //�ַ���
}MY_STRING2;


typedef struct _STRING_POSITON2_
{
	unsigned char *start;
	unsigned char *end;
	int fileOffsetOfStart;
	int fileOffsetOfEnd;
	int lineNumberOfStart;
	int lineNumberOfEnd;
	int length;
	char str[1024];

public:
	int copyStrFromBuffer()
	{
		int ret = 0;
		if (start == NULL){ return -1; }

		if (str == NULL){ printf("Error: malloc() failed!\n"); return -1; }
		int len = length;
		if (len > sizeof(str) - 1)
		{
			len = sizeof(str) - 1;
		}

		if (len > 0)
		{
			memcpy(str, start, len);
			str[len] = '\0';
		}
		return 0;
	}
}STRING_POSITON2;


typedef struct _CPP_STEP_INDEX_
{
	CPP_STEP step; //�﷨������Ĺ���
	std::vector<int> indexes; //vStep1���±�����
}CPP_STEP_INDEX;


typedef struct _CPP_STEPS_
{
	std::vector<STRING_POSITON2> vStep1;
	std::vector<CPP_STEP_INDEX> vStep2;
	std::vector<CPP_STEP_INDEX> vStep3;
	std::vector<CPP_STEP_INDEX> vStep4;
}CPP_STEPS;


//---------C/C++Դ�����ļ��������ù�ϵ��-----------------
class CFuncRoute2
{
public:
	std::string m_srcCodesFilePath; //C/C++Դ�����ļ�·��
	std::vector<std::string> m_fileSuffixes; //C/C++Դ�����ļ���׺�������Դ�Сд�����飬���� [".h", ".hpp", ".c", ".cpp", ".cc", "*"]

public:
	CFuncRoute2();
	~CFuncRoute2();
};

#endif //__FUNC_ROUTE2_H__
