#include "FuncRoute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./os/share_library.h"
#include "CPPKeyword.h"
#include "CKeyword.h"


#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define ROUND(x) ((int) ((x) + 0.5))

#define RETURN_IF_FAILED(condition, ret)                                                      \
    do                                                                                        \
    {                                                                                         \
        if (condition)                                                                        \
        {                                                                                     \
            printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);    \
            return ret;                                                                       \
        }                                                                                     \
    } while (0)


//#define A(x)    #@x         //�Ե��ַ��ӵ�����,���磺A(x) ��ʾ 'x',A(abcd)����Ч
//#define B(x)    #x          //��˫���ţ�����xת�����ַ��������磺B(hello)����ʾ "hello"
//#define C(x)    hello##x    //�ѱ�ʶ���������������罺ˮһ��������������ճ���������磺C(_world)����ʾhello_world


//-----------------------------
CFuncRoute::CFuncRoute()
{

}


CFuncRoute::~CFuncRoute()
{

}


int CFuncRoute::findAllFunctionsName(std::string filePath, std::vector<std::string> suffixes)
{
	int ret = 0;
	std::vector<std::string> files;
	std::vector<std::string> files2;

	ret = get_nested_dir_files(filePath.c_str(), files);
	RETURN_IF_FAILED(ret != 0, ret);

	//---------------------------
	int len1 = files.size();
	RETURN_IF_FAILED(len1 <= 0, -1);

	int len2 = suffixes.size();
	RETURN_IF_FAILED(len2 <= 0, -2);

	bool isShouldFilterFiles = true; //�Ƿ���Ҫ���պ�׺�������ļ���
	for (int j = 0; j < len2; ++j)
	{
		if (suffixes[j] == "*") //�Ǻ�"*"��ʾƥ�������ļ�
		{
			isShouldFilterFiles = false;
			break;
		}
	}

	if (isShouldFilterFiles == true)
	{
		for (int i = 0; i < len1; ++i)
		{
			for (int j = 0; j < len2; ++j)
			{
				if (suffixes[j] != "*")
				{
					int len21 = files[i].length();
					int len22 = suffixes[j].length();

					std::size_t pos = files[i].rfind(".");
					if (pos != std::string::npos)
					{
						if (files[i].substr(pos) == suffixes[j])
						{
							files2.push_back(files[i]);
						}
					}
				}
			}
		}
	}
	else
	{
		files2 = files;
	}

	//------------------------
	int len3 = files2.size();
	RETURN_IF_FAILED(len3 <= 0, -3);
	
	//---------�������ļ�����궨����ȡ����---------------
	std::vector<MACRO> macros;
	ret = findAllMacros(files2, macros);
	RETURN_IF_FAILED(ret, -3);

	//---------��ȡÿ���ļ�����ĺ�������-------------------
	std::vector<FUNCTIONS> allFuncs;

	for (int i = 0; i < len3; ++i)
	{
		printf("%s: %s\n", __FUNCTION__, files2[i].c_str());

		//-----------��ȡ�����ļ����ڴ���---------------
		FILE * fp = fopen(files2[i].c_str(), "rb");
		RETURN_IF_FAILED(fp == NULL, -4);

		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char)* file_size);
		RETURN_IF_FAILED(buffer == NULL, -5);

		size_t read_size = fread(buffer, file_size, 1, fp);
		if (read_size != 1)
		{
			printf("%s: Error: read_size=%d != 1\n", __FUNCTION__, read_size);
			fclose(fp);
			free(buffer);
			return -6;
		}

		fclose(fp);

		//---------------------
		FUNCTIONS functions;
		memset(&functions, 0, sizeof(FUNCTIONS));

		int len = MIN(files2[i].length(), sizeof(functions.fllename) - 1);

		memcpy(functions.fllename, files2[i].c_str(), len);
		functions.fllename[len] = '\0';

		ret = search_CPP_FuncName(buffer, file_size, functions);
		free(buffer);

		int len2 = functions.funcs.size();
		if (ret != 0 && len2 <= 0)
		{
			printf("[%d] %s; Warn: can not find any functions;\n", i, functions.fllename);
			continue;
		}

		allFuncs.push_back(functions);

		//-----------------
		for (int i = 0; i < len2; ++i)
		{
			printf("[%d/%d] %s; line=%d;\n", i + 1, len2, functions.funcs[i].funcString, functions.funcs[i].functionName.lineNumberOfStart);
		}
	}

	//---------������������֮��ĵ��ù�ϵ--------------
	ret = statAllFuns(allFuncs);

	return ret;
}


int CFuncRoute::search_C_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions)
{
	int ret = 0;

	return 0;
}


