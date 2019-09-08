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
		
		ret = search_CPP_FuncName(buffer, file_size, functions);
		free(buffer);

		int len2 = functions.funcs.size();
		if (ret != 0 && len2 <= 0)
		{
			printf("[%d] %s; Warn: can not find any functions;\n", i, functions.fllename);
			continue;
		}

		int len = MIN(files2[i].length(), sizeof(functions.fllename) - 1);

		memcpy(functions.fllename, files2[i].c_str(), len);
		functions.fllename[len] = '\0';

		allFuncs.push_back(functions);

		//-----------------
		for (int i = 0; i < len2; ++i)
		{
			printf("[%d/%d] %s; line=%d;\n", i + 1, len2, functions.funcs[i].funcString, functions.funcs[i].functionName.lineNumberOfStart);
		}
	}

	//---------������������֮��ĵ��ù�ϵ--------------
	int len31 = allFuncs.size();

	for (int i = 0; i < len31; ++i)
	{
		int len32 = allFuncs[i].funcs.size();
		for (int j = 0; j < len32; ++j)
		{
			
		}
	}

	return 0;
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

	FUNCTIONS funcs;
	FUNCTION_STRUCTURE funcStruct;

	memset(&funcs, 0, sizeof(FUNCTIONS));
	memset(&funcStruct, 0, sizeof(FUNCTION_STRUCTURE));

	unsigned char *buffer2 = (unsigned char *)malloc(bufferSize);
	RETURN_IF_FAILED(buffer2 == NULL, -2);

	memcpy(buffer2, buffer, bufferSize); //����һ��

	unsigned char *p1 = buffer2;
	unsigned char *p2 = buffer2;
	unsigned char *p3 = buffer2 + bufferSize - 1;
	unsigned int lineNumber = 1;
	unsigned int lineNumberTemp = 1;
	unsigned char *p11 = NULL;
	unsigned char *p21 = NULL;
	unsigned char *p22 = NULL;
	int braceCount = 0; //�����ŶԼ���
	int parenthesesCount = 0; //С���ŶԼ���
	int lenFuncString = sizeof(funcStruct.funcString);
	bool ret2 = false;

	int wordCountMax = 5; //������ǰ�棬�������5���Կո�������ַ���
	int wordCount = 0;
	int overSerach = 0; //ֹͣ����
	int lineCntMax = 5; //������ǰ�棬�������5��
	int lineCnt = 0;

	//------�Ƚ���ע�͵��Ĵ����ÿո�' '���棨�кŷ�'\n'������--------
	ret = replaceAllCodeCommentsBySpace(buffer2, bufferSize);
	RETURN_IF_FAILED(ret, -2);

	//------�������е�C++����(�ؼ���class, struct)--------
	char classKeyword[] = "class";
	char structKeyword[] = "struct";
	int classLen = strlen(classKeyword);
	int structLen = strlen(structKeyword);
	bool isStruct = false;

	std::vector<CLASS_STRUCT> classes;

	//--------���ҹؼ���class/struct----------------
	while (p2 <= p3 - structLen)
	{
		isStruct = false;

		if (memcmp(p2, classKeyword, classLen) == 0 || memcmp(p2, structKeyword, structLen) == 0)
		{
			int lenKeyword = classLen;
			if (memcmp(p2, structKeyword, structLen) == 0)
			{
				lenKeyword = structLen;
				isStruct = true;
			}

			if (p2 - 1 >= p1)
			{
				p21 = p2 - 1;
				if (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n' || *p21 == ';' || *p21 == '}') //�ؼ���ǰ�������һ���հ��ַ�
				{
					p21 = p2 + lenKeyword;
					while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����ؼ���class/struct����հ��ַ�
					{
						if (*p21 == '\n')
						{
							lineNumber++;
						}
						p21++;
					}

					if (p21 >= p2 + lenKeyword + 1) //�ؼ���class/struct����������һ���հ��ַ�
					{
						//-----����C++����-----
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

						if (p21 <= p3 && p21 > p22) //�ҵ��ؼ���class/struct��
						{
							CLASS_STRUCT cs;
							memset(&cs, 0, sizeof(CLASS_STRUCT));
							cs.isStruct = isStruct;

							cs.className.start = p22;
							cs.className.end = p21 - 1;
							cs.className.fileOffsetOfStart = cs.className.start - p1;
							cs.className.fileOffsetOfEnd = cs.className.end - p1;
							cs.className.lineNumberOfStart = lineNumber;
							cs.className.lineNumberOfEnd = lineNumber;
							cs.className.length = cs.className.end - cs.className.start + 1;

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
								if (*p21 == ':') //˵���̳��Ը��࣬���� class B : public A{};
								{
									p22 = p21;
									lineNumberTemp = lineNumber;
									while (p21 <= p3 && *p21 != '{')
									{
										if (*p21 == '\n')
										{
											lineNumber++;
										}
										p21++;
									}
									if (p21 < p3 && *p21 == '{')
									{
										cs.classParent.start = p22 + 1;
										cs.classParent.end = p21 - 1;
										cs.classParent.fileOffsetOfStart = cs.classParent.start - p1;
										cs.classParent.fileOffsetOfEnd = cs.classParent.end - p1;
										cs.classParent.lineNumberOfStart = lineNumberTemp;
										cs.classParent.lineNumberOfEnd = lineNumber;
										cs.classParent.length = cs.classParent.end - cs.classParent.start + 1;
										cs.classParent.copyStrFromBuffer();
									}
								}

								if (*p21 == '{') //˵��û�и��࣬���� public A{};
								{
									cs.className.copyStrFromBuffer();
									
									cs.classBody.start = p21;
									cs.classBody.fileOffsetOfStart = cs.classBody.start - p1;
									cs.classBody.lineNumberOfStart = lineNumber;
								}

								//-------����C++������Ҵ�����-----------
								p22 = p21;
								lineNumberTemp = lineNumber;
								int curlyBracketsCnt = 0; //�����ţ���Ҫ����������ڲ���"{}"�����Ŷ�
								p21++;
								while (p21 <= p3)
								{
									if(*p21 == '{')
									{
										curlyBracketsCnt++;
									}else if(*p21 == '}')
									{
										if(curlyBracketsCnt == 0)
										{
											break;
										}
										curlyBracketsCnt--;
									}

									if (*p21 == '\n')
									{
										lineNumber++;
									}
									p21++;
								}

								if (*p21 == '}')
								{
									p21++;
									while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
									{
										if (*p21 == '\n')
										{
											lineNumber++;
										}
										p21++;
									}

									if (isStruct) //�ṹ��ı��������� typedef struct _A_ {} A;
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

										if (p21 <= p3 && p21 > p22)
										{
											cs.classNameAlias.start = p22;
											cs.classNameAlias.end = p21 - 1;
											cs.classNameAlias.fileOffsetOfStart = cs.classNameAlias.start - p1;
											cs.classNameAlias.fileOffsetOfEnd = cs.classNameAlias.end - p1;
											cs.classNameAlias.lineNumberOfStart = lineNumber;
											cs.classNameAlias.lineNumberOfEnd = lineNumber;
											cs.classNameAlias.length = cs.classNameAlias.end - cs.classNameAlias.start + 1;
											cs.classNameAlias.copyStrFromBuffer();

											while (p21 <= p3 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
											{
												if (*p21 == '\n')
												{
													lineNumber++;
												}
												p21++;
											}
										}
									}

									if (p21 + 1 <= p3 && *p21 == ';') //˵���ҵ���һ��������C++��
									{
										cs.classBody.end = p21;
										cs.classBody.fileOffsetOfEnd = cs.classBody.end - p1;
										cs.classBody.lineNumberOfEnd = lineNumber;
										cs.classBody.length = cs.classBody.end - cs.classBody.start + 1;
										cs.classBody.copyStrFromBuffer();

										classes.push_back(cs);

										p2 = p21;
									}
								}
							}
						}
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
	
	//---------------------------
	p2 = buffer2;
	p21 = NULL;
	p22 = NULL;
	lineNumber = 1;

retry:
	while (p2 <= p3)
	{
		memset(&funcStruct, 0, sizeof(FUNCTION_STRUCTURE));

		//--------���Һ������������----------------
		while (p2 <= p3 && *p2 != '{')
		{
			if (*p2 == '\n')
			{
				lineNumber++;
			}
			p2++;
		}

		if (p2 >= p3)
		{
			ret = -2;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		if (*p2 == '{')
		{
			funcStruct.functionBody.start = p2;
			funcStruct.functionBody.fileOffsetOfStart = funcStruct.functionBody.start - p1;
			funcStruct.functionBody.lineNumberOfStart = lineNumber;
		}

		//--------���Һ���������С����----------------
		lineNumberTemp = lineNumber;
		p21 = funcStruct.functionBody.start - 1;

		while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
		{
			if (*p21 == '\n')
			{
				lineNumber--;
			}
			p21--;
		}

		if (p21 <= p1)
		{
			ret = -3;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		if (*p21 == ')')
		{
			funcStruct.functionParameter.end = p21;
			funcStruct.functionParameter.fileOffsetOfEnd = funcStruct.functionParameter.end - p1;
			funcStruct.functionParameter.lineNumberOfEnd = lineNumber;
		}
		else
		{
			//-----���Բ��Һ��������б���С���ź����C/C++���η�(type-qualifier�����޶���)--------
			while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
			{
				if (*p21 == '\n')
				{
					lineNumber--;
				}
				p21--;
			}

			if (p21 <= p1)
			{
				ret = -6;
				printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
				break;
			}

			funcStruct.functionTypeQualifier.end = p21;
			funcStruct.functionTypeQualifier.fileOffsetOfEnd = funcStruct.functionTypeQualifier.end - p1;
			funcStruct.functionTypeQualifier.lineNumberOfEnd = lineNumber;

			while (p21 >= p1)
			{
				if ((*p21 >= '0' && *p21 <= '9')
					|| (*p21 >= 'a' && *p21 <= 'z')
					|| (*p21 >= 'A' && *p21 <= 'Z')
					|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
					)
				{

				}
				else
				{
					break;
				}

				p21--;
			}

			if (p21 <= p1)
			{
				ret = -7;
				printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
				break;
			}

			funcStruct.functionTypeQualifier.start = p21 + 1;
			funcStruct.functionTypeQualifier.fileOffsetOfStart = funcStruct.functionTypeQualifier.start - p1;
			funcStruct.functionTypeQualifier.lineNumberOfStart = lineNumber;

			//---------�������Բ��Һ���������С����--------------
			while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
			{
				if (*p21 == '\n')
				{
					lineNumber--;
				}
				p21--;
			}

			if (p21 <= p1)
			{
				ret = -6;
				printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
				break;
			}

			if (*p21 == ')')
			{
				funcStruct.functionParameter.end = p21;
				funcStruct.functionParameter.fileOffsetOfEnd = funcStruct.functionParameter.end - p1;
				funcStruct.functionParameter.lineNumberOfEnd = lineNumber;
			}
			else //------˵������һ�������ĺ������壬�����һ��λ�����²���--------
			{
				lineNumber = lineNumberTemp;
				p2++;
				if (*p2 == '\n')
				{
					lineNumber++;
				}
				p1 = p2; //���� p1 ��ֵ
				goto retry;
			}
		}

		//--------���Һ���������С����----------------
		parenthesesCount = 0;
		p21 = funcStruct.functionParameter.end - 1;

		while (p21 >= p1)
		{
			if (*p21 == ')') //���������б��ڲ�����Ҳ����"()"С���Ŷ�
			{
				parenthesesCount++;
			}
			else if (*p21 == '(')
			{
				if (parenthesesCount == 0) //˵���ҵ���
				{
					break;
				}
				else
				{
					parenthesesCount--;
				}
			}

			if (*p21 == '\n')
			{
				lineNumber--;
			}
			p21--;
		}

		if (p21 <= p1)
		{
			ret = -8;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		if (*p21 == '(')
		{
			funcStruct.functionParameter.start = p21;
			funcStruct.functionParameter.fileOffsetOfStart = funcStruct.functionParameter.start - p1;
			funcStruct.functionParameter.lineNumberOfStart = lineNumber;
		}

		//--------���Һ�����----------------
		p21 = funcStruct.functionParameter.start - 1;

		while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
		{
			if (*p21 == '\n')
			{
				lineNumber--;
			}
			p21--;
		}

		if (p21 <= p1)
		{
			ret = -4;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}
		
		funcStruct.functionName.end = p21;
		funcStruct.functionName.fileOffsetOfEnd = funcStruct.functionName.end - p1;
		funcStruct.functionName.lineNumberOfEnd = lineNumber;

		p21 = funcStruct.functionName.end - 1;
		while (p21 >= p1)
		{
			if (!((*p21 >= '0' && *p21 <= '9') 
				|| (*p21 >= 'a' && *p21 <= 'z') 
				|| (*p21 >= 'A' && *p21 <= 'Z') 
				|| (*p21 == '_')
				|| (*p21 == '~') //C++ �����������
				|| (*p21 == ':') //C++ ���������޶���"::"
				)) //C++ �������ͱ��������������� + ��ĸ + �»���
			{
				break;
			}

			p21--;
		}

		if (p21 <= p1)
		{
			ret = -5;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		funcStruct.functionName.start = p21 + 1;
		funcStruct.functionName.fileOffsetOfStart = funcStruct.functionName.start - p1;
		funcStruct.functionName.lineNumberOfStart = lineNumber;

		ret2 = isKeyword(funcStruct.functionName.start, funcStruct.functionName.end - funcStruct.functionName.start + 1);
		if(ret2) //��������C/C++���Թؼ���
		{
			lineNumber = lineNumberTemp;
			p2++;
			if (*p2 == '\n')
			{
				lineNumber++;
			}
			p1 = p2; //���� p1 ��ֵ
			goto retry;
		}

		//--------���Һ�����ֵ----------------
		p21 = funcStruct.functionName.start - 1;

		while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
		{
			if (*p21 == '\n')
			{
				lineNumber--;
			}
			p21--;
		}

		if (p21 <= p1)
		{
			ret = -6;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
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
		while (p21 >= p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
		{
			if (*p21 == '\n')
			{
				lineNumber--;
			}
			p21--;
		}

		while (p21 >= p1)
		{
			if ((*p21 >= '0' && *p21 <= '9')
				|| (*p21 >= 'a' && *p21 <= 'z')
				|| (*p21 >= 'A' && *p21 <= 'Z')
				|| (*p21 == '_') //C++ �������ͱ��������������� + ��ĸ + �»���
				|| (*p21 == '<' || *p21 == '>') //C++ ģ�� template <typename T> class A {};
				)
			{

			}
			else
			{
				if (*p21 == '{' //���ܺ�������C++����ڲ�
					|| *p21 == '}' //��������һ������������λ��
					|| *p21 == ';' //������
					|| *p21 == '��' //���� #pragma comment(lib, "user32.lib")
					|| *p21 == '\\' //���� '#define  MACRO_A \'
					|| *p21 == ':' //���� class A { public: A(){} };
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

		if (p21 <= p1)
		{
			ret = -7;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		wordCount++;
		if (overSerach == 0 && wordCount < wordCountMax && lineCnt < lineCntMax)
		{
			goto retry2;
		}

		if (p21 < funcStruct.functionName.start)
		{
			funcStruct.functionReturnValue.start = p21;
			funcStruct.functionReturnValue.fileOffsetOfStart = funcStruct.functionReturnValue.start - p1;
			funcStruct.functionReturnValue.lineNumberOfStart = lineNumber;
		}

		//--------���Һ������Ҵ�����----------------
retry3:
		lineNumber = lineNumberTemp;
		braceCount = 0;
		p2 = funcStruct.functionBody.start + 1;

		while (p2 <= p3)
		{
			if (*p2 == '{') //�������ڲ�����Ҳ����"{}"�����Ŷ�
			{
				braceCount++;
			}else if (*p2 == '}')
			{
				if (braceCount == 0) //˵���ҵ���
				{
					break;
				}
				else
				{
					braceCount--;
				}
			}

			if (*p2 == '\n')
			{
				lineNumber++;
			}
			p2++;
		}

		if (p2 >= p3)
		{
			ret = -8;
			printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
			break;
		}

		if (*p2 == '}')
		{
			funcStruct.functionBody.end = p2;
			funcStruct.functionBody.fileOffsetOfEnd = funcStruct.functionBody.end - p1;
			funcStruct.functionBody.lineNumberOfEnd = lineNumber;

			//--------���Һ������ڲ���������Щ��������---------------
			p21 = funcStruct.functionBody.start;
			std::vector<unsigned char *> vPointers;

			while(p21 < p2)
			{
				if(*p21 == '(') //�����Ǻ������õĲ����б��������
				{
					vPointers.push_back(p21);
				}else if(*p21 == ')') //�����Ǻ������õĲ����б��������
				{
					
				}

				p21++;
			}

			//-----------------------
			p1 = p2; //���� p1 ��ֵ
		}

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

		//------ȷ����������һ��C++��ĳ�Ա����-----------
		for(int i = 0; i < classes.size(); ++i)
		{
			if(funcStruct.functionName.start > classes[i].classBody.start && funcStruct.functionName.end < classes[i].classBody.end)
			{
				if(classes[i].isStruct) //˵���ǽṹ��ĳ�Ա����
				{
					int len = MIN(classes[i].className.length, sizeof(funcStruct.className) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.structName, classes[i].className.str, len);
						funcStruct.structName[len] = '\0';
					}

					len = MIN(classes[i].classNameAlias.length, sizeof(funcStruct.classNameAlias) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.classNameAlias, classes[i].classNameAlias.str, len);
						funcStruct.classNameAlias[len] = '\0';
					}
				}else //˵����C++��ĳ�Ա����
				{
					int len = MIN(classes[i].className.length, sizeof(funcStruct.className) - 1);
					if(len > 0)
					{
						memcpy(funcStruct.className, classes[i].className.str, len);
						funcStruct.className[len] = '\0';
					}
				}
			}
		}
		
		//------�Ӻ������У���ȡ��������-----------
		if(strlen(funcStruct.className) <= 0)
		{
			for(int i = 0; i < funcStruct.functionName.length - 2; ++i)
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
		funcs.funcs.push_back(funcStruct);
	}

end:
	//----------------
	if (buffer2){ free(buffer2); buffer2 = NULL; }

	functions = funcs;

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

	//------�Ƚ���ע�͵��Ĵ����ÿո�' '���棨�кŷ�'\n'������--------
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
							else
							{
								ret = -6;
								printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
								break;
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
				else
				{
					ret = -6;
					printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
					break;
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

