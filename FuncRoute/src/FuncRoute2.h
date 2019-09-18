#ifndef __FUNC_ROUTE2_H__
#define __FUNC_ROUTE2_H__

#include <string>
#include <vector>


typedef enum _CPP_STEP_
{
	/*
	* C/C++�﷨Ԫ��
	*/
	CPP_STEP0_UNKNOWN = 0,    //δ֪�﷨Ԫ��

	/*
	* �������� ����+��ĸ+�»��� ��ɵĵ���
	*/
	CPP_STEP1_WORD_UNKNOWN,              //�������ͣ�δ֪��������� +,-,*,/,,%,!,?,~,=,==,<,<=,=>,>,<<,>>,&,|,,||,^,//,/*,*/,",',#,;,:,::,[,],...��Щ �� ����+��ĸ+�»��� ����Ϊ����
	CPP_STEP1_WORD_KEYWORD,              //�������ͣ�C/C++�ؼ��� char,int,short,long,float,double,unsigned,if,else,for,do,while,private,protected,public,new,delete,class,struct,...
	CPP_STEP1_WORD_UNKNOWN_NAME,         //�������ͣ�δ֪����
	CPP_STEP1_WORD_CLASS_NAME,           //�������ͣ�����
	CPP_STEP1_WORD_STRUCT_NAME,          //�������ͣ��ṹ����
	CPP_STEP1_WORD_VAR_NAME,             //�������ͣ�������
	CPP_STEP1_WORD_FUNCTION_NAME,        //�������ͣ�������
	CPP_STEP1_WORD_ENUM_NAME,            //�������ͣ�ö����
	CPP_STEP1_WORD_UNION_NAME,           //�������ͣ���������
	CPP_STEP1_WORD_ARRAY_NAME,           //�������ͣ�������

	/*
	* �� CPP_STEP1 �Ѿ�����ĵ��ʽ�һ��������ԣ���ϳɸ���һ���Ĵʷ���Ԫ
	*/
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
	CPP_STEP2_WORDS_KEYWORDS,                    //�ʷ���Ԫ�����������������ϵĹؼ������һ�飬���� "static unsigned long long"
	CPP_STEP2_WORDS_OPERATOR_PAIR,               //�ʷ���Ԫ������Ե���������һ�飬���� "()","{}","[]","<>",...

	/*
	* �� CPP_STEP2 �Ļ����ϣ�������Ĵʷ���Ԫ�������������һ����������䣬���� "int a;","int a = 0;","class A {};","int A::get(){}"����ϳɸ���һ���ľ䷨��Ԫ
	*/
	CPP_STEP3_STATEMENT_UNKNOWN,                       //�䷨��Ԫ��δ֪
	CPP_STEP3_STATEMENT_END_SYMBOL,                    //�䷨��Ԫ���������������� "int a;" �е�";"
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