int CFuncRoute::search_CPP_FuncName(unsigned char *buffer, unsigned int bufferSize, FUNCTIONS &functions)
{
	int ret = 0;

	RETURN_IF_FAILED(buffer == NULL, -1);

	char whiteSpace[] = {' ', '\t', '\r', '\n'}; //�հ��ַ�
	char scopeResolutionOperator[] = "::"; //C++ �������޶���
	char varName[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"; //C++ ���������������� + ��ĸ + �»���

	FUNCTION_STRUCTURE funcStruct;

	memset(&funcStruct, 0, sizeof(FUNCTION_STRUCTURE));

	unsigned char *buffer2 = (unsigned char *)malloc(bufferSize);
	RETURN_IF_FAILED(buffer2 == NULL, -2);

	memcpy(buffer2, buffer, bufferSize); //����һ��

	unsigned char *p1 = buffer2;
	unsigned char *p2 = buffer2;
	unsigned char *p3 = buffer2 + bufferSize - 1;
	unsigned char *p11 = NULL;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	unsigned char *p23 = NULL;
	int lineNumber = 1;
	int lineNumberTemp = 1;
	int braceCount = 0; //�����ŶԼ���
	int parenthesesCount = 0; //С���ŶԼ���
	int lenFuncString = sizeof(funcStruct.funcString);
	bool ret2 = false;

	int wordCountMax = 5; //������ǰ�棬�������5���Կո�������ַ���
	int wordCount = 0;
	int overSerach = 0; //ֹͣ����
	int lineCntMax = 5; //������ǰ�棬�������5��
	int lineCnt = 0;
	int len = 0;

	//------�Ƚ���ע�͵��Ĵ����ÿո�' '���棨�кŷ�'\n'������--------
	ret = replaceAllCodeCommentsBySpace(buffer2, bufferSize);
	RETURN_IF_FAILED(ret, -2);
	
//	static int cnt = 0;
//	char file22[600] = "";
//	sprintf(file22, "./out%d.txt", cnt++);
//	ret = dumpBufferToFile(buffer2, bufferSize, file22);

	//------��������˫����""�Ĵ����ÿո�' '����--------
	ret = replaceAllStrBySpace(buffer2, bufferSize);
	RETURN_IF_FAILED(ret, -2);

//	sprintf(file22, "./out%d.txt", cnt++);
//	ret = dumpBufferToFile(buffer2, bufferSize, file22);

	//------�������е�C++����(�ؼ���class, struct)--------
	char classKeyword[] = "class";
	char structKeyword[] = "struct";
	int classLen = strlen(classKeyword);
	int structLen = strlen(structKeyword);
	bool isStruct = false;
	bool isDestructor = false; //�Ƿ�����������
	bool isConstructorArgs = false; //�Ƿ��Ǵ������б�Ĺ��캯��
	bool isFuncNameWhithClassName = false; //���������Ƿ������������ int A::get(){}

	//--------���ҹؼ���class/struct----------------
	ret = findAllClassAndStructDeclare(p1, p3 - p1 + 1, functions.classes);

	//---------------------------
	p2 = buffer2;
	p21 = p2;
	p22 = NULL;

retry:
	while (p21 <= p3)
	{
		memset(&funcStruct, 0, sizeof(FUNCTION_STRUCTURE));

		//--------���Һ��������Ҵ�����"{}"----------------
		ret = findCharForward(p21, p3 - p21 + 1, '{', p22);
		if (ret != 0)
		{
			break;
		}

		lineNumber += statBufferLinesCount(p21, p22 - p21 + 1);
		
		funcStruct.functionBody.start = p22;
		funcStruct.functionBody.fileOffsetOfStart = funcStruct.functionBody.start - p1;
		funcStruct.functionBody.lineNumberOfStart = lineNumber;

		ret = findPairCharForward(p22, p3 - p21 + 1, p22, '{', '}', p22);
		if (ret != 0)
		{
			break;
		}

		funcStruct.functionBody.end = p22;
		funcStruct.functionBody.fileOffsetOfEnd = funcStruct.functionBody.end - p1;
		lineNumber += statBufferLinesCount(funcStruct.functionBody.start, funcStruct.functionBody.end - funcStruct.functionBody.start + 1);
		funcStruct.functionBody.lineNumberOfEnd = lineNumber;

		//---�������Ŷ������Ƿ��Ǻ궨��----
		ret = findQueryStrBackStop(p1, funcStruct.functionBody.start - p1 + 1, funcStruct.functionBody.start, "#", "\n", p21);
		if (ret == 0)
		{
			goto retry4; //˵���ǵ��к궨�壬�����²���
		}
		
		p22 = funcStruct.functionBody.start;
		ret = findCharForwardStop(p22, funcStruct.functionBody.end - p22 + 1, '\\', "\n", p21);
		if (ret == 0)
		{
			goto retry4; //���������� "#define AAAA \\ " �������������Ǻ������壬������
		}

		//--------���Һ�����������С���Ŷ�"()"----------------
		p21 = funcStruct.functionBody.start - 1;

		lineNumber = funcStruct.functionBody.lineNumberOfStart;
		ret = skipWhiteSpaceBack(p1, funcStruct.functionBody.start - p21 + 1, p21, p21, lineNumber);
		RETURN_IF_FAILED(ret, ret);
		
		ret = findCharBackStop(p1, p21 - p1 + 1, ')', "{}", p21);
		if (ret != 0)
		{
			goto retry4; //δ�ҵ�����������С����')'
		}

retry0:
		funcStruct.functionParameter.end = p21;
		funcStruct.functionParameter.fileOffsetOfEnd = funcStruct.functionParameter.end - p1;
		funcStruct.functionParameter.lineNumberOfEnd = lineNumber;

		ret = findPairCharBackStop(p1, p21 - p1 + 1, p21, '(', ')', ";{}", p21);
		if (ret != 0)
		{
			goto retry4; //���������� "struct A {};" �������������Ǻ������壬������
		}

		funcStruct.functionParameter.start = p21;
		funcStruct.functionParameter.fileOffsetOfStart = funcStruct.functionParameter.end - p1;

		lineNumber -= statBufferLinesCount(funcStruct.functionParameter.start, funcStruct.functionParameter.length);
		funcStruct.functionParameter.lineNumberOfStart = lineNumber;

		//-----���Բ��Һ��������б���С���ź����C/C++���η�(type-qualifier�����޶���)--------
		p21 = funcStruct.functionParameter.end + 1;
		lineNumber = funcStruct.functionParameter.lineNumberOfEnd;

		ret = skipWhiteSpaceForward(p21, funcStruct.functionBody.start - p21 + 1, p21, p21, lineNumber);
		RETURN_IF_FAILED(ret, ret);

		funcStruct.functionTypeQualifier.start = p21;
		funcStruct.functionTypeQualifier.fileOffsetOfStart = funcStruct.functionTypeQualifier.start - p1;
		funcStruct.functionTypeQualifier.lineNumberOfStart = lineNumber;

		p22 = funcStruct.functionBody.start - 1;
		ret = skipWhiteSpaceBack(p21, p22 - p21 + 1, p22, p22, lineNumber);
		if (ret != 0)
		{
			goto retry1; //˵��û�����η���������
		}

		funcStruct.functionTypeQualifier.end = p22;
		funcStruct.functionTypeQualifier.fileOffsetOfEnd = funcStruct.functionTypeQualifier.end - p1;
		funcStruct.functionTypeQualifier.lineNumberOfEnd = lineNumber;

		if(funcStruct.functionTypeQualifier.end - funcStruct.functionTypeQualifier.start + 1 > 1)
		{
			ret = findCharForward(funcStruct.functionTypeQualifier.start, funcStruct.functionTypeQualifier.end - funcStruct.functionTypeQualifier.start + 1, '#', p22);
			if(ret == 0) //FIXME: Ŀǰֻ֧�� ���� int get() const { return 0; }
			{
				goto retry4;
			}
		}

		if (funcStruct.functionTypeQualifier.start == funcStruct.functionTypeQualifier.end)
		{
			memset(&funcStruct.functionTypeQualifier, 0, sizeof(funcStruct.functionTypeQualifier));
		}

retry1:
		//--------���Һ�����----------------
		lineNumber = funcStruct.functionParameter.lineNumberOfStart;
		p21 = funcStruct.functionParameter.start - 1;

		ret = skipWhiteSpaceBack(p1, p21 - p1 + 1, p21, p21, lineNumber);
		RETURN_IF_FAILED(ret, ret);

		funcStruct.functionName.end = p21;
		funcStruct.functionName.fileOffsetOfEnd = funcStruct.functionName.end - p1;
		funcStruct.functionName.lineNumberOfEnd = lineNumber;

		ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
		RETURN_IF_FAILED(ret, ret);

		//---�����Բ���C++ ���������޶���"::"------
		ret = findQueryStrBackStop(p1, p21 - p1 + 1, p21, "::", ";{}()", p21); //���� int A::B::get() { return 0; }
		if(ret == 0)
		{
			p21--;

			ret = skipWhiteSpaceBack(p1, p21 - p1 + 1, p21, p21, lineNumber);
			RETURN_IF_FAILED(ret, ret);

			ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
			RETURN_IF_FAILED(ret, ret);

			isFuncNameWhithClassName = true; //���������Ƿ������������ int A::get(){}
		}

		ret = findCharBackStop(p1, p21 - p1 + 1, '~', ";{}()", p21); //���Բ����������������� ~A::A(){}
		if (ret == 0)
		{
			isDestructor = true;
		}

		funcStruct.functionName.start = p21;
		funcStruct.functionName.fileOffsetOfStart = funcStruct.functionName.start - p1;
		funcStruct.functionName.lineNumberOfStart = lineNumber;
		
		//---�ж��Ƿ��Ǵ���ʼ���б�Ĺ��캯�������� "class A {public: int m_a; int m_b; public: A::A():m_a(1),m_b(0){}};"
		if(isFuncNameWhithClassName == false)
		{
			ret = findCharBackStop(p1, p21 - p1 + 1, ':', ";{}", p21);
			if (ret == 0)
			{
				p21--;
				ret = skipWhiteSpaceBack(p1, p21 - p1 + 1, p21, p21, lineNumber);
				RETURN_IF_FAILED(ret, ret);

				if(*p21 == ')') //˵���Ǵ���ʼ���б�Ĺ��캯��
				{
					goto retry0;
				}
			}
		}

		//-----��麯�����Ƿ���C++�ؼ��ʣ����� if (a == 1) {} --------
		ret2 = isKeyword(funcStruct.functionName.start, funcStruct.functionName.end - funcStruct.functionName.start + 1);
		if (ret2) //��������C/C++���Թؼ���
		{
			goto retry4;
		}

		//--------���Һ�����ֵ----------------
		if (isDestructor == true)
		{
			goto retry3; //��������û�з���ֵ
		}

		p21 = funcStruct.functionName.start - 1;

		ret = skipWhiteSpaceBack(p1, p21 - p1 + 1, p21, p21, lineNumber);
		if (ret != 0)
		{
			goto retry3; //���캯��������������û�з���ֵ
		}

		funcStruct.functionReturnValue.end = p21;
		funcStruct.functionReturnValue.fileOffsetOfEnd = funcStruct.functionReturnValue.end - p1;
		funcStruct.functionReturnValue.lineNumberOfEnd = lineNumber;

		if(*p21 == '*') //��������ֵ������һ����ַָ��
		{
			p21--;
		}

		// ��������ֵ(type-qualifier�����޶���)�� const��template, virtual, inline, static, extern, explicit, friend, constexpr
		// ��������ֵ����(type - specifier�������ַ�) : void *, void, int, short, long, float, double, auto, struct�ṹ�����ͣ�enumö�����ͣ�typedef����, uint32_t
		// ����: inline const unsigned long long * get(int &a) const { return 0; }
		
		wordCountMax = 5; //������ǰ�棬�������5���Կո�������ַ���
		wordCount = 0;
		overSerach = 0; //ֹͣ����
		lineCntMax = 5; //������ǰ�棬�������5��
		lineCnt = 0;
		p22 = p21; //����p21ֵ

retry2:
		ret = skipWhiteSpaceBack(p1, p21 - p1 + 1, p21, p21, lineNumber);

		while (p21 > p1)
		{
			if ((*p21 >= '0' && *p21 <= '9')
				|| (*p21 >= 'a' && *p21 <= 'z')
				|| (*p21 >= 'A' && *p21 <= 'Z')
				|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
				|| (*p21 == '<' || *p21 == '>') //C++ ģ�� template <typename T> class A {};
				|| (*p21 == '\r' || *p21 == '\n') //����ֵ��ɢ�ڶ���
				)
			{

			}
			else
			{
				if (p21 == p1 //�ѵ���ʼλ����
					|| *p21 == '{' //���ܺ�������C++����ڲ�
					|| *p21 == '}' //��������һ������������λ��
					|| *p21 == ';' //������
					|| *p21 == ')' //���� #pragma comment(lib, "user32.lib")
					|| *p21 == '\\' //���� '#define  MACRO_A \'
					|| *p21 == ':' //���� class A { public: A(){} };
					|| *p21 == '"' //���� '#include "FuncRoute.h" int main() { return 0; };'
					|| *p21 == '`' //�����ַ���������C++�淶������Ϊ���������滻�ַ���
					)
				{
					overSerach = 1; //ֹͣ����

					p21++;
					while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
					{
						if (*p21 == '\n')
						{
							lineNumber++;
						}
						p21++;
					}
				}
				else if (*p21 == '\n')
				{
					lineCnt++;
				}
				break;
			}

			p21--;
		}

		if (p21 < p1)
		{
			ret = -7;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		wordCount++;
		if (p21 != p1 && overSerach == 0 && wordCount < wordCountMax && lineCnt < lineCntMax)
		{
			goto retry2;
		}

		funcStruct.functionReturnValue.start = p21;
		funcStruct.functionReturnValue.fileOffsetOfStart = funcStruct.functionReturnValue.start - p1;
		funcStruct.functionReturnValue.lineNumberOfStart = lineNumber;

		funcStruct.functionReturnValue.length = funcStruct.functionReturnValue.end - funcStruct.functionReturnValue.start + 1;
		funcStruct.functionReturnValue.copyStrFromBuffer();

retry3:
		//--------���ҵ�һ�������ĺ���----------------
		funcStruct.functionReturnValue.length = (funcStruct.functionReturnValue.end && funcStruct.functionReturnValue.start) ? (funcStruct.functionReturnValue.end - funcStruct.functionReturnValue.start + 1) : 0;
		funcStruct.functionName.length = (funcStruct.functionName.end && funcStruct.functionName.start) ? (funcStruct.functionName.end - funcStruct.functionName.start + 1) : 0;
		funcStruct.functionParameter.length = (funcStruct.functionParameter.end && funcStruct.functionParameter.start) ? (funcStruct.functionParameter.end - funcStruct.functionParameter.start + 1) : 0;
		funcStruct.functionTypeQualifier.length = (funcStruct.functionTypeQualifier.end && funcStruct.functionTypeQualifier.start) ? (funcStruct.functionTypeQualifier.end - funcStruct.functionTypeQualifier.start + 1) : 0;
		funcStruct.functionBody.length = (funcStruct.functionBody.end && funcStruct.functionBody.start) ? (funcStruct.functionBody.end - funcStruct.functionBody.start + 1) : 0;

		funcStruct.functionReturnValue.copyStrFromBuffer();
		funcStruct.functionName.copyStrFromBuffer();
		funcStruct.functionParameter.copyStrFromBuffer();
		funcStruct.functionTypeQualifier.copyStrFromBuffer();
		funcStruct.functionBody.copyStrFromBuffer();

		if (funcStruct.functionReturnValue.length + 1 + funcStruct.functionName.length + 1 + funcStruct.functionParameter.length + 1 + funcStruct.functionTypeQualifier.length < lenFuncString)
		{
			char * pTemp = funcStruct.funcString;
			int flag= 0;

			if (funcStruct.functionReturnValue.length > 0)
			{
				memcpy(pTemp, funcStruct.functionReturnValue.start, funcStruct.functionReturnValue.length);
				pTemp += funcStruct.functionReturnValue.length;
				flag = 1;
			}

			if (funcStruct.functionName.length > 0)
			{
				if(flag != 0){ *pTemp = ' '; pTemp++; }
				memcpy(pTemp, funcStruct.functionName.start, funcStruct.functionName.length);
				pTemp += funcStruct.functionName.length;
				flag = 2;
			}

			if (funcStruct.functionParameter.length > 0)
			{
				if(flag != 0){ *pTemp = ' '; pTemp++; }
				memcpy(pTemp, funcStruct.functionParameter.start, funcStruct.functionParameter.length);
				pTemp += funcStruct.functionParameter.length;
				flag = 3;
			}

			if (funcStruct.functionTypeQualifier.length > 0)
			{
				if(flag != 0){ *pTemp = ' '; pTemp++; }
				memcpy(pTemp, funcStruct.functionTypeQualifier.start, funcStruct.functionTypeQualifier.length);
				pTemp += funcStruct.functionTypeQualifier.length;
				flag = 4;
			}
			*pTemp = '\0';
		}
		else
		{
			ret = -9;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}
		
		//--------���Һ������ڲ���������Щ��������---------------
		ret = findAllFuncsInFunctionBody(funcStruct.functionBody.start, funcStruct.functionBody.length, funcStruct.funcsWhichInFunctionBody, p1, funcStruct.functionBody.lineNumberOfStart);
		if (ret != 0)
		{
			ret = -8;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		//------ȷ����������һ��C++��ĳ�Ա����-----------
		for (int i = 0; i < functions.classes.size(); ++i)
		{
			if (funcStruct.functionName.start > functions.classes[i].classBody.start && funcStruct.functionName.end < functions.classes[i].classBody.end)
			{
				if (functions.classes[i].isStruct) //˵���ǽṹ��ĳ�Ա����
				{
					int len = MIN(functions.classes[i].className.length, sizeof(funcStruct.className) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.structName, functions.classes[i].className.str, len);
						funcStruct.structName[len] = '\0';
					}

					len = MIN(functions.classes[i].classNameAlias.length, sizeof(funcStruct.classNameAlias) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.classNameAlias, functions.classes[i].classNameAlias.str, len);
						funcStruct.classNameAlias[len] = '\0';
					}
				}else //˵����C++��ĳ�Ա����
				{
					int len = MIN(functions.classes[i].className.length, sizeof(funcStruct.className) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.className, functions.classes[i].className.str, len);
						funcStruct.className[len] = '\0';
					}
				}
			}
		}
		
		//------�Ӻ������У���ȡ��������-----------
		if(strlen(funcStruct.className) <= 0)
		{
			for(int i = 0; i < (funcStruct.functionName.length - 2); ++i)
			{
				if(memcmp(funcStruct.functionName.str + i, "::", 2) == 0) //�� "B::set(int a)"�е�"B"��ȡ����
				{
					int len = MIN(i, sizeof(funcStruct.className) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.className, funcStruct.functionName.str, len);
						funcStruct.className[len] = '\0';
					}
				}
			}
		}

		//------�������������----------
		functions.funcs.push_back(funcStruct);

retry4:
		//-------������һ��ѭ��---------------
		lineNumber = funcStruct.functionBody.lineNumberOfStart;
		p21 = funcStruct.functionBody.start + 1;
		if (*p21 == '\n')
		{
			lineNumber++;
		}
		p1 = p21; //���� p1 ��ֵ
	}

