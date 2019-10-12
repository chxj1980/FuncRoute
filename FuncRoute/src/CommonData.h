#ifndef __COMMON_DATA_H__
#define __COMMON_DATA_H__

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stack>


typedef enum _VAR_TYPE_
{
	VAR_TYPE_UNKNOWN = 0, //δ֪����
	VAR_TYPE_NORMAL, //��������
	VAR_TYPE_POINTER, //ָ�����
	VAR_TYPE_STATIC1, //��̬���� static a = 0;
	VAR_TYPE_STATIC2, //��̬���� static A::m_a = 0;
}VAR_TYPE;


typedef struct _MY_STRING_
{
	char str[256]; //�ַ���
}MY_STRING;


typedef struct _STRING_POSITON_
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
	int printfInfo()
	{
		printf("-----STRING_POSITON----START---\n");
		printf("start: %p;\n", start);
		printf("end: %p;\n", end);
		printf("fileOffsetOfStart: %d;\n", fileOffsetOfStart);
		printf("fileOffsetOfEnd: %d;\n", fileOffsetOfEnd);
		printf("lineNumberOfStart: %d;\n", lineNumberOfStart);
		printf("lineNumberOfEnd: %d;\n", lineNumberOfEnd);
		printf("length: %d;\n", length);
		printf("str: ---%s---\n", str);
		printf("-----STRING_POSITON----END---\n");

		return 0;
	}
}STRING_POSITON;


//C++��/�ṹ���ʵ��������
typedef struct _CLASS_INSTANCE_
{
	STRING_POSITON className; //����
	STRING_POSITON classInstanceName; //���ʵ����������
	STRING_POSITON functionName; //���õ��������һ������
	STRING_POSITON functionArgs; //�����Ĳ���
	STRING_POSITON functionReturnValue; //��������ֵ
	VAR_TYPE instanceType; //���ʵ��������������
	int functionIndex; //������ţ�ȫ��Ψһ

public:
	int printfInfo()
	{
		printf("-----CLASS_INSTANCE----START--%d---\n", functionIndex);
		printf("functionIndex: %d;\n", functionIndex);
		printf("instanceType: %d;\n", instanceType);

		className.printfInfo();
		classInstanceName.printfInfo();
		functionName.printfInfo();
		functionArgs.printfInfo();
		functionReturnValue.printfInfo();
		printf("-----CLASS_INSTANCE----END--%d---\n", functionIndex);

		return 0;
	}
}CLASS_INSTANCE;


//C++��������
typedef struct _VAR_DECLARE_
{
	STRING_POSITON varType; //��������
	STRING_POSITON varName; //������
	bool isPointerVar; //�Ƿ���ָ�����������
	int varIndex; //�������

public:
	int printfInfo()
	{
		printf("-----VAR_DECLARE----START--%d---\n", varIndex);
		printf("varIndex: %d;\n", varIndex);
		printf("isPointerVar: %d;\n", isPointerVar);
		varType.printfInfo();
		varName.printfInfo();
		printf("-----VAR_DECLARE----END--%d---\n", varIndex);

		return 0;
	}
}VAR_DECLARE;


typedef struct _FUNCTION_STRUCTURE_
{
	STRING_POSITON functionReturnValueTypeQualifier; //��������ֵ(type-qualifier�����޶���)�� const��template, virtual, inline, static, extern, explicit, friend, constexpr
	STRING_POSITON functionReturnValue; //��������ֵ����(type-specifier�������ַ�): void *, void, int, short, long, float, double , auto, struct�ṹ�����ͣ�enumö�����ͣ�typedef����
	STRING_POSITON functionName; //������
	STRING_POSITON functionParameter; //��������
	STRING_POSITON functionTypeQualifier; //����������С���ź�����������η�(type-qualifier�����޶���)��=0, =default, =delete, const��voliate, &(��ֵ�����޶���), &&(��ֵ�����޶���), override, final, noexcept, throw
	STRING_POSITON functionBody; //������
	std::vector<CLASS_INSTANCE> funcsWhichInFunctionBody; //�������ڲ�����������Щ��������
//	std::map<int, int> funcsWhichCalledMe; //�ú�������Щ����(ʹ�ú�����ű�ʾ)�����ˣ����ܻᱻͬһ��������ε��ã�
	char className[200]; //�������ڵ�C++������
	char structName[200]; //�������ڵ�C++�ṹ������
	char classNameAlias[200]; //��/�ṹ��ı���
	char funcString[1024]; //��������ֵ + ������ + ��������
	int functionIndex; //������ţ�ȫ��Ψһ

public:
	int printfInfo()
	{
		printf("-----FUNCTION_STRUCTURE---START--%d---\n", functionIndex);
		functionReturnValueTypeQualifier.printfInfo();
		functionReturnValue.printfInfo();
		functionName.printfInfo();
		functionParameter.printfInfo();
		functionTypeQualifier.printfInfo();
		functionBody.printfInfo();

		int len = funcsWhichInFunctionBody.size();
		for (int i = 0; i < len; ++i)
		{
			printf("funcsWhichInFunctionBody[%d]:\n", i);
			funcsWhichInFunctionBody[i].printfInfo();
		}
		printf("className: %s;\n", className);
		printf("structName: %s;\n", structName);
		printf("classNameAlias: %s;\n", classNameAlias);
		printf("funcString: %s;\n", funcString);
		printf("functionIndex: %d;\n", functionIndex);
		printf("-----FUNCTION_STRUCTURE---END--%d---\n", functionIndex);

		return 0;
	}
}FUNCTION_STRUCTURE;


