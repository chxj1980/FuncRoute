#ifndef __FUNC_ROUTE_H__
#define __FUNC_ROUTE_H__

#include <string>
#include <vector>


typedef struct _BYTES_STREAM_
{
	unsigned char *buffer;
	unsigned int bufferSize;
	unsigned int offset;
}BYTES_STREAM;


typedef struct _STRING_POSITON_
{
	unsigned char *start;
	unsigned char *end;
	unsigned int fileOffsetOfStart;
	unsigned int fileOffsetOfEnd;
	unsigned int lineNumberOfStart;
	unsigned int lineNumberOfEnd;
	unsigned int length;
	char str[1024];
/*
public:
	_STRING_POSITON_()
	{
		start = NULL;
		end = NULL;
		lineNumberOfStart = 0;
		lineNumberOfEnd = 0;
		length = 0;
		str = NULL;
	}
	_STRING_POSITON_(const _STRING_POSITON_ &s)
	{
		*this = s;
	}
	~_STRING_POSITON_()
	{
		freeData();
	}
	_STRING_POSITON_ operator = (const _STRING_POSITON_ &s)
	{
		this->start = s.start;
		this->end = s.end;
		this->lineNumberOfStart = s.lineNumberOfStart;
		this->lineNumberOfEnd = s.lineNumberOfEnd;
		this->length = s.length;
		this->str = NULL;

		this->copyStrFromBuffer();

		return *this;
	}
	int freeData(){ if(str){free(str); str = NULL;} return 0; }
	int copyStrFromBuffer()
	{
		int ret = 0;
		if (start == NULL){ return -1; }
		ret = freeData();
		str = (char *)malloc(length + 1);
		if (str == NULL){ printf("Error: malloc() failed!\n"); return -1; }
		memcpy(str, start, length);
		str[length] = '\0';
		return 0;
	}*/
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
}STRING_POSITON;


typedef struct _FUNCTION_STRUCTURE_
{
	STRING_POSITON functionReturnValueTypeQualifier; //��������ֵ(type-qualifier�����޶���)�� const��template, virtual, inline, static, extern, explicit, friend, constexpr
	STRING_POSITON functionReturnValue; //��������ֵ����(type-specifier�������ַ�): void *, void, int, short, long, float, double , auto, struct�ṹ�����ͣ�enumö�����ͣ�typedef����
	STRING_POSITON functionName; //������
	STRING_POSITON functionParameter; //��������
	STRING_POSITON functionTypeQualifier; //����������С���ź�����������η�(type-qualifier�����޶���)��=0, =default, =delete, const��voliate, &(��ֵ�����޶���), &&(��ֵ�����޶���), override, final, noexcept, throw
	STRING_POSITON functionBody; //������
	std::vector<STRING_POSITON> funcsInFunctionBody; //�������ڲ�����������Щ��������
	std::vector<STRING_POSITON> funcsWhichCallMe; //�ú�������Щ����������
	char className[200]; //�������ڵ�C++������
	char structName[200]; //�������ڵ�C++�ṹ������
	char classNameAlias[200]; //��/�ṹ��ı���
	char funcString[1024]; //��������ֵ + ������ + ��������
/*
public:
	_FUNCTION_STRUCTURE_(){}
	~_FUNCTION_STRUCTURE_(){}
	_FUNCTION_STRUCTURE_(const _FUNCTION_STRUCTURE_ &s)
	{
		*this = s;
	}
	_FUNCTION_STRUCTURE_ operator = (const _FUNCTION_STRUCTURE_ &f)
	{
		this->functionReturnValueTypeQualifier = f.functionReturnValueTypeQualifier;
		this->functionReturnValue = f.functionReturnValue;
		this->functionName = f.functionName;
		this->functionParameter = f.functionParameter;
		this->functionTypeQualifier = f.functionTypeQualifier;
		this->functionBody = f.functionBody;

		memcpy(this->funcString, f.funcString, sizeof(funcString));

		return *this;
	}*/
}FUNCTION_STRUCTURE;


typedef struct _FUNCTIONS_
{
	unsigned char fllename[600]; //�����ļ���
	std::vector<FUNCTION_STRUCTURE> funcs;
/*
public:
	_FUNCTIONS_(){}
	~_FUNCTIONS_(){}
	_FUNCTIONS_ operator = (const _FUNCTIONS_ &f)
	{
		memcpy(this->fllename, f.fllename, sizeof(fllename));

		int len = f.funcs.size();
		for (int i = 0; i < len; ++i)
		{
			this->funcs.push_back(f.funcs[i]);
		}

		return *this;
	}*/
}FUNCTIONS;


typedef struct _CLASS_STRUCT_
{
	STRING_POSITON className; //��/�ṹ����
	STRING_POSITON classNameAlias; //��/�ṹ��ı���
	STRING_POSITON classBody; //�����
	STRING_POSITON classParent; //����
	bool isStruct; //�Ƿ��ǽṹ��
}CLASS_STRUCT;


typedef struct _MACRO_
{
	char macroName[256]; //����
	char macroArgs[256]; //���������б�
	char macroBody[1024]; //����
}MACRO;


//---------C/C++Դ�����ļ��������ù�ϵ��-----------------
class CFuncRoute
{
public:
	std::string m_srcCodesFilePath; //C/C++Դ�����ļ�·��
	std::vector<std::string> m_fileSuffixes; //C/C++Դ�����ļ���׺�������Դ�Сд�����飬���� [".h", ".hpp", ".c", ".cpp", ".cc", "*"]

public:
	CFuncRoute();
	~CFuncRoute();

	int findAllFunctionsName(std::string filePath, std::vector<std::string> suffixes); //��Դ�����ļ����棬��ȡ�����к�����
	int search_C_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C���Ժ�����
	int search_CPP_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C++���Ժ�����
	bool isKeyword(unsigned char *buffer, int bufferSize); //�ַ����Ƿ���C/C++���Թػ���
	int replaceAllCodeCommentsBySpace(unsigned char *buffer, int bufferSize); //��������"//..."��"/*...*/"ע�͵��Ĵ����ÿո�' '����
	int findStr(unsigned char *buffer, int bufferSize, const char *str, int &pos); //���ڴ��У�����ָ�����ַ���
	int findAllMacros(std::vector<std::string> files, std::vector<MACRO> &macros); //�����д���Դ�ļ��У��ҵ����еĺ궨��
	int macroExpand(); //���궨��չ��
};

#endif //__FUNC_ROUTE_H__