end:
	//----------------
	if (buffer2){ free(buffer2); buffer2 = NULL; }

	return 0;
}


bool CFuncRoute::isKeyword(unsigned char *buffer, int bufferSize)
{
	if(buffer == NULL || bufferSize <= 0)
	{
		return false;
	}

	int len1 = sizeof(cpp_keywords) /  sizeof(cpp_keywords[0]);
	int len2 = sizeof(cpp_preprocessors) /  sizeof(cpp_preprocessors[0]);

	for(int i = 0; i < len1; ++i)
	{
		int strLen = strlen(cpp_keywords[i]);
		if(bufferSize == strLen)
		{
			if(memcmp(buffer, cpp_keywords[i], strLen) == 0)
			{
				return true;
			}
		}
	}
	
	for(int i = 0; i < len2; ++i)
	{
		int strLen = strlen(cpp_preprocessors[i]);
		if(bufferSize == strLen)
		{
			if(memcmp(buffer, cpp_preprocessors[i], strLen) == 0)
			{
				return true;
			}
		}
	}

	return false;
}


int  CFuncRoute::replaceAllCodeCommentsBySpace(unsigned char *buffer, int bufferSize)
{
	int ret = 0;
	
	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;

	//------�Ƚ���ע�͵��Ĵ����ÿո�' '���棨���кŷ�'\n'������--------
	while (p2 <= p3)
	{
		if (*p2 == '/') //��б��(oblique line)"/"ע�͵����ַ���
		{
			p21 = p2;

			if (p2 + 1 <= p3 && *(p2 + 1) == '*') //˵������ "/*...*/" ���еĶ��д���ע��
			{
				p2 += 2;

				while (p2 <= p3)
				{
					if (*p2 == '*' && p2 + 1 <= p3 && *(p2 + 1) == '/') //˵���ҵ���"*/"
					{
						p2++;
						break;
					}
					p2++;
				}

				if (p2 <= p3)
				{
					for (; p21 <= p2; ++p21)
					{
						if (*p21 != '\n') //���з�������
						{
							*p21 = ' '; //�ÿո�' '����ԭ�е��ַ�
						}
					}
				}
			}
			else if (p2 + 1 <= p3 && *(p2 + 1) == '/') //˵������ "//..." ���еĵ��д���ע��
			{
				while (p2 <= p3)
				{
					if (*p2 != '\n') //���з�������
					{
						*p2 = ' '; //�ÿո�' '����ԭ�е��ַ�
					}
					else
					{
						break; //����������һ��ע��
					}
					p2++;
				}
			}
			else
			{
				//do nothing
			}
		}

		p2++;
	}

	return 0;
}


int  CFuncRoute::replaceAllStrBySpace(unsigned char *buffer, int bufferSize)
{
	int ret = 0;
	
	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;

	//------�Ƚ����������ķ�б��"\\"�ÿո�' '����--------
	while (p2 <= p3 - 1)
	{
		if (*p2 == '\\' && *(p2 + 1) == '\\') //����������"\\"
		{
			*p2 = '`';
			p2++;
			*p2 = '`';
		}
		p2++;
	}
	
	//------�ٽ���б��"\w"ת���ַ��ÿո�' '����--------
	p2 = p1;
	while (p2 <= p3 - 1)
	{
		if (*p2 == '\\' && (*(p2 + 1) == '"' || *(p2 + 1) == '\''))
		{
			*p2 = '`';
			p2++;
			*p2 = '`';
		}
		p2++;
	}

	//------����������'�����Ĵ����ÿո�' '���棨���кŷ�'\n'������--------
	int flag = 0;
	p2 = p1;
	while (p2 <= p3)
	{
		if (*p2 == '\'')
		{
			if (flag == 0)
			{
				flag = 1;
				p21 = p2;
			}
			else if (flag == 1)
			{
				flag = 0;
				while (p21 <= p2)
				{
					*p21 = '`'; //�ÿո�' '����
					p21++;
				}
			}
		}
		p2++;
	}

	//------��˫������""�����Ĵ����ÿո�' '���棨���кŷ�'\n'������--------
	flag = 0;
	p2 = p1;
	while (p2 <= p3)
	{
		if (*p2 == '"')
		{
			if(flag == 0)
			{
				flag = 1;
				p21 = p2;
			}else if(flag == 1)
			{
				flag = 0;
				while (p21 <= p2)
				{
					*p21 = '`'; //�ÿո�' '����
					p21++;
				}
			}
		}
		p2++;
	}
	
	return 0;
}


int CFuncRoute::findStr(unsigned char *buffer, int bufferSize, const char *str, int &pos)
{
	RETURN_IF_FAILED(buffer == NULL || bufferSize <= 0 || str == NULL, -1);
	
	int strLen = strlen(str);
	RETURN_IF_FAILED(strLen <= 0, -2);

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1 - strLen;
	unsigned char *p21 = NULL;

	while(p2 <= p3)
	{
		if(memcmp(p2, str, strLen) == 0) //found it
		{
			pos = p2 - p1;
			return 0;
		}
		p2++;
	}

	return -1;
}


int CFuncRoute::findAllMacros(std::vector<std::string> files, std::vector<MACRO> &macros)
{
	int ret = 0;

	int len = files.size();
	RETURN_IF_FAILED(len <= 0, -1);

	for (int i = 0; i < len; ++i)
	{
		printf("%s: %s\n", __FUNCTION__, files[i].c_str());

		//-----------��ȡ�����ļ����ڴ���---------------
		FILE * fp = fopen(files[i].c_str(), "rb");
		RETURN_IF_FAILED(fp == NULL, -2);

		fseek(fp, 0, SEEK_END);
		long file_size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char)* file_size);
		RETURN_IF_FAILED(buffer == NULL, -3);

		size_t read_size = fread(buffer, file_size, 1, fp);
		if (read_size != 1)
		{
			printf("%s: Error: read_size=%d != 1\n", __FUNCTION__, read_size);
			fclose(fp);
			free(buffer);
			return -6;
		}

		fclose(fp);

		//-------��������"//..."��"/*...*/"ע�͵��Ĵ����ÿո�' '����----------------
		ret = replaceAllCodeCommentsBySpace(buffer, file_size);
		RETURN_IF_FAILED(ret, -4);

		//-----------------------
		char define[] = "#define";
		int defineLen = strlen(define);

		unsigned char *p1 = buffer;
		unsigned char *p2 = buffer;
		unsigned char *p3 = buffer + file_size - 1 - defineLen;
		unsigned char *p21 = NULL;
		unsigned char *p22 = NULL;
		int lineNumber = 1;

		while(p2 <= p3)
		{
			if(memcmp(p2, define, defineLen) == 0) //found it
			{
				p2 += defineLen;
				p21 = p2;
				while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
				{
					if (*p21 == '\n')
					{
						lineNumber++;
					}
					p21++;
				}

				if (p21 <= p3 && p21 > p2) //�궨��  "#define PI(a, b)    3.1415926"�У�"#define"��"PI(a, b)"֮�����������һ���հ��ַ�
				{
					p22 = p21;
					while (p21 <= p3)
					{
						if (!((*p21 >= '0' && *p21 <= '9')
							|| (*p21 >= 'a' && *p21 <= 'z')
							|| (*p21 >= 'A' && *p21 <= 'Z')
							|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
							))
						{
							break;
						}

						p21++;
					}

					if(p21 <= p3) //�ҵ� "#define PI(a, b)    3.1415926" �е� ����"PI(a, b)"
					{
						MACRO macro;
						memset(&macro, 0, sizeof(MACRO));

						int len33 = MIN(p21 - p22, sizeof(macro.macroName) - 1);
						if(len33 > 0)
						{
							memcpy(macro.macroName, p22, len33);
							macro.macroName[len33] = '\0';

							//-------���궨��Ĳ����б���"#define PI(a, b)    3.1415926" �е�"(a, b)"---------------
							if(*p21 == '(')
							{
								p22 = p21;
								while (p21 <= p3)
								{
									if(*p21 == ')')
									{
										break;
									}
									if (*p21 == '\r'|| *p21 == '\n') //����������һ���հ��ַ�����
									{
										*p21 = ' '; //�ÿո��滻
									}

									p21++;
								}

								if(p21 <= p3) //�ҵ����������б���
								{
									int macroArgsLen = p21 - p22 + 1;
									if(macroArgsLen > sizeof(macro.macroArgs) - 1)
									{
										ret = -6;
										printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
										break;
									}
									
									int len33 = MIN(macroArgsLen, sizeof(macro.macroName) - 1);
									if(len33 > 0)
									{
										memcpy(macro.macroArgs, p22, len33);
										macro.macroArgs[len33] = '\0';
										p21++;
									}
								}else
								{
									ret = -6;
									printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
									break;
								}
							}
							//--------���Һ���-------------
							p22 = p21;
							while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t')) //�����հ��ַ�
							{
								p21++;
							}

							if(p21 > p22) //�����ͺ���֮�����������һ���ո񣬼�"#define PI(a, b)    3.1415926"��"PI(a, b)"��"3.1415926"֮�����������һ���ո�
							{
								//-------���к궨��ʱ��ÿ�����һ���ַ�������'\'��б��---------
								p22 = p21;
								while (p21 <= p3 + 3)
								{
									if(memcmp(p21, "\\\r\n", 3) == 0) //for windows
									{
										memset(p21, ' ', 3); //�ÿո��滻
										p21 += 3;
									}
									else if(memcmp(p21, "\\\n", 2) == 0) //for linux
									{
										memset(p21, ' ', 2); //�ÿո��滻
										p21 += 2;
									}else if(*p21 == '\n') //�궨������һ�л��з�
									{
										break;
									}

									p21++;
								}

								if(p21 > p22) //˵���ҵ�������
								{
									//-----�������е���������հ��ַ��滻��һ���հ��ַ�-------
									int macroBodyLen = p21 - p22 + 1;
									if(macroBodyLen > sizeof(macro.macroBody) - 1)
									{
										ret = -6;
										printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
										break;
									}
									char * p41 = macro.macroBody;

									while(p22 <= p21)
									{
										if(*p22 == ' ' || *p22 == '\t' || *p22 == '\r' || *p22 == '\n')
										{
											*p41 = ' '; //��һ���ո����
										}
										else
										{
											*p41 = *p22;
											p41++;
										}
										p22++;
									}
									*p41 = '\0';

									//-------���ҵ�һ�������ĺ궨��----------
									macros.push_back(macro); //ע�⣺�еĺ궨�����滹�к궨�壬���궨��Ƕ�ף���Ҫ�ں����ٴ�չ���궨��
								}
								else
								{
									ret = -6;
									printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
									break;
								}
							}
							else //�� "#define __C_KEYWORD_H__" Ҳ�ǺϷ���
							{
								ret = 0;
								printf("%s(%d): %s: Warn: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
//								break;
							}
						}else
						{
							ret = -6;
							printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
							break;
						}
					}
					else
					{
						ret = -6;
						printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
						break;
					}
				}
				else //�� char define[] = "#define"; Ҳ�ǺϷ���
				{
					ret = 0;
					printf("%s(%d): %s: Warn: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
//					break;
				}
			}
			p2++;
		}
	}

	//-----չ��Ƕ�׵ĺ�---------
	// ...
	
	//-----����#ifdef��������---------
	// ...

	return ret;
}