typedef struct _CLASS_STRUCT_
{
	STRING_POSITON className; //��/�ṹ����
	STRING_POSITON classNameAlias; //��/�ṹ��ı���
	STRING_POSITON classBody; //�����
	STRING_POSITON classParent; //����
	std::vector<MY_STRING> classParents; //�����ǣ����԰�������ĸ��ࣩ�����磺"class B : public A, public C{};"
	std::vector<VAR_DECLARE> memberVars; //��Ա����
	std::vector<FUNCTION_STRUCTURE> memberFuncs; //��Ա����
	bool isStruct; //�Ƿ��ǽṹ��

public:
	int printfInfo()
	{
		printf("-----CLASS_STRUCT---START--isStruct=%d---\n", isStruct);
		printf("isStruct: %d;\n", isStruct);
		className.printfInfo();
		classNameAlias.printfInfo();
		classBody.printfInfo();
		classParent.printfInfo();

		int len = classParents.size();
		for (int i = 0; i < len; ++i)
		{
			printf("classParents[%d]: %s;\n", i, classParents[i].str);
		}

		len = memberVars.size();
		for (int i = 0; i < len; ++i)
		{
			printf("memberVars[%d]:\n", i);
			memberVars[i].printfInfo();
		}

		len = memberFuncs.size();
		for (int i = 0; i < len; ++i)
		{
			printf("memberFuncs[%d]:\n", i);
			memberFuncs[i].printfInfo();
		}

		printf("-----CLASS_STRUCT---END--isStruct=%d---\n", isStruct);

		return 0;
	}
}CLASS_STRUCT;


typedef struct _FUNCTIONS_
{
	char fllename[600]; //�����ļ���
	std::vector<FUNCTION_STRUCTURE> funcs;
	std::vector<CLASS_STRUCT> classes; //�洢���ļ�����������ЩC++��/�ṹ��

public:
	int printfInfo()
	{
		printf("-----FUNCTIONS---START---fllename=%s--\n", fllename);
		printf("fllename: %s;\n", fllename);

		int len = funcs.size();
		for (int i = 0; i < len; ++i)
		{
			printf("funcs[%d]:\n", i);
			funcs[i].printfInfo();
		}

		len = classes.size();
		for (int i = 0; i < len; ++i)
		{
			printf("classes[%d]:\n", i);
			classes[i].printfInfo();
		}

		printf("-----FUNCTIONS---END---fllename=%s--\n", fllename);

		return 0;
	}
}FUNCTIONS;


typedef struct _FUNC_INDEX_
{
	int index1;
	int index2;
	int funcIndex;
	int treeDepth;
	int refCount;
	int lastI; //��Ҫ�������� pdf ʱ��Ҫ
	int lastJ; //��Ҫ�������� pdf ʱ��Ҫ
	int parentIndexTemp; //��Ҫ�������� pdf ʱ��Ҫ
	std::vector<_FUNC_INDEX_ *> parentIndexs; //ͬһ���������ܱ��ü�����������
	std::vector<_FUNC_INDEX_ *> childrenIndexs; //�ú�����������Щ���������������Լ������Լ���

public:
	_FUNC_INDEX_();
	~_FUNC_INDEX_();

	bool isRecursiveFunction(int funcIndex, std::string &strChain); //�Ƿ��ǵݹ麯����������A->A���Լ� A->B->A
	bool isRecursiveFunctionExplicitCalled(int funcIndex); //�Ƿ�����ʽ�ݹ麯������A->A������ A->B->A
	int freeMem();
	int printInfo();
	int printInfoFuncRoute(std::vector<_FUNC_INDEX_ *> &funcs);
}FUNC_INDEX;


typedef struct _FUNC_INDEX_POS_
{
	FUNC_INDEX * node;
	int colBase;
	int rowBase;
}FUNC_INDEX_POS;


typedef struct _FUNCS_CALLED_TREE_
{
	std::vector<FUNC_INDEX *> funcsIndexs;

public:
	int freeMem()
	{
		int len1 = funcsIndexs.size();
		for (int i = 0; i < len1; ++i)
		{
			int len2 = funcsIndexs[i]->childrenIndexs.size();
			for (int j = 0; j < len2; ++j)
			{
				funcsIndexs[i]->childrenIndexs[j]->freeMem();
			}
		}
		return 0;
	}
}FUNCS_CALLED_TREE;


typedef struct _MACRO_
{
	char macroName[256]; //����
	char macroArgs[256]; //���������б�
	char macroBody[1024]; //����

public:
	int printfInfo()
	{
		printf("-----MACRO---START----\n");
		printf("macroName: %s;\n", macroName);
		printf("macroArgs: %s;\n", macroArgs);
		printf("macroBody: %s;\n", macroBody);
		printf("-----MACRO---END----\n");

		return 0;
	}
}MACRO;

#endif //__COMMON_DATA_H__
