#ifndef __FUNC_ROUTE_H__
#define __FUNC_ROUTE_H__

#include <string>
#include <vector>
#include <map>


typedef struct _MY_STRING_
{
	char str[256]; //�ַ���
}MY_STRING;


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
}STRING_POSITON;


//C++��/�ṹ���ʵ��������
typedef struct _CLASS_INSTANCE_
{
	STRING_POSITON className; //����
	STRING_POSITON classInstanceName; //���ʵ����������
	STRING_POSITON functionName; //���õ��������һ������
	STRING_POSITON functionArgs; //�����Ĳ���
	STRING_POSITON functionReturnValue; //��������ֵ
	int functionIndex; //������ţ�ȫ��Ψһ
}CLASS_INSTANCE;


//C++��������
typedef struct _VAR_DECLARE_
{
	STRING_POSITON varType; //��������
	STRING_POSITON varName; //������
	int varIndex; //�������
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
}FUNCTION_STRUCTURE;


typedef struct _CLASS_STRUCT_
{
	STRING_POSITON className; //��/�ṹ����
	STRING_POSITON classNameAlias; //��/�ṹ��ı���
	STRING_POSITON classBody; //�����
	STRING_POSITON classParent; //����
	std::vector<MY_STRING> classParents; //�����ǣ����԰�������ĸ��ࣩ�����磺"class B : public A, public C{};"
	std::vector<VAR_DECLARE> memberVars; //��Ա����
	std::vector<FUNCTION_STRUCTURE> memberFunc; //��Ա����
	bool isStruct; //�Ƿ��ǽṹ��
}CLASS_STRUCT;


typedef struct _FUNCTIONS_
{
	unsigned char fllename[600]; //�����ļ���
	std::vector<FUNCTION_STRUCTURE> funcs;
	std::vector<CLASS_STRUCT> classes; //�洢���ļ�����������ЩC++��/�ṹ��
}FUNCTIONS;


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
	bool isKeyword(unsigned char *buffer, int bufferSize); //�ַ����Ƿ���C/C++���Թؼ���
	int replaceAllCodeCommentsBySpace(unsigned char *buffer, int bufferSize); //��������"//..."��"/*...*/"ע�͵��Ĵ����ÿո�' '����
	int findStr(unsigned char *buffer, int bufferSize, const char *str, int &pos); //���ڴ��У�����ָ�����ַ���
	int findAllMacros(std::vector<std::string> files, std::vector<MACRO> &macros); //�����д���Դ�ļ��У��ҵ����еĺ궨��
	int findAllFuncsInFunctionBody(unsigned char *buffer, int bufferSize, std::vector<CLASS_INSTANCE> &funcsWhichInFunctionBody, unsigned char *bufferBase, int lineNumberBase); //���Һ������ڲ����õ�������������
	int macroExpand(); //���궨��չ��
	bool isParentClass(std::string child, std::string parent, std::vector<FUNCTIONS> &vFunctions); //�ж�parent���Ƿ���child�ĸ���
	int splitParentsClass(unsigned char *buffer, int bufferSize, std::vector<MY_STRING> &classParents); //��ָ����ǣ����磺"class B : public A, public C{};"
	int updateParentClass(std::vector<FUNCTIONS> &vFunctions); //��Ҫ����������ϵĸ���̳�
	int findAllMemberVarsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase); //����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����
	bool isFunctionArgsMatch(std::string parameter, std::string functionArgs); //�����������Ĳ����б�ͺ����ĵ��ô���Ĳ����б��Ƿ�ƥ��
	int statAllFuns(std::vector<FUNCTIONS> &vFunctions); //ͳ�����к���֮��ĵ��ù�ϵ
};

#endif //__FUNC_ROUTE_H__