int CFuncRoute::findAllClassAndStructDeclare(unsigned char *buffer, int bufferSize, std::vector<CLASS_STRUCT> &classes)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	unsigned char *p23 = NULL;
	int lineNumber = 1;

	//------�������е�C++����(�ؼ���class, struct)--------
	char classKeyword[] = "class";
	char structKeyword[] = "struct";
	int classLen = strlen(classKeyword);
	int structLen = strlen(structKeyword);
	bool isStruct = false;

	//--------���ҹؼ���class/struct----------------
	p21 = p2;
	while (p21 <= p3 - structLen)
	{
		isStruct = false;

		if (memcmp(p21, classKeyword, classLen) == 0 || memcmp(p21, structKeyword, structLen) == 0)
		{
			int lenKeyword = classLen;
			if (memcmp(p21, structKeyword, structLen) == 0)
			{
				lenKeyword = structLen;
				isStruct = true;
			}

			if ((p21 - 1 >= p1 && isValidVarChar(*(p21 - 1)) == false) //�ؼ���ǰ�������һ���հ��ַ�
				&& (p21 + lenKeyword + 1 <= p3 && isWhiteSpace(*(p21 + lenKeyword)) == true) //�ؼ���class/struct����������һ���հ��ַ�
				)
			{
				p21 += lenKeyword + 1;

				ret = skipWhiteSpaceForward(p21, p3 - p21 + 1, p21, p21, lineNumber);
				RETURN_IF_FAILED(ret, ret);

				//-----��ǰ����C++����-----
				CLASS_STRUCT cs;
				memset(&cs, 0, sizeof(CLASS_STRUCT));
				cs.isStruct = isStruct;

				cs.className.start = p21;
				cs.className.fileOffsetOfStart = cs.className.start - p1;
				cs.className.lineNumberOfStart = lineNumber;

				ret = findStrForward(p21, p3 - p21 + 1, p21, p21);
				RETURN_IF_FAILED(ret, ret);

				cs.className.end = p21;
				cs.className.fileOffsetOfEnd = cs.className.end - p1;
				cs.className.lineNumberOfEnd = lineNumber;

				cs.className.length = cs.className.end - cs.className.start + 1;
				cs.className.copyStrFromBuffer();

				//-------��ǰ����C++��/�ṹ��������----------
				ret = findCharForward(p21, p3 - p21 + 1, '{', p22); //ǰ��������������������ʼ�ַ�'{'
				RETURN_IF_FAILED(ret, ret);

				ret = findCharForward(p21, p3 - p21 + 1, ';', p23); //ǰ��������������ַ�';'
				RETURN_IF_FAILED(ret, ret);

				if (p23 <= p22)
				{
					p21 = p23 + 1;
					continue; //�������� class A; ���� class A {...};
				}

				cs.classBody.start = p22;
				cs.classBody.fileOffsetOfStart = cs.classBody.start - p1;
				lineNumber += statBufferLinesCount(cs.className.end, cs.classBody.start - cs.className.end + 1);
				cs.classBody.lineNumberOfStart = lineNumber;

				ret = findPairCharForward(p22, p3 - p21 + 1, p22, '{', '}', p22);
				RETURN_IF_FAILED(ret, ret);

				cs.classBody.end = p22;
				cs.classBody.fileOffsetOfEnd = cs.classBody.end - p1;

				cs.classBody.length = cs.classBody.end - cs.classBody.start + 1;
				lineNumber += statBufferLinesCount(cs.classBody.start, cs.classBody.length);
				cs.classBody.lineNumberOfEnd = lineNumber;
				cs.classBody.copyStrFromBuffer();

				//-------�����������������֮���Ƿ��м̳еĸ���----------
				p21 = cs.className.end + 1;
				
				ret = findCharForward(p21, cs.classBody.start - p21 + 1, ':', p21);
				if (ret == 0) //˵���и��࣬���ƣ�"class B : public A, public C{};"
				{
					p21++;
					ret = skipWhiteSpaceForward(p21, cs.classBody.start - p21 + 1, p21, p21, lineNumber);
					RETURN_IF_FAILED(ret, ret);

					cs.classParent.start = p21;
					cs.classParent.fileOffsetOfStart = cs.classParent.start - p1;
					cs.classParent.lineNumberOfStart = lineNumber;

					ret = skipWhiteSpaceBack(p21, cs.classBody.start - p21 + 1, cs.classBody.start - 1, p21, lineNumber);
					RETURN_IF_FAILED(ret, ret);

					cs.classParent.end = p21;
					cs.classParent.fileOffsetOfEnd = cs.classParent.end - p1;
					cs.classParent.lineNumberOfEnd = lineNumber;

					cs.classParent.length = cs.classParent.end - cs.classParent.start + 1;
					cs.classParent.copyStrFromBuffer();

					//-------��ָ��ࣺ"class B : public A, public C{};"------------
					ret = splitParentsClass(cs.classParent.start, cs.classParent.length, cs.classParents);
				}

				//--------���Բ��ҽṹ��ı��������� typedef struct _A_ {} A;---------------
				p21 = cs.classBody.end + 1;
				ret = findCharForward(p21, p3 - p21 + 1, ';', p22); //�����������Էֺ�';'����
				RETURN_IF_FAILED(ret, ret);

				ret = findCharForward(p21, p22 - p21 + 1, '{', p23);
				RETURN_IF_FAILED(ret == 0, ret); // "class A {};" �� '}'��';'֮�䲻�ܺ���'{'

				if (isStruct)
				{
					ret = skipWhiteSpaceForward(p21, p22 - p21 + 1, p21, p21, lineNumber);
					RETURN_IF_FAILED(ret, ret);

					cs.classNameAlias.start = p21;
					cs.classNameAlias.fileOffsetOfStart = cs.classNameAlias.start - p1;
					cs.classNameAlias.lineNumberOfStart = lineNumber;

					ret = findStrForward(p21, p22 - p21 + 1, p21, p21);
					RETURN_IF_FAILED(ret, ret);

					cs.classNameAlias.end = p21;
					cs.classNameAlias.fileOffsetOfEnd = cs.classNameAlias.end - p1;
					cs.classNameAlias.lineNumberOfEnd = lineNumber;

					cs.classNameAlias.length = cs.classNameAlias.end - cs.classNameAlias.start + 1;
					cs.classNameAlias.copyStrFromBuffer();
				}

				//-------����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����---------
				ret = findAllMemberVarsInClassDeclare(cs.classBody.start, cs.classBody.length, cs, p1, cs.classBody.lineNumberOfStart);

				//-------����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����---------
				ret = findAllMemberFuncsInClassDeclare(cs.classBody.start, cs.classBody.length, cs, p1, cs.classBody.lineNumberOfStart);

				classes.push_back(cs); //�ҵ�һ��������C++��/�ṹ������
			}
		}

		if (*p21 == '\n')
		{
			lineNumber++;
		}

		p21++;
	}

	return 0;
}


int CFuncRoute::findAllFuncsInFunctionBody(unsigned char *buffer, int bufferSize, std::vector<CLASS_INSTANCE> &funcsWhichInFunctionBody, unsigned char *bufferBase, int lineNumberBase)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p11 = bufferBase;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int lineNumber = lineNumberBase;

	//--------���Һ������ڲ���������Щ��������---------------
	while (p2 <= p3)
	{
		if (*p2 == '(') //�����Ǻ������õĲ����б��������
		{
			CLASS_INSTANCE ci;
			memset(&ci, 0, sizeof(CLASS_INSTANCE));

			ret = findWholeFuncCalled(p1, p3 - p1 + 1, p2, ci, bufferBase, lineNumber);
			if (ret == 0)
			{
				funcsWhichInFunctionBody.push_back(ci);
			}
		}
		
		if (*p2 == '\n')
		{
			lineNumber++;
		}

		p2++;
	}

end:

	return 0;
}


int CFuncRoute::findWholeFuncCalled(unsigned char *buffer, int bufferSize, unsigned char *parentheseLeft, CLASS_INSTANCE &classInstance, unsigned char *bufferBase, int lineNumberBase)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p11 = bufferBase;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int lineNumber = lineNumberBase;

	int flag = 0;

	p21 = parentheseLeft; //�������õĲ����б���С����λ��

	//-------ǰ�������Ե���С����------------
	classInstance.functionArgs.start = p21;
	classInstance.functionArgs.fileOffsetOfStart = p21 - p11;
	classInstance.functionArgs.lineNumberOfStart = lineNumber;

	ret = findPairCharForward(p21, p3 - p21 + 1, p21, '(', ')', p21);
	RETURN_IF_FAILED(ret, ret);

	classInstance.functionArgs.end = p21;
	classInstance.functionArgs.fileOffsetOfEnd = p21 - p11;
	classInstance.functionArgs.lineNumberOfEnd = lineNumber;

	classInstance.functionArgs.length = classInstance.functionArgs.end - classInstance.functionArgs.start + 1;
	classInstance.functionArgs.copyStrFromBuffer();

	//------������С����ǰ��ĵ�һ���ǿհ��ַ�--------------
	p21 = parentheseLeft - 1;

	ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
	RETURN_IF_FAILED(ret, ret);

	//-------������Һ�����------------
	classInstance.functionName.end = p21;
	classInstance.functionName.fileOffsetOfEnd = p21 - p11;
	classInstance.functionName.lineNumberOfEnd = lineNumber;

	if (*p21 == '>') //���� std::TemplateA<std::string>(a, b); ģ�庯������
	{
		ret = findPairCharBackStop(p1, p21 - p1 + 1, p21, '<', '>', ";:{}", p21);
		RETURN_IF_FAILED(ret, ret);

		p21--;
		RETURN_IF_FAILED(p21 < p1, -1);
	}

	ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
