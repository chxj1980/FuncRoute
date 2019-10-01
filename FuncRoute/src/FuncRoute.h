#ifndef __FUNC_ROUTE_H__
#define __FUNC_ROUTE_H__

#include <string>
#include <vector>
#include <map>
#include <queue>


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
	unsigned char fllename[600]; //�����ļ���
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
	std::vector<_FUNC_INDEX_ *> parentIndexs; //ͬһ���������ܱ��ü�����������
	std::vector<_FUNC_INDEX_ *> childrenIndexs; //�ú�����������Щ���������������Լ������Լ���

public:
	bool isRecursiveFunction(int funcIndex) //�Ƿ��ǵݹ麯��
	{
		int len = parentIndexs.size();
		if(len == 0)
		{
			return false;
		}else
		{
			for(int i = 0; i < len; ++i)
			{
				if(parentIndexs[i]->funcIndex == funcIndex)
				{
					return true;
				}else
				{
					bool ret = parentIndexs[i]->isRecursiveFunction(funcIndex);
					if(ret == true)
					{
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	int freeMem()
	{
		int len1 = childrenIndexs.size();
		for(int i = 0; i < len1; ++i)
		{
			if(childrenIndexs[i])
			{
				childrenIndexs[i]->freeMem();
				free(childrenIndexs[i]);
				childrenIndexs[i] = NULL;
			}
		}
		return 0;
	}
	
	int printInfo()
	{
		int ret = 0;

		std::vector<_FUNC_INDEX_ *> stack;
		std::vector<int> stackParent;
		std::vector<int> stackDepth;

		//---------------------------------
		printf("==========\n");
		stack.push_back(this);
		stackParent.push_back(this->funcIndex);
		stackDepth.push_back(1);
		
		while (stack.empty() == false)
		{
			_FUNC_INDEX_ *node = stack.front();
			int parentIndex = stackParent.front();
			int depth = stackDepth.front();

			stack.erase(stack.begin());
			stackParent.erase(stackParent.begin());
			stackDepth.erase(stackDepth.begin());

			if(stack.empty() == true)
			{
				printf("%d\n", node->funcIndex); //����
			}else
			{
				//---------------------
				if (parentIndex == stackParent[0]) //��ͬ�ĸ��ڵ�
				{
					printf("%d-", node->funcIndex);
				} else //�ǹ�ͬ�ĸ��ڵ�
				{
					if (depth == stackDepth[0]) //������
					{
						printf("%d ", node->funcIndex);
					} else
					{
						printf("%d\n", node->funcIndex);
					}
				}
			}
			
			bool bRet = node->isRecursiveFunction(node->funcIndex);
			if(bRet == false) //���ǵݹ麯��
			{
				int len = node->childrenIndexs.size();
				for(int i = 0; i < len; ++i)
				{
					stack.push_back(node->childrenIndexs[i]);
					stackParent.push_back(node->funcIndex);
					stackDepth.push_back(depth + 1);
				}
			}
		}
		printf("\n");
		return 0;
	}

	int printInfoFuncRoute(std::vector<_FUNC_INDEX_ *> &funcs)
	{
		int ret = 0;
		
		int len1 = this->childrenIndexs.size();
		int len2 = funcs.size();
		bool bRet = this->isRecursiveFunction(this->funcIndex);

		if(len1 == 0 || bRet == true)
		{
			if(len2 != 0)
			{
				printf("[");
				for (int i = 0; i < len2; ++i)
				{
					if(i == len2 - 1)
					{
						printf("%d", funcs[i]->funcIndex);
					}else
					{
						printf("%d-", funcs[i]->funcIndex);
					}
				}
				printf("]\n");
			}else
			{
				printf("[%d]\n", this->funcIndex);
			}
		}else
		{
			if(bRet == false)
			{
				for (int i = 0; i < len1; ++i)
				{
					funcs.push_back(this->childrenIndexs[i]);
					this->childrenIndexs[i]->printInfoFuncRoute(funcs);
					funcs.pop_back();
				}
			}
		}

		return 0;
	}

}FUNC_INDEX;


typedef struct _FUNCS_CALLED_TREE_
{
	std::vector<FUNC_INDEX *> funcsIndexs;

public:
	int freeMem()
	{
		int len1 = funcsIndexs.size();
		for(int i = 0; i < len1; ++i)
		{
			int len2 = funcsIndexs[i]->childrenIndexs.size();
			for(int j = 0; j < len2; ++j)
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


//---------C/C++Դ�����ļ��������ù�ϵ��-----------------
class CFuncRoute
{
public:
	std::string m_srcCodesFilePath; //C/C++Դ�����ļ�·��
	std::vector<std::string> m_fileSuffixes; //C/C++Դ�����ļ���׺�������Դ�Сд�����飬���� [".h", ".hpp", ".c", ".cpp", ".cc", "*"]

public:
	CFuncRoute();
	~CFuncRoute();

	int CFuncRoute::splitDirsBySemicolon(std::string dirs, std::vector<std::string> &vecDirs); //�÷ֺ�';'�ֿ��ַ���
	int findAllFunctionsName(std::vector<std::string> dirsInclude, std::vector<std::string> fileDirsExclude, std::vector<std::string> suffixes); //��Դ�����ļ����棬��ȡ�����к�����
	int search_C_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C���Ժ�����
	int search_CPP_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions); //���ڴ�buffer�У�����C++���Ժ�����
	bool isKeyword(unsigned char *buffer, int bufferSize); //�ַ����Ƿ���C/C++���Թؼ���
	bool isKeywordVarType(unsigned char *buffer, int bufferSize); //�ַ����Ƿ���C/C++���Ա����ؼ���
	int findOverloadOperatorsBack(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, unsigned char *&leftCharPos); //�����Ƿ���C++�����������
	int replaceAllCodeCommentsBySpace(unsigned char *buffer, int bufferSize); //��������"//..."��"/*...*/"ע�͵��Ĵ����ÿո�' '����
	int replaceAllStrBySpace(unsigned char *buffer, int bufferSize); //��������˫����""�Ĵ����ÿո�' '����
	int replaceAllMacroDefineStrBySpace(unsigned char *buffer, int bufferSize); //������#define�궨���ÿո�' '����
	int findStr(unsigned char *buffer, int bufferSize, const char *str, int &pos); //���ڴ��У�����ָ�����ַ���
	int findAllMacros(std::vector<std::string> files, std::vector<MACRO> &macros); //�����д���Դ�ļ��У��ҵ����еĺ궨��
	int findAllClassAndStructDeclare(unsigned char *buffer, int bufferSize, std::vector<CLASS_STRUCT> &classes); //���ڴ��в�������������C++��/�ṹ������
	int findAllFuncsInFunctionBody(unsigned char *buffer, int bufferSize, std::vector<CLASS_INSTANCE> &funcsWhichInFunctionBody, unsigned char *bufferBase, int lineNumberBase); //���Һ������ڲ����õ�������������
	int findWholeFuncCalled(unsigned char *buffer, int bufferSize, unsigned char *parentheseLeft, CLASS_INSTANCE &classInstance, unsigned char *bufferBase, int lineNumberBase); //������С����'('��λ�ã�����һ�������ĺ�������
	int findWholeFuncDeclare(unsigned char *buffer, int bufferSize, unsigned char *parentheseLeft, FUNCTION_STRUCTURE &funcDeclare, unsigned char *bufferBase, int lineNumberBase); //������С����'('��λ�ã�����һ�������ĺ�������
	int findNextMacroDefine(unsigned char *buffer, int bufferSize, unsigned char *&leftPos, unsigned char *&rightCharPos); //���Ҳ�������һ��������#define������
	int findNextCodeComments(unsigned char *buffer, int bufferSize, unsigned char *&leftPos, unsigned char *&rightCharPos); //���Ҳ�������һ��������ע�����

	int skipWhiteSpaceForward(unsigned char *buffer, int bufferSize, unsigned char *leftPos, unsigned char *&rightPos, int &lineNumber); //ǰ�������հ��ַ�
	int skipWhiteSpaceBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos, int &lineNumber); //���������հ��ַ�
	int findPairCharForward(unsigned char *buffer, int bufferSize, unsigned char *leftCharPos, char leftChar, char rightChar, unsigned char *&rightCharPos); //ǰ���������ַ�������"{}","<>","()","[]","''"
	int findPairCharBack(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, char leftChar, char rightChar, unsigned char *&leftCharPos); //�����������ַ�������"{}","<>","()","[]","''"
	int findPairCharBackStop(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, char leftChar, char rightChar, char *stopChar, unsigned char *&leftCharPos); //�����������ַ�������ֹͣ���򷵻�ʧ�ܣ�����"{}","<>","()","[]","''"
	int findCharForward(unsigned char *buffer, int bufferSize, char ch, unsigned char *&rightCharPos); //ǰ�����ָ���ַ�
	int findCharForwardStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, unsigned char *&rightCharPos); //ǰ�����ָ���ַ�������ֹͣ���򷵻�ʧ��
	int findCharBack(unsigned char *buffer, int bufferSize, char ch, unsigned char *&leftCharPos); //�������ָ���ַ�
	int findCharsBackGreedy(unsigned char *buffer, int bufferSize, char *chars, unsigned char *&leftCharPos); //�������ָ����ĳ�����ַ���̰������ֱ���ҵ�Ϊֹ
	int findCharBackStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, unsigned char *&leftCharPos); //�������ָ���ַ�������ֹͣ���򷵻�ʧ��
	int findCharStrsBackStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, const char stopStrs[][20], int stopStrsSize, unsigned char *&leftCharPos); //�������ָ���ַ�������ֹͣ����ָ�����ַ����򷵻�ʧ��
	int findStrForward(unsigned char *buffer, int bufferSize, unsigned char *leftPos, unsigned char *&rightPos); //ǰ������ַ�����C++ �������ͱ��������������� + ��ĸ + �»���
	int findStrBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos); //��������ַ�����C++ �������ͱ��������������� + ��ĸ + �»���
	int findStrVarTypeBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos); //������ұ��������ַ�����C++ �������ͱ��������������� + ��ĸ + �»���
	int findStrCharsBackGreedy(unsigned char *buffer, int bufferSize, unsigned char *rightPos, char *chars, unsigned char *&leftPos); //��������ַ�������ָ�����ַ���̰������ֱ���ҵ�Ϊֹ��C++ �������ͱ��������������� + ��ĸ + �»���
	int findQueryStrBackStop(unsigned char *buffer, int bufferSize, unsigned char *rightPos, char *queryStr, char *stopChar, unsigned char *&leftPos); //�������ָ���ַ���������ֹͣ���򷵻�ʧ��
	int findQueryStrForwardStop(unsigned char *buffer, int bufferSize, unsigned char *leftPos, char *queryStr, char *stopChar, unsigned char *&rightPos); //ǰ�����ָ���ַ���������ֹͣ���򷵻�ʧ��
	int findVarDeclareForward(unsigned char *buffer, int bufferSize, std::string queryStr, std::string &varDeclareType); //ǰ����ұ�������������
	int findVarDeclareBack(unsigned char *buffer, int bufferSize, std::string queryStr, std::string &varDeclareType); //������ұ�������������
	bool isValidVarChar(char ch); //�Ƿ���һ����Ч�ı���������C++ �������ͱ��������������� + ��ĸ + �»���
	bool isWhiteSpace(char ch); //�Ƿ���һ���հ��ַ���' ', '\t', '\r', '\n'
	int replaceTwoMoreWhiteSpaceByOneSpace(unsigned char *buffer, int bufferSize); //����������Ŀհ��ַ���һ���ո����
	int statBufferLinesCount(unsigned char *buffer, int bufferSize); //ͳ�Ƹ����ڴ��л��з�'\n'�ĸ���
	
	int macroExpand(); //���궨��չ��
	bool isParentClass(std::string child, std::string parent, std::vector<FUNCTIONS> &vFunctions); //�ж�parent���Ƿ���child�ĸ���
	int splitParentsClass(unsigned char *buffer, int bufferSize, std::vector<MY_STRING> &classParents); //��ָ����ǣ����磺"class B : public A, public C{};"
	int updateParentClass(std::vector<FUNCTIONS> &vFunctions); //��Ҫ����������ϵĸ���̳�
	int findAllMemberVarsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase); //����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����
	int findAllMemberFuncsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase); //����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����
	bool isFunctionArgsMatch(std::string parameter, std::string functionArgs); //�����������Ĳ����б�ͺ����ĵ��ô���Ĳ����б��Ƿ�ƥ��
	int statAllFuns(std::vector<FUNCTIONS> &vFunctions); //ͳ�����к���֮��ĵ��ù�ϵ
	int createAllFunsCalledTree(std::vector<FUNCTIONS> &vFunctions, FUNCS_CALLED_TREE &trees); //�����������ù�ϵ�����
	int createFunsCalledTree(std::vector<FUNCTIONS> &vFunctions, int queryFuncIndex, FUNC_INDEX **&arry, int arryLen); //�����������ù�ϵ�����
	int getFuncsPos(std::vector<FUNCTIONS> &vFunctions, int queryFuncIndex, int &index1, int &index2); //�����������ù�ϵ�����
	int dumpBufferToFile(unsigned char *buffer, int bufferSize, char *filename); //���ڴ�����д�������ļ�
	int printInfo(std::vector<FUNCTIONS> &vFunctions);
};

#endif //__FUNC_ROUTE_H__