//	RETURN_IF_FAILED(ret, ret);
	if (ret != 0) //���� char * str = (char *)malloc(2);
	{
		return -1;
	}

	classInstance.functionName.start = p21;
	classInstance.functionName.fileOffsetOfStart = p21 - p11;
	classInstance.functionName.lineNumberOfStart = lineNumber;

	classInstance.functionName.length = classInstance.functionName.end - classInstance.functionName.start + 1;
	classInstance.functionName.copyStrFromBuffer();

	if (isKeyword((unsigned char *)classInstance.functionName.str, classInstance.functionName.length) == true) //���� if( a == 2 ) �����ɺ���������
	{
		return -1;
	}

	//-------����������ʵ�����ú����ķ�ʽ------------
	p21--;

	ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
	RETURN_IF_FAILED(ret, ret);

	classInstance.instanceType = VAR_TYPE_UNKNOWN; //������ȫ�ֺ���

	if (*p21 == '.') //ʹ�õ��"."���ú��������� ret = A.set(123);
	{
		classInstance.instanceType = VAR_TYPE_NORMAL;
	}else if (*p21 == '>') //ʹ�ü�ͷ"->"���ú��������� ret = A->set(123);
	{
		RETURN_IF_FAILED(p21 - 1 < p1, -1);
		p21--;

		RETURN_IF_FAILED(*p21 != '-', -1);

		classInstance.instanceType = VAR_TYPE_POINTER;
	}
	else if (*p21 == ':') //ʹ���������޶���"::"���ú��������� ret = A::set(123);
	{
		RETURN_IF_FAILED(p21 - 1 < p1, -1);
		p21--;

		RETURN_IF_FAILED(*p21 != ':', -1);

		classInstance.instanceType = VAR_TYPE_STATIC2;
	}

	//--------����������ʵ����--------------
	RETURN_IF_FAILED(p21 - 1 < p1, -1);
	p21--;

	ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
	RETURN_IF_FAILED(ret, ret);

	if (classInstance.instanceType == VAR_TYPE_NORMAL || classInstance.instanceType == VAR_TYPE_POINTER)
	{
		classInstance.classInstanceName.end = p21;
		classInstance.classInstanceName.fileOffsetOfEnd = p21 - p11;
		classInstance.classInstanceName.lineNumberOfEnd = lineNumber;

		if (*p21 == ']') //���� int len = a[i][j].length();
		{
			while (p21 >= p1 && *p21 == ']')
			{
				ret = findPairCharBack(p1, p21 - p1 + 1, p21, '[', ']', p21);
				if (ret != 0)
				{
					break;
				}
				p21--;
			}
		}

		ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
		RETURN_IF_FAILED(ret, ret);

		classInstance.classInstanceName.start = p21;
		classInstance.classInstanceName.fileOffsetOfStart = p21 - p11;
		classInstance.classInstanceName.lineNumberOfStart = lineNumber;

		classInstance.classInstanceName.length = classInstance.classInstanceName.end - classInstance.classInstanceName.start + 1;
		classInstance.classInstanceName.copyStrFromBuffer();
	}
	else if (classInstance.instanceType == VAR_TYPE_STATIC2)
	{
		//--------�����������(�������ռ���namespace)--------------
		classInstance.className.end = p21;
		classInstance.className.fileOffsetOfEnd = p21 - p11;
		classInstance.className.lineNumberOfEnd = lineNumber;

		ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
		RETURN_IF_FAILED(ret, ret);

		classInstance.className.start = p21;
		classInstance.className.fileOffsetOfStart = p21 - p11;
		classInstance.className.lineNumberOfStart = lineNumber;

		classInstance.className.length = classInstance.className.end - classInstance.className.start + 1;
		classInstance.className.copyStrFromBuffer();
	}

	//--------������Һ������õķ���ֵ--------------
	RETURN_IF_FAILED(p21 - 1 < p1, -1);
	p21--;

	ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
	RETURN_IF_FAILED(ret, ret);

	if (*p21 == '=') //���ֵȺ�'='��˵���з���ֵ
	{
		RETURN_IF_FAILED(p21 - 1 < p1, -1);
		p21--;

		if (*p21 == '=') //���� if( true == A.get() ){}
		{
			return 0;
		}

		ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
		RETURN_IF_FAILED(ret, ret);

		//--------������ҷ���ֵ������--------------
		classInstance.functionReturnValue.end = p21;
		classInstance.functionReturnValue.fileOffsetOfEnd = p21 - p11;
		classInstance.functionReturnValue.lineNumberOfEnd = lineNumber;

		ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
		RETURN_IF_FAILED(ret, ret);

		classInstance.functionReturnValue.start = p21;
		classInstance.functionReturnValue.fileOffsetOfStart = p21 - p11;
		classInstance.functionReturnValue.lineNumberOfStart = lineNumber;

		classInstance.functionReturnValue.length = classInstance.functionReturnValue.end - classInstance.functionReturnValue.start + 1;
		classInstance.functionReturnValue.copyStrFromBuffer();
		
		p21--;
	}
	
	//-------�ں������ڲ����Է������ʵ����Ӧ���������������ĳ�Ա�������鲻����-------------
	if (classInstance.instanceType == VAR_TYPE_NORMAL || classInstance.instanceType == VAR_TYPE_POINTER)
	{
		std::string classInstanceName  = classInstance.classInstanceName.str;
		std::string varDeclareType = "";
		ret = findVarDeclareBack(p1, p21 - p1 + 1, classInstanceName, varDeclareType);
		if (ret == 0)
		{
			int len = MIN(varDeclareType.length(), sizeof(classInstance.className.str) - 1);
			memcpy(classInstance.className.str, varDeclareType.c_str(), len);
			classInstance.className.str[len] = '\0';
		}
	}
	
	//-------�ں��������б��г��Է������ʵ����Ӧ���������������ĳ�Ա�������鲻����-------------
	if (classInstance.instanceType == VAR_TYPE_NORMAL || classInstance.instanceType == VAR_TYPE_POINTER)
	{
//		std::string classInstanceName  = classInstance.classInstanceName.str;
//		std::string varDeclareType = "";
	}

	return 0;
}


int CFuncRoute::findWholeFuncDeclare(unsigned char *buffer, int bufferSize, unsigned char *parentheseLeft, FUNCTION_STRUCTURE &funcDeclare, unsigned char *bufferBase, int lineNumberBase)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p11 = bufferBase;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int lineNumber = lineNumberBase;

	int flag = 0;

	p21 = parentheseLeft; //�������õĲ����б���С����λ��

	//-------ǰ�������Ե���С����------------
	funcDeclare.functionParameter.start = p21;
	funcDeclare.functionParameter.fileOffsetOfStart = p21 - p11;
	funcDeclare.functionParameter.lineNumberOfStart = lineNumber;

	ret = findPairCharForward(p21, p3 - p21 + 1, p21, '(', ')', p21);
	RETURN_IF_FAILED(ret, ret);
	
	funcDeclare.functionParameter.end = p21;
	funcDeclare.functionParameter.fileOffsetOfEnd = p21 - p11;
	funcDeclare.functionParameter.lineNumberOfEnd = lineNumber;

	funcDeclare.functionParameter.length = funcDeclare.functionParameter.end - funcDeclare.functionParameter.start + 1;
	funcDeclare.functionParameter.copyStrFromBuffer();

	//------������С����ǰ��ĵ�һ���ǿհ��ַ�--------------
	p21 = parentheseLeft - 1;

	ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
	RETURN_IF_FAILED(ret, ret);

	//-------������Һ�����------------
	funcDeclare.functionName.end = p21;
	funcDeclare.functionName.fileOffsetOfEnd = p21 - p11;
	funcDeclare.functionName.lineNumberOfEnd = lineNumber;

	if (*p21 == '>') //���� std::TemplateA<std::string>(a, b); ģ�庯������
	{
		ret = findPairCharBack(p1, p21 - p1 + 1, p21, '<', '>', p21);
		RETURN_IF_FAILED(ret, ret);

		p21--;
		RETURN_IF_FAILED(p21 < p1, -1);
	}

	ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
	if (ret != 0) //���� char * str = (char *)malloc(2);
	{
		return -1;
	}
	
	if (p21 - 1 >= p1 && *(p21 - 1) == '~') //�������������� class A {public: ~A();};
	{
		p21--;
	}

	funcDeclare.functionName.start = p21;
	funcDeclare.functionName.fileOffsetOfStart = p21 - p11;
	funcDeclare.functionName.lineNumberOfStart = lineNumber;

	funcDeclare.functionName.length = funcDeclare.functionName.end - funcDeclare.functionName.start + 1;
	funcDeclare.functionName.copyStrFromBuffer();

	if (isKeyword((unsigned char *)funcDeclare.functionName.str, funcDeclare.functionName.length) == true) //���� if( a == 2 ) �����ɺ���������
	{
		return -1;
	}

	//-------������Һ�������������------------
	p21--;
	ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
	RETURN_IF_FAILED(ret, ret);

	//---------------------
	funcDeclare.functionReturnValue.end = p21;
	funcDeclare.functionReturnValue.fileOffsetOfEnd = p21 - p11;
	funcDeclare.functionReturnValue.lineNumberOfEnd = lineNumber;

	if (*p21 == '>') //���� std::vector<std::string> a;
	{
		ret = findPairCharBack(p1, p21 - p1 + 1, p21, '<', '>', p21);
		RETURN_IF_FAILED(ret, ret);
		p21--;
	}

	ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
	if (ret == 0)
	{
		if (p21 - 2 >= p1 && *(p21 - 1) == ':' && *(p21 - 2) == ':') //���� std::string get(int a);
		{
			p21 -= 3;
			ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
			if (ret != 0)
			{
				return 0;
			}
		}

		funcDeclare.functionReturnValue.start = p21;
		funcDeclare.functionReturnValue.fileOffsetOfStart = p21 - p11;
		funcDeclare.functionReturnValue.lineNumberOfStart = lineNumber;

		funcDeclare.functionReturnValue.length = funcDeclare.functionReturnValue.end - funcDeclare.functionReturnValue.start + 1;
		funcDeclare.functionReturnValue.copyStrFromBuffer();

		ret = replaceTwoMoreWhiteSpaceByOneSpace((unsigned char *)funcDeclare.functionReturnValue.str, funcDeclare.functionReturnValue.length);

		return 0; //�ҵ���
	}

	return 0;
}


int CFuncRoute::skipWhiteSpaceForward(unsigned char *buffer, int bufferSize, unsigned char *leftPos, unsigned char *&rightPos, int &lineNumber)
{
	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	int flag = 0;

	p21 = leftPos;

	while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
	{
		if (*p21 == '\n')
		{
			lineNumber++;
		}
		p21++;
	}

	if (p21 <= p3)
	{
		rightPos = p21;
		return 0;
	}

	return -1;
}


int CFuncRoute::skipWhiteSpaceBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos, int &lineNumber)
{
	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	int flag = 0;

	p21 = rightPos;

	while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
	{
		if (*p21 == '\n')
		{
			lineNumber--;
		}
		p21--;
	}

	if (p21 >= p1)
	{
		leftPos = p21;
		return 0;
	}

	return -1;
}


int CFuncRoute::findPairCharForward(unsigned char *buffer, int bufferSize, unsigned char *leftCharPos, char leftChar, char rightChar, unsigned char *&rightCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	int flag = 0;

	p21 = leftCharPos; //�������õĲ����б���С����λ��
	while (p21 <= p3)
	{
		if (*p21 == leftChar) //���� int ret = A.set(B.get());
		{
			flag++;
		}
		else
		{
			if (*p21 == rightChar)
			{
				if (flag == 1)
				{
					rightCharPos = p21; //�ҵ���С���ŵ�λ��
					return 0;
				}
				else
				{
					flag--;
				}
			}
		}
		p21++;
	}

	return -1;
}


int CFuncRoute::findPairCharBack(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, char leftChar, char rightChar, unsigned char *&leftCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	int flag = 0;

	p21 = rightCharPos; //�������õĲ����б���С����λ��
	while (p21 >= p1)
	{
		if (*p21 == rightChar) //���� int ret = A.set(B.get());
		{
			flag++;
		}
		else
		{
			if (*p21 == leftChar)
			{
				if (flag == 1)
				{
					leftCharPos = p21; //�ҵ���С���ŵ�λ��
					return 0;
				}
				else
				{
					flag--;
				}
			}
		}
		p21--;
	}

	return -1;
}


int CFuncRoute::findPairCharBackStop(unsigned char *buffer, int bufferSize, unsigned char *rightCharPos, char leftChar, char rightChar, char *stopChar, unsigned char *&leftCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	int stopCharLen = strlen(stopChar);
	int flag = 0;

	p21 = rightCharPos; //�������õĲ����б���С����λ��
	while (p21 >= p1)
	{
		for (int i = 0; i < stopCharLen; ++i)
		{
			if (*p21 == stopChar[i]) //����ֹͣ�ַ����򷵻�ʧ��
			{
				return -1;
			}
		}

		if (*p21 == rightChar) //���� int ret = A.set(B.get());
		{
			flag++;
		}
		else
		{
			if (*p21 == leftChar)
			{
				if (flag == 1)
				{
					leftCharPos = p21; //�ҵ���С���ŵ�λ��
					return 0;
				}
				else
				{
					flag--;
				}
			}
		}
		p21--;
	}

	return -1;
}


int CFuncRoute::findCharForward(unsigned char *buffer, int bufferSize, char ch, unsigned char *&rightCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;

	p21 = p2;
	while (p21 <= p3)
	{
		if (*p21 == ch)
		{
			rightCharPos = p21;
			return 0;
		}

		p21++;
	}

	return -1;
}


int CFuncRoute::findCharForwardStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, unsigned char *&rightCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	int stopCharLen = strlen(stopChar);

	p21 = p2;
	while (p21 <= p3)
	{
		for (int i = 0; i < stopCharLen; ++i)
		{
			if (*p21 == stopChar[i]) //����ֹͣ�ַ����򷵻�ʧ��
			{
				return -1;
			}
		}

		if (*p21 == ch)
		{
			rightCharPos = p21;
			return 0;
		}

		p21++;
	}

	return -1;
}


int CFuncRoute::findCharBack(unsigned char *buffer, int bufferSize, char ch, unsigned char *&leftCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;

	p21 = p3;
	while (p21 >= p1)
	{
		if (*p21 == ch)
		{
			leftCharPos = p21;
			return 0;
		}

		p21--;
	}

	return -1;
}


int CFuncRoute::findCharsBackGreedy(unsigned char *buffer, int bufferSize, char *chars, unsigned char *&leftCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	int charsLen = strlen(chars);

	p21 = p3;
	while (p21 >= p1)
	{
		for (int i = 0; i < charsLen; ++i)
		{
			if (*p21 == chars[i]) //����ֹͣ�ַ����򷵻�ʧ��
			{
				leftCharPos = p21;
				return 0;
			}
		}

		p21--;
	}
	
	leftCharPos = p21;

	return -1;
}


int CFuncRoute::findCharBackStop(unsigned char *buffer, int bufferSize, char ch, char *stopChar, unsigned char *&leftCharPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	int stopCharLen = strlen(stopChar);

	p21 = p3;
	while (p21 >= p1)
	{
		for (int i = 0; i < stopCharLen; ++i)
		{
			if (*p21 == stopChar[i]) //����ֹͣ�ַ����򷵻�ʧ��
			{
				return -1;
			}
		}

		if (*p21 == ch)
		{
			leftCharPos = p21;
			return 0;
		}

		p21--;
	}

	return -1;
}


int CFuncRoute::findStrForward(unsigned char *buffer, int bufferSize, unsigned char *leftPos, unsigned char *&rightPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int flag = 0;

	p21 = leftPos;
	while (p21 <= p3)
	{
		if ((*p21 >= '0' && *p21 <= '9')
			|| (*p21 >= 'a' && *p21 <= 'z')
			|| (*p21 >= 'A' && *p21 <= 'Z')
			|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
			)
		{
			flag = 1;
		}
		else
		{
			if (flag == 1)
			{
				rightPos = p21 - 1;
				return 0;
			}
		}

		p21++;
	}

	return -1;
}


int CFuncRoute::findStrBack(unsigned char *buffer, int bufferSize, unsigned char *rightPos, unsigned char *&leftPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int flag = 0;

	p21 = rightPos;
	while (p21 >= p1)
	{
		if ((*p21 >= '0' && *p21 <= '9')
			|| (*p21 >= 'a' && *p21 <= 'z')
			|| (*p21 >= 'A' && *p21 <= 'Z')
			|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
			)
		{
			flag = 1;
		}
		else
		{
			if (flag == 1)
			{
				leftPos = p21 + 1;
				return 0;
			}
			else
			{
				return -1;
			}
		}

		p21--;
	}

	return -1;
}


int CFuncRoute::findStrCharsBackGreedy(unsigned char *buffer, int bufferSize, unsigned char *rightPos, char *chars, unsigned char *&leftPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int charsLen = strlen(chars);

	p21 = rightPos;
	while (p21 >= p1)
	{
		if ((*p21 >= '0' && *p21 <= '9')
			|| (*p21 >= 'a' && *p21 <= 'z')
			|| (*p21 >= 'A' && *p21 <= 'Z')
			|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
			)
		{
			leftPos = p21;
			return 0;
		}
		
		for (int i = 0; i < charsLen; ++i)
		{
			if (*p21 == chars[i])
			{
				leftPos = p21;
				return 0;
			}
		}

		p21--;
	}

	return -1;
}


int CFuncRoute::findQueryStrBackStop(unsigned char *buffer, int bufferSize, unsigned char *rightPos, char *queryStr, char *stopChar, unsigned char *&leftPos)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int flag = 0;
	int queryStrLen = strlen(queryStr);
	int stopCharLen = strlen(stopChar);

	p21 = rightPos - queryStrLen + 1;
	while (p21 >= p1)
	{
		for (int i = 0; i < stopCharLen; ++i)
		{
			if (*p21 == stopChar[i]) //����ֹͣ�ַ����򷵻�ʧ��
			{
				return -1;
			}
		}

		if (memcmp(p21, queryStr, queryStrLen) == 0)
		{
			flag = 1;
		}
		else
		{
			if (flag == 1)
			{
				leftPos = p21 + 1;
				return 0;
			}
		}

		p21--;
	}

	return -1;
}


int CFuncRoute::findVarDeclareForward(unsigned char *buffer, int bufferSize, std::string queryStr, std::string &varDeclareType)
{
	int ret = 0;

	return -1;
}


int CFuncRoute::findVarDeclareBack(unsigned char *buffer, int bufferSize, std::string queryStr, std::string &varDeclareType)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int lineNumber = 1;
	int flag = 0;

	p21 = p3 - queryStr.length();
	while (p21 >= p1)
	{
		if (memcmp(p21, queryStr.c_str(), queryStr.length()) == 0)
		{
			p22 = p21 + queryStr.length();
			if (p22 <= p3)
			{
				if (isValidVarChar(*p22) == false) //ȷ���� B var; ������ B varB; ����
				{
					p22 = p21 - 1;
					if (p22 >= p1)
					{
						if (isValidVarChar(*p22) == false)//ȷ���� B var; ������ B Cvar; ����
						{
							p21--;

							ret = skipWhiteSpaceBack(p1, p21 - p1, p21, p21, lineNumber);
							RETURN_IF_FAILED(ret, ret);

							//---------------------
							STRING_POSITON sp;
							memset(&sp, 0, sizeof(STRING_POSITON));

							sp.end = p21;

							if (*p21 == '>') //���� std::vector<std::string> a;
							{
								ret = findPairCharBack(p1, p21 - p1 + 1, p21, '<', '>', p21);
								RETURN_IF_FAILED(ret, ret);
								p21--;
							}

							ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
							if (ret == 0)
							{
								if (p21 - 2 >= p1 && *(p21 - 1) == ':' && *(p21 - 2) == ':') //���� std::string a;
								{
									p21 -= 3;
									ret = findStrBack(p1, p21 - p1 + 1, p21, p21);
									if (ret != 0)
									{
										continue;
									}
								}

								sp.start = p21;
								sp.length = sp.end - sp.start + 1;
								sp.copyStrFromBuffer();

								ret = replaceTwoMoreWhiteSpaceByOneSpace((unsigned char *)sp.str, sp.length);

								varDeclareType = sp.str;
								return 0; //�ҵ���
							}
						}
					}
				}
			}
		}

		p21--;
	}

	return -1;
}


bool CFuncRoute::isValidVarChar(char ch)
{
	if ((ch >= '0' && ch <= '9')
		|| (ch >= 'a' && ch <= 'z')
		|| (ch >= 'A' && ch <= 'Z')
		|| (ch == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
		)
	{
		return true;
	}
	return false;
}


bool CFuncRoute::isWhiteSpace(char ch)
{
	if (ch == ' ' //�ո��ַ�(space character)
		|| ch == '\t' //�Ʊ��(tab character)
		|| ch == '\r' //�س���(carriage return)
		|| ch == '\n' //���з�(new line)
		)
	{
		return true;
	}
	return false;
}


int CFuncRoute::replaceTwoMoreWhiteSpaceByOneSpace(unsigned char *buffer, int bufferSize)
{
	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	int whiteSpaceFlag = 0;
	int pos = 0;
	p21 = p1;

	while (p21 <= p3)
	{
		if (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n') //�հ��ַ�
		{
			whiteSpaceFlag = 1;
		}
		else
		{
			if (pos < bufferSize)
			{
				if (whiteSpaceFlag == 1 && pos != 0)
				{
					whiteSpaceFlag = 0;
					p1[pos] = ' '; //��������հ��ַ�����һ���ո����
					pos++;
				}
				p1[pos] = *p21;
				pos++;
			}
		}

		p21++;
	}

	p1[pos] = '\0';

	return 0;
}


int CFuncRoute::statBufferLinesCount(unsigned char *buffer, int bufferSize)
{
	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;

	int lineCount = 0;
	p21 = p1;

	while (p21 <= p3)
	{
		if (*p21 == '\n') //���з�
		{
			lineCount++;
		}

		p21++;
	}

	return lineCount;
}


int CFuncRoute::macroExpand()
{
	return 0;
}


bool CFuncRoute::isParentClass(std::string child, std::string parent, std::vector<FUNCTIONS> &vFunctions)
{
	//-------�ж�parent���Ƿ���child�ĸ���-----------
	int len1 = vFunctions.size();

	for (int i = 0; i < len1; ++i)
	{
		int len2 = vFunctions[i].classes.size();
		for (int j = 0; j < len2; ++j)
		{
			std::string className = vFunctions[i].classes[j].className.str;
			if (className == child) //���ҵ�child��Ӧ��λ��
			{
				int len3 = vFunctions[i].classes[j].classParents.size();
				for (int k = 0; k < len3; ++k)
				{
					std::string classParent = vFunctions[i].classes[j].classParents[k].str;
					if (classParent == parent) //˵��parent��child�ĸ���
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


int CFuncRoute::updateParentClass(std::vector<FUNCTIONS> &vFunctions)
{
	int ret = 0;

	//-------����C++��ĸ���------------
	int len1 = vFunctions.size();

	for (int i = 0; i < len1; ++i)
	{
		int len2 = vFunctions[i].classes.size();
		for (int j = 0; j < len2; ++j)
		{
			int len3 = vFunctions[i].classes[j].classParents.size();

			for (int k = 0; k < len3; ++k)
			{
				std::string className1 = vFunctions[i].classes[j].classParents[k].str;

				//------------------------------------
				for (int i2 = 0; i2 < len1; ++i2)
				{
					int len22 = vFunctions[i2].classes.size();
					for (int j2 = 0; j2 < len22; ++j2)
					{
						std::string className2 = vFunctions[i2].classes[j2].className.str;

						if (i != i2 && j != j2 && className1 == className2) //ȷ����ͬһ��C++����
						{
							int len4 = vFunctions[i2].classes[j2].classParents.size();
							for (int k2 = 0; k2 < len4; ++k2)
							{
								std::string className21 = vFunctions[i2].classes[j2].classParents[k2].str;

								int flag = 0;
								int len32 = vFunctions[i].classes[j].classParents.size();
								for (int k3 = 0; k3 < len32; ++k3)
								{
									std::string className11 = vFunctions[i].classes[j].classParents[k3].str;
									if (className21 == className11) //���� "class A1 �� public B1, public C1{};" �� "class A2 �� public B2, public C1{};" ��ͬ�ĸ��� "public C1"��������
									{
										flag = 1;
										break;
									}
								}

								if (flag == 0)
								{
									vFunctions[i].classes[j].classParents.push_back(vFunctions[i2].classes[j2].classParents[k2]);
								}
							}
						}
					}
				}
			}
		}
	}

	return ret;
}


int CFuncRoute::splitParentsClass(unsigned char *buffer, int bufferSize, std::vector<MY_STRING> &classParents)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;

	char publicClass[] = "public";
	char protectedClass[] = "protected";
	char privateClass[] = "private";

	while (p2 <= p3)
	{
		p1 = p2;
		while (p2 < p3 && *p2 != ',') //��֮���ö��Ÿ����ģ�"class B : public A, public C{};"
		{
			*p2++;
		}

		if (p2 <= p3)
		{
			if(*p2 == ',')
			{
				p2--;
			}

			p21 = p1;

			while (p21 <= p2 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
			{
				p21++;
			}

			if (p21 <= p2)
			{
				//------��������C++��Ĺؼ���"public/protected/private"--------
				if (memcmp(p21, publicClass, strlen(publicClass)) == 0)
				{
					p21 += strlen(publicClass);
				}
				else if (memcmp(p21, protectedClass, strlen(protectedClass)) == 0)
				{
					p21 += strlen(protectedClass);
				}
				else if (memcmp(p21, privateClass, strlen(privateClass)) == 0)
				{
					p21 += strlen(privateClass);
				}
				else
				{
					ret = -1;
					printf("%s(%d): not in [public, protected, private]; ret=%d;\n", __FUNCTION__, __LINE__, ret);
					break;
				}

				if (p21 <= p2)
				{
					while (p21 <= p2 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
					{
						p21++;
					}

					//----------���Ҹ�����-------------
					p22 = p21;

					while (p21 < p2)
					{
						if (!((*p21 >= '0' && *p21 <= '9')
							|| (*p21 >= 'a' && *p21 <= 'z')
							|| (*p21 >= 'A' && *p21 <= 'Z')
							|| (*p21 == '_')
							)) //C++ �������ͱ��������������� + ��ĸ + �»���
						{
							break;
						}
						p21++;
					}

					if (p21 <= p2 && p21 >= p22)
					{
						MY_STRING myStrParentClass;
						memset(&myStrParentClass, 0, sizeof(MY_STRING));

						int len = MIN(p21 - p22 + 1, sizeof(myStrParentClass.str) - 1);
						memcpy(myStrParentClass.str, p22, len);
						myStrParentClass.str[len] = '\0';

						classParents.push_back(myStrParentClass);
					}
				}
			}
		}

		p2++;
		if(*p2 == ',')
		{
			p2++;
		}
	}

	return ret;
}

/*
int CFuncRoute::findAllMemberVarsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p11 = bufferBase;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	unsigned char *p23 = NULL;
	int lineNumber = lineNumberBase;
	int lineNumberTemp = lineNumber;
	bool ret2 = false;

	if (*p2 == '{') //������һ���ַ���������ŵ����
	{
		p2++;
	}

	//--------����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����---------------
	while (p2 <= p3)
	{
		//--------����������-----------------
		int curlyBracesFlag = 0; //������

		if (*p2 == '{')
		{
			while (p2 <= p3)
			{
				if (*p2 == '{')
				{
					curlyBracesFlag++;
				}else if (*p2 == '}')
				{
					if (curlyBracesFlag == 0)
					{
						break;
					}
					else
					{
						curlyBracesFlag--;
					}
				}
				else if (*p2 == '\n')
				{
					lineNumber++;
				}
				p2++;
			}

			p2++;
			if (p2 >= p3)
			{
				break;
			}

			p1 = p2; //���� p1 ��ֵ
		}

		//---------------------
		if (*p2 == ';') //ÿ�������������������Էֺ�';'����
		{
			//-----��ǰ�����������----
			int equalSignFlag = 0; //�Ⱥ�
			int parenthesesLeftFlag = 0; //��С����
			int parenthesesRightFlag = 0; //��С����

			p21 = p2 - 1;
			p22 = p21;

			while (p21 >= p1)
			{
				if (((*p21 == ';') //��һ������β
					|| (*p21 == '{') //���鿪ʼ
					|| (*p21 == '}') //�������
					|| (*p21 == ':') //���� "public: A a;"
					//|| (*p21 == ')') //���� if(b) A a; ������д������û���κ������
					)) //C++ �������ͱ��������������� + ��ĸ + �»���
				{
					break;
				}
				else if (*p21 == '=') //���� virtual int func1() = 0; ���� int operator=(int &i); ���߱Ƚ������д�� class A {int m_a = 0;}; ������������ʱ����ʼ����Ա����
				{
					equalSignFlag++;
				}
				else if (*p21 == ')') //���� int func2();
				{
					parenthesesLeftFlag++;
				}
				else if (*p21 == ')') //���� int func2();
				{
					parenthesesRightFlag++;
				}
				else if (*p21 == '\n')
				{
					lineNumber--;
				}

				p21--;
			}

			//--------�ж��Ǻ����������Ǳ�������------------
			if (p21 >= p1 && p21 < p22)
			{
				if (parenthesesLeftFlag > 0 && parenthesesRightFlag > 0 && parenthesesLeftFlag == parenthesesRightFlag) //˵���Ǻ�������
				{
					//do nothing
				}
				else //˵���Ǳ������������ƣ�unsgned int m_a;
				{
					if (equalSignFlag == 0)
					{
						VAR_DECLARE var;
						memset(&var, 0, sizeof(VAR_DECLARE));

						p23 = p21 + 1;
						p21 = p22;
						lineNumberTemp = lineNumber;

						while (p21 >= p23 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
						{
							if (*p21 == '\n')
							{
								lineNumber--;
							}
							p21--;
						}

						if (p21 >= p23)
						{
							//----���ұ�����----
							var.varName.end = p21;
							var.varName.fileOffsetOfEnd = var.varName.end - p11;
							var.varName.lineNumberOfEnd = lineNumber;

							p22 = p21;

							while (p21 >= p23)
							{
								if (!((*p21 >= '0' && *p21 <= '9')
									|| (*p21 >= 'a' && *p21 <= 'z')
									|| (*p21 >= 'A' && *p21 <= 'Z')
									|| (*p21 == '_')
									)) //C++ �������ͱ��������������� + ��ĸ + �»���
								{
									break;
								}
								p21--;
							}

							if (p21 >= p23 && p21 < p22)
							{
								var.varName.start = p21 + 1;
								var.varName.fileOffsetOfStart = var.varName.start - p11;
								var.varName.lineNumberOfStart = lineNumber;
								var.varName.length = var.varName.end - var.varName.start + 1;

								var.varName.copyStrFromBuffer();

								//-------�������ұ���������----------
								while (p21 >= p23 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
								{
									if (*p21 == '\n')
									{
										lineNumber--;
									}
									p21--;
								}

								if (p21 >= p23)
								{
									var.varType.end = p21;
									var.varType.fileOffsetOfEnd = var.varType.end - p11;
									var.varType.lineNumberOfEnd = lineNumber;

									p21 = p23;
									lineNumber = lineNumberTemp;

									while (p21 >= p23 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
									{
										if (*p21 == '\n')
										{
											lineNumber++;
										}
										p21++;
									}

									if (p21 <= var.varType.end)
									{
										var.varType.start = p21;
										var.varType.fileOffsetOfStart = var.varType.start - p11;
										var.varType.lineNumberOfStart = lineNumber;
										var.varType.length = var.varType.end - var.varType.start + 1;

										var.varType.copyStrFromBuffer();

										classes.memberVars.push_back(var);

										p1 = p2; //���� p1 ��ֵ
									}
								}
							}
						}
					}
					else
					{
						printf("%s(%d): WARN: not suppport like 'class A {int m_a = 0;};'\n", __FUNCTION__, __LINE__);
					}
				}
			}
		}
		
		if (*p2 == '\n')
		{
			lineNumber++;
		}

		p2++;
	}

	return ret;
}
*/

int CFuncRoute::findAllMemberVarsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p11 = bufferBase;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	unsigned char *p23 = NULL;
	int lineNumber = lineNumberBase;
	int lineNumberTemp = lineNumber;
	bool ret2 = false;
	unsigned char * pSemicolon = 0; //�ֺ�

	p21 = p2;
	if (*p21 == '{') //������һ���ַ���������ŵ����
	{
		p21++;
	}

	//--------����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����---------------
	while (p21 <= p3)
	{
		//--------���ұ�־�������ķֺ�';'--------
		ret = findCharForward(p21, p3 - p21 + 1, ';', p22);
		if(ret != 0)
		{
			break;
		}

		pSemicolon = p22;

		ret = findCharBackStop(p21, p22 - p21 + 1, '{', "}", p23); //˵��';'���ڵ���䴦��ĳ�������ڲ�����Ҫ����
		if(ret == 0)
		{
			ret = findPairCharForward(p23, p3 - p23 + 1, p23, '{', '}', p21);
			RETURN_IF_FAILED(ret, ret);
			p21++;
			continue;
		}
		
		ret = findCharBackStop(p21, p22 - p21 + 1, '(', ")", p23); //����: "for(int i = 0; i < 2; ++i)"����Ҫ����
		if(ret == 0)
		{
			ret = findPairCharForward(p23, p3 - p23 + 1, p23, '(', ')', p21);
			RETURN_IF_FAILED(ret, ret);
			p21++;
			continue;
		}

		//----���ұ�����----
		VAR_DECLARE var;
		memset(&var, 0, sizeof(VAR_DECLARE));
		
		p22--;
		
		ret = skipWhiteSpaceBack(p21, p22 - p21 + 1, p22, p22, lineNumber);
		if(ret != 0)
		{
			p21 = pSemicolon + 1;
			continue;
		}

		var.varName.end = p22;
		var.varName.fileOffsetOfEnd = var.varName.end - p11;
		lineNumber = lineNumberBase + statBufferLinesCount(p2, p22 - p2 + 1);
		var.varName.lineNumberOfEnd = lineNumber;

		if(*var.varName.end == ']')
		{
			ret = findPairCharBack(p21, var.varName.end - p21 + 1, p22, '[', ']', p22); //����: int a[20];
			if(ret == 0)
			{
				p22--;
			}
		}

		ret = findStrBack(p21, p22 - p21 + 1, p22, p22);
		if(ret != 0)
		{
			p21 = pSemicolon + 1;
			continue;
		}
		
		var.varName.start = p22;
		var.varName.fileOffsetOfStart = var.varName.start - p11;
		var.varName.lineNumberOfStart = lineNumber;
		
		var.varName.length = var.varName.end - var.varName.start + 1;
		var.varName.copyStrFromBuffer();
		
		//----���ұ���������----
		p22--;
		ret = skipWhiteSpaceBack(p21, p22 - p21 + 1, p22, p22, lineNumber);

		ret = findStrCharsBackGreedy(p21, p22 - p21 + 1, p22, ">", p22); //����: std::vector<int> a;
		RETURN_IF_FAILED(ret, ret);
		
		var.varType.end = p22;
		var.varType.fileOffsetOfEnd = var.varType.end - p11;
		lineNumber = lineNumberBase + statBufferLinesCount(p2, p22 - p2 + 1);
		var.varType.lineNumberOfEnd = lineNumber;
		
		ret = findCharsBackGreedy(p21, p22 - p21 + 1, ";:{}", p23);
		if(ret != 0 && p23 < p21)
		{
			p23 = p21;
		}
		
		p23++;
		ret = skipWhiteSpaceForward(p23, p22 - p23 + 1, p23, p23, lineNumber);
		RETURN_IF_FAILED(ret, ret);
		
		var.varType.start = p23;
		var.varType.fileOffsetOfStart = var.varType.start - p11;
		lineNumber = lineNumberBase + statBufferLinesCount(p2, p23 - p2 + 1);
		var.varType.lineNumberOfStart = lineNumber;
		
		var.varType.length = var.varType.end - var.varType.start + 1;
		var.varType.copyStrFromBuffer();

		ret = replaceTwoMoreWhiteSpaceByOneSpace((unsigned char *)var.varType.str, var.varType.length);
		
		//----���ұ����������ͱ�����֮���Ƿ��еȺ�'='----
		ret = findCharForward(var.varType.start, var.varName.end - var.varType.start + 1, '=', p22); //����: A *a = new A; ����Ҫ����
		if(ret == 0)
		{
			p21 = pSemicolon + 1;
			continue;
		}

		//----���ұ����������ͱ�����֮���Ƿ����Ǻ�'*'��ȡ��ַ��'&'----
		var.isPointerVar = false;

		ret = findCharForward(var.varType.end, var.varName.start - var.varType.end + 1, '*', p22);
		if(ret == 0)
		{
			var.isPointerVar = true;
		}

		ret = findCharForward(var.varType.end, var.varName.start - var.varType.end + 1, '&', p22);
		if(ret == 0)
		{
			var.isPointerVar = true;
		}
		
		//--------���ҵ�һ�������ı�������-----------
		classes.memberVars.push_back(var);

		//-------------------
		p21 = pSemicolon + 1;
	}

	return ret;
}


int CFuncRoute::findAllMemberFuncsInClassDeclare(unsigned char *buffer, int bufferSize, CLASS_STRUCT &classes, unsigned char *bufferBase, int lineNumberBase)
{
	int ret = 0;

	unsigned char *p1 = buffer;
	unsigned char *p2 = buffer;
	unsigned char *p3 = buffer + bufferSize - 1;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int lineNumber = lineNumberBase;

	p21 = p1;
	while (p21 <= p3)
	{
		if (*p21 == '(')
		{
			FUNCTION_STRUCTURE fs;
			memset(&fs, 0, sizeof(FUNCTION_STRUCTURE));

			ret = findWholeFuncDeclare(p1, p3 - p1 + 1, p21, fs, bufferBase, lineNumberBase);
			if(ret == 0)
			{
				int len = MIN(strlen(classes.className.str), sizeof(fs.className) - 1);
				if(len > 0)
				{
					memcpy(fs.className, classes.className.str, len);
					fs.className[len] = '\0';
				}
				
				len = MIN(strlen(classes.classNameAlias.str), sizeof(fs.classNameAlias) - 1);
				if(len > 0)
				{
					memcpy(fs.classNameAlias, classes.classNameAlias.str, len);
					fs.classNameAlias[len] = '\0';
				}

				classes.memberFuncs.push_back(fs);
			}
		}else if (*p21 == '{' && p21 != p1) //FIXME: ������������飬����Ҳ������������
		{
			ret = findPairCharForward(p21, p3 - p21, p21, '{', '}', p21);
		}

		p21++;
	}

	return 0;
}


bool CFuncRoute::isFunctionArgsMatch(std::string parameter, std::string functionArgs)
{
	const char * str1 = parameter.c_str();
	const char * str2 = functionArgs.c_str();

	int commaCnt1 = 0; //����
	int commaCnt2 = 0; //����

	for (int i = 0; i < strlen(str1); ++i)
	{
		if (str1[i] == ',')
		{
			commaCnt1++;
		}
	}

	for (int i = 0; i < strlen(str2); ++i)
	{
		if (str2[i] == ',')
		{
			commaCnt2++;
		}
	}

	if (commaCnt1 == commaCnt2) //FIXME: Ŀǰֻ�򵥵ıȽϺ��������б�Ķ��Ÿ����Ƿ����
	{
		return true;
	}

	return false;
}


int CFuncRoute::statAllFuns(std::vector<FUNCTIONS> &vFunctions)
{
	int ret = 0;

	int funcCnt = 1;

	//-------�ȶ����к������б��------------
	int len1 = vFunctions.size();
	for (int i = 0; i < len1; ++i)
	{
		int len2 = vFunctions[i].funcs.size();
		for (int j = 0; j < len2; ++j)
		{
			vFunctions[i].funcs[j].functionIndex = funcCnt++;
		}
	}

	//-------���¸���ĸ���------------
	ret = updateParentClass(vFunctions);

	//-------���º�������C++�����������------------
	for (int i = 0; i < len1; ++i)
	{
		int len2 = vFunctions[i].funcs.size();
		for (int j = 0; j < len2; ++j)
		{
			int len3 = vFunctions[i].funcs[j].funcsWhichInFunctionBody.size();
			for (int k = 0; k < len3; ++k)
			{
				int flag = 0;
				std::string classInstanceName1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].classInstanceName.str;
				std::string className1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str;
				std::string className12 = vFunctions[i].funcs[j].className;
				std::string classNameAlias12 = vFunctions[i].funcs[j].classNameAlias;
				std::string functionName1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionName.str;
				
				if (className1 == "") //ʵ��û���������������£��ű�������
				{
					for (int i2 = 0; i2 < len1; ++i2)
					{
						int len4 = vFunctions[i2].classes.size();
						for (int j2 = 0; j2 < len4; ++j2)
						{
							//---���ڳ�Ա�����в���---
							int len4 = vFunctions[i2].classes[j2].memberVars.size();
							for (int m2 = 0; m2 < len4; ++m2)
							{
								std::string varName = vFunctions[i2].classes[j2].memberVars[m2].varName.str;
								std::string varType = vFunctions[i2].classes[j2].memberVars[m2].varType.str;
								std::string className41 = vFunctions[i2].classes[j2].className.str;

								if (varName == classInstanceName1) //C++��ĳ�Ա�����ͳ�Ա�������е�ĳ�����������
								{
									//--------���Բ��ұ����Ƿ��ڱ�����߸����������ģ�������ǣ��������ȫ�ֱ���----------------
									if (className41 == className12)
									{
										flag = 1;
									}
									else //���ж�className12�ĸ������Ƿ����className41��
									{
										bool isParent = isParentClass(className12, className41, vFunctions);
										if (isParent == true)
										{
											flag = 1;
										}
									}

									//------------------------
									if (flag == 1)
									{
										int len63 = strlen(vFunctions[i2].classes[j2].memberVars[m2].varType.str);
										int len64 = MIN(len63, sizeof(vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str) - 1);

										memcpy(vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str, vFunctions[i2].classes[j2].memberVars[m2].varType.str, len64);
										vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str[len64] = '\0';
										break;
									}
								}
							}
							
							if (flag == 1)
							{
								break; //�Ѿ��ҵ��ˣ��Ͳ��ټ�������
							}

							//---���ڳ�Ա�����в���---
							int len5 = vFunctions[i2].classes[j2].memberFuncs.size();
							for (int m2 = 0; m2 < len5; ++m2)
							{
								std::string functionName51 = vFunctions[i2].classes[j2].memberFuncs[m2].functionName.str;
								std::string className52 = vFunctions[i2].classes[j2].memberFuncs[m2].className;
								std::string classNameAlias53 = vFunctions[i2].classes[j2].memberFuncs[m2].classNameAlias;

								if (functionName51 == functionName1) //C++��ĳ�Ա������ĳ���������е�ĳ�������������
								{
									int len63 = strlen(vFunctions[i2].classes[j2].memberFuncs[m2].className);
									int len64 = MIN(len63, sizeof(vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str) - 1);

									memcpy(vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str, vFunctions[i2].classes[j2].memberFuncs[m2].className, len64);
									vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str[len64] = '\0';
									flag = 1;
									break;
								}
							}
						}
						
						if (flag == 1)
						{
							break; //�Ѿ��ҵ��ˣ��Ͳ��ټ�������
						}
					}
				}
			}
		}
	}

	//-------�ٲ���ĳ����������Щ����������------------
	for (int i = 0; i < len1; ++i)
	{
		int len2 = vFunctions[i].funcs.size();

		for (int j = 0; j < len2; ++j)
		{
			int len3 = vFunctions[i].funcs[j].funcsWhichInFunctionBody.size();

			for (int k = 0; k < len3; ++k)
			{
				std::string functionName1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionName.str;
				std::string className1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str;
				std::string functionArgs1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionArgs.str;

				//-------�����������б�������----------------
				for (int i2 = 0; i2 < len1; ++i2)
				{
					int len22 = vFunctions[i2].funcs.size();
					for (int j2 = 0; j2 < len22; ++j2)
					{
						std::string functionName2 = vFunctions[i2].funcs[j2].functionName.str;
						std::string className2 = vFunctions[i2].funcs[j2].className;
						std::string classNameAlias21 = vFunctions[i2].funcs[j2].classNameAlias;
						std::string functionParameter2 = vFunctions[i2].funcs[j2].functionParameter.str;

						size_t pos = functionName2.rfind("::");
						if (pos != std::string::npos)
						{
							functionName2 = functionName2.substr(pos + 2);
						}

						if (functionName2 == functionName1
							&& isFunctionArgsMatch(functionParameter2, functionArgs1)
							) //˵�����������������
						{
							bool isParent = isParentClass(className1, className2, vFunctions);

							if ((className1 != "" && className2 != "" && (className1 == className2) || isParent == true)
								|| (className1 != "" && classNameAlias21 != "" && className1 == classNameAlias21)
								|| (className1 == "" && className2 == "" && classNameAlias21 == "")
								)
							{
								if (vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionIndex == 0
									|| className1 == className2
									) //FIXME
								{
									vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionIndex = vFunctions[i2].funcs[j2].functionIndex;
								}
							}
//							vFunctions[i].funcs[j].funcsWhichCalledMe[vFunctions[i2].funcs[j2].functionIndex] += 1;
						}
					}
				}
			}
		}
	}

	//--------��ӡ������Ϣ-------------------
//	ret = printInfo(vFunctions);

	//--------��ӡͳ����Ϣ-------------------
	printf("============Total files��%d;=======\n", vFunctions.size());
	
	for (int i = 0; i < len1; ++i)
	{
		int len2 = vFunctions[i].funcs.size();

		for (int j = 0; j < len2; ++j)
		{
			std::string className = "";
			std::string str1 = vFunctions[i].funcs[j].classNameAlias + std::string("::");
			std::string str2 = vFunctions[i].funcs[j].functionName.str;
			
			if (strlen(vFunctions[i].funcs[j].className) > 0)
			{
				str1 = vFunctions[i].funcs[j].className + std::string("::");
			}

			if (str1 != "::" 
				&& !(str2.length() > str1.length() && str2.substr(0, str1.length()) == str1)
				)
			{
				className = str1;
			}

			printf("%s\t%d\t%d\t%s%s%s\t", vFunctions[i].fllename, vFunctions[i].funcs[j].functionName.lineNumberOfStart, vFunctions[i].funcs[j].functionIndex, 
				className.c_str(), vFunctions[i].funcs[j].functionName.str, vFunctions[i].funcs[j].functionParameter.str);
			
			int len3 = vFunctions[i].funcs[j].funcsWhichInFunctionBody.size();
			for (int k = 0; k < len3; ++k)
			{
				if (k != len3 - 1)
				{
					printf("%d,", vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionIndex);
				}
				else
				{
					printf("%d", vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].functionIndex);
				}
			}
/*
			printf("\t");

			int len4 = vFunctions[i].funcs[j].funcsWhichCalledMe.size();
			int cnt = 0;
			std::map<int, int>::iterator it;
			for (it = vFunctions[i].funcs[j].funcsWhichCalledMe.begin(); it != vFunctions[i].funcs[j].funcsWhichCalledMe.end(); ++it)
			{
				if (cnt <= len4 - 1)
				{
					printf("%d-%d,", it->first, it->second);
				}
				else
				{
					printf("%d-%d", it->first, it->second);
				}
				cnt++;
			}*/
			printf("\n");
		}
	}

	return ret;
}


int CFuncRoute::dumpBufferToFile(unsigned char *buffer, int bufferSize, char *filename)
{
	int ret = 0;

	RETURN_IF_FAILED(buffer == NULL || bufferSize <= 0 || filename == NULL, -1);

	printf("%s: filename=%s;\n", __FUNCTION__, filename);

	//-----------��ȡ�����ļ����ڴ���---------------
	FILE * fp = fopen(filename, "wb");
	RETURN_IF_FAILED(fp == NULL, -2);

	size_t read_size = fwrite(buffer, bufferSize, 1, fp);
	if (read_size != 1)
	{
		printf("%s: Error: read_size=%d != 1\n", __FUNCTION__, read_size);
		fclose(fp);
		return -6;
	}

	fclose(fp);

	return ret;
}


int CFuncRoute::printInfo(std::vector<FUNCTIONS> &vFunctions)
{
	int ret = 0;

	int len1 = vFunctions.size();
	for (int i = 0; i < len1; ++i)
	{
		printf("i=%d: ----%s-----\n", i, vFunctions[i].fllename);

		ret = vFunctions[i].printfInfo();
	}

	return ret;
}
