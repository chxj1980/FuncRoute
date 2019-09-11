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
								if (*p21 == ':') //˵���̳��Ը��࣬���ƣ�"class B : public A, public C{};"
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

										//-------��ָ��ࣺ"class B : public A, public C{};"------------
										ret = splitParentsClass(cs.classParent.start, cs.classParent.length, cs.classParents);
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

										//-------����/�ṹ������������ڲ�����ȡ�����������ĳ�Ա����---------
										ret = findAllMemberVarsInClassDeclare(cs.classBody.start, cs.classBody.length, cs, p1, cs.classBody.lineNumberOfStart);

										functions.classes.push_back(cs);

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
			funcStruct.functionBody.length = funcStruct.functionBody.end - funcStruct.functionBody.start + 1;

			//--------���Һ������ڲ���������Щ��������---------------
			ret = findAllFuncsInFunctionBody(funcStruct.functionBody.start, funcStruct.functionBody.length, funcStruct.funcsWhichInFunctionBody, p1, funcStruct.functionBody.lineNumberOfStart);
			if (ret != 0)
			{
				ret = -8;
				printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
				break;
			}
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
		functions.funcs.push_back(funcStruct);
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


int CFuncRoute::findAllFuncsInFunctionBody(unsigned char *buffer, int bufferSize, std::vector<CLASS_INSTANCE> &funcsWhichInFunctionBody, unsigned char *bufferBase, int lineNumberBase)
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
	int pointerClassFlag = 0;

	//--------���Һ������ڲ���������Щ��������---------------
retry:
	while (p2 <= p3)
	{
		if (*p2 == '(') //�����Ǻ������õĲ����б��������
		{
			p21 = p2;
			lineNumberTemp = lineNumber;

			CLASS_INSTANCE instance;
			memset(&instance, 0, sizeof(CLASS_INSTANCE));

			instance.functionArgs.start = p21;
			instance.functionArgs.fileOffsetOfStart = instance.functionArgs.start - p11;
			instance.functionArgs.lineNumberOfStart = lineNumber;

			p21--;
			while (p21 > p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
			{
				if (*p21 == '\n')
				{
					lineNumber--;
				}
				p21--;
			}

			if (p21 > p1)
			{
				instance.functionName.end = p21;
				instance.functionName.fileOffsetOfEnd = instance.functionName.end - p11;
				instance.functionName.lineNumberOfEnd = lineNumber;

				//----���Һ�����----
				while (p21 >= p1)
				{
					if (!((*p21 >= '0' && *p21 <= '9')
						|| (*p21 >= 'a' && *p21 <= 'z')
						|| (*p21 >= 'A' && *p21 <= 'Z')
						|| (*p21 == '_')
						)) //C++ �������ͱ��������������� + ��ĸ + �»���
					{
						break;
					}

					if (*p21 == '\n')
					{
						lineNumber--;
					}
					p21--;
				}

				if (p21 > p1) //�ҵ���������
				{
					instance.functionName.start = p21 + 1;
					instance.functionName.fileOffsetOfStart = instance.functionName.start - p11;
					instance.functionName.lineNumberOfStart = lineNumber;
					instance.functionName.length = instance.functionName.end - instance.functionName.start + 1;

					instance.functionName.copyStrFromBuffer();

					//����Ƿ������� if(...) �ȹؼ����﷨������ǣ�������
					ret2 = isKeyword(instance.functionName.start, instance.functionName.end - instance.functionName.start + 1);
					if (ret2) //��������C/C++���Թؼ���
					{
						lineNumber = lineNumberTemp;
						p2++;
						if (*p2 == '\n')
						{
							lineNumber++;
						}
//						p1 = p2 + 1; //���� p1 ��ֵ
						goto retry;
					}

					//-----���Ҹú����Ƿ���C++��ĳ�Ա����----------
					while (p21 > p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
					{
						if (*p21 == '\n')
						{
							lineNumber--;
						}
						p21--;
					}

					if (p21 > p1)
					{
						if (*p21 == '.' //ʹ�õ��"."���ú��������� ret = A.set(123);
							|| *p21 == '>' //ʹ�ü�ͷ"->"���ú��������� ret = A->set(123);
							)
						{
							if (*p21 == '>')
							{
								if (p21 - 1 > p1 && *(p21 - 1) == '-')
								{
									pointerClassFlag = 1;
									p21--;
								}
								else //��ͷ"->"�е�"-"��">"֮�䲻�����κ������ַ�
								{
									ret = -8;
									printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
									goto end;
								}
							}

							p21--;

							if (p21 > p1)
							{
								while (p21 > p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
								{
									if (*p21 == '\n')
									{
										lineNumber--;
									}
									p21--;
								}

								if (p21 > p1) //�ҵ�C++���ʵ��������
								{
									instance.classInstanceName.end = p21;
									instance.classInstanceName.fileOffsetOfEnd = instance.classInstanceName.end - p11;
									instance.classInstanceName.lineNumberOfEnd = lineNumber;

									//----����ʵ������----
									while (p21 >= p1)
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

									if (p21 > p1)
									{
										instance.classInstanceName.start = p21 + 1;
										instance.classInstanceName.fileOffsetOfStart = instance.classInstanceName.start - p11;
										instance.classInstanceName.lineNumberOfStart = lineNumber;
										instance.classInstanceName.length = instance.classInstanceName.end - instance.classInstanceName.start + 1;

										instance.classInstanceName.copyStrFromBuffer();

										//-------�ں������ڲ����Է������ʵ����Ӧ���������������ĳ�Ա�������鲻����-------------
										p21 = instance.classInstanceName.start - instance.classInstanceName.length;
										unsigned char * pTemp = p21;

										while (p21 >= p1)
										{
											if (memcmp(p21, instance.classInstanceName.start, instance.classInstanceName.length) == 0)
											{
												p21--;
												p22 = p21;
												pTemp = p21;

												int equalSignFlag = 0;
												while (p21 >= p1)
												{
													if (((*p21 >= '0' && *p21 <= '9')
														|| (*p21 >= 'a' && *p21 <= 'z')
														|| (*p21 >= 'A' && *p21 <= 'Z')
														|| (*p21 == '_')
														)) //C++ �������ͱ��������������� + ��ĸ + �»���
													{
														break;
													}
													else if (*p21 == '=') //�ҵ����еȺŵ���� ret = classB.set(1); �ˣ�������������������
													{
														equalSignFlag = 1;
														break;
													}

													if (*p21 == '\n')
													{
														lineNumber--;
													}

													p21--;
												}

												if (equalSignFlag == 1)
												{
													continue;
												}

												p23 = p22;
												if (p21 > p1 && p21 < p23) //��������������֮�����������һ����������ĸ�»����ַ������磺 A *a = new A(); A b(); A &c = d; std::vector<int>e;
												{
													p23 += instance.classInstanceName.length + 1;
													
													while (p23 < instance.classInstanceName.start)
													{
														if (*p23 == ';') //C++ ��������������һ���ֺ�';'��β
														{
															p23--;
															break;
														}
														if (*p23 == '\n')
														{
															//lineNumber++;
														}
														p23++;
													}

													if (p23 < instance.classInstanceName.start) //˵�����ȷ��һ���������ڲ������ı���
													{
														//------�������ұ�������--------
														while (p21 >= p1)
														{
															if (((*p21 == ';') //��һ������β
																|| (*p21 == '{') //���鿪ʼ
																|| (*p21 == '}') //�������
																|| (*p21 == '��') //���� "public: A a;"
																//|| (*p21 == ')') //���� if(b) A a; ������д������û���κ������
																)) //C++ �������ͱ��������������� + ��ĸ + �»���
															{
																break;
															}

															if (*p21 == '\n')
															{
																lineNumber--;
															}

															p21--;
														}

														p21++;
														if (p21 >= p1) //�ҵ�������������
														{
															while (p21 < instance.classInstanceName.start && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
															{
																if (*p21 == '\n')
																{
																	lineNumber++;
																}
																p21++;
															}

															//----------------------------
															instance.className.start = p21;
															instance.className.fileOffsetOfStart = instance.className.start - p11;
															instance.className.lineNumberOfStart = lineNumber;

															instance.className.end = p22;
															instance.className.fileOffsetOfEnd = instance.className.end - p11;
															instance.className.lineNumberOfEnd = lineNumber;

															instance.className.length = instance.className.end - instance.className.start + 1;
															//instance.className.copyStrFromBuffer();

															int whiteSpaceFlag = 0;
															int pos = 0;

															while (p21 <= p22)
															{
																if (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n') //�հ��ַ�
																{
																	whiteSpaceFlag = 1;
																}
																else
																{
																	if (pointerClassFlag == 1 && *p21 == '*')
																	{
																		break;
																	}

																	if (pos < sizeof(instance.className.str) - 2)
																	{
																		if (whiteSpaceFlag == 1)
																		{
																			whiteSpaceFlag = 0;
																			instance.className.str[pos] = ' '; //��������հ��ַ�����һ���ո����
																			pos++;
																		}
																		instance.className.str[pos] = *p21;
																		pos++;
																	}
																}

																if (*p21 == '\n')
																{
																	lineNumber++;
																}

																p21++;
															}

															instance.className.str[pos] = '\0';
															break; //�ں������ڲ��ҵ����������ˣ���ֱ���˳�ѭ��
														}
													}
												}

												p21 = pTemp;
											}
											p21--;
										}
									}
								}
							}
						}
						else if (*p21 == ':') //ʹ�ü�ͷ"::"���ú��������� ret = A::set(123);
						{
							if (p21 - 1 > p1 && *(p21 - 1) == ':')
							{
								p21 -= 2;
							}
							else //��ͷ"::"�е�":"��":"֮�䲻�����κ������ַ�
							{
								ret = -8;
								printf("%s(%d): %s: Error: ret=%d; lineNumber=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret, lineNumber);
								goto end;
							}

							if (p21 > p1)
							{
								while (p21 > p1 && (*p21 == ' ' || *p21 == '\t' || *p21 == '\r' || *p21 == '\n')) //�����հ��ַ�
								{
									if (*p21 == '\n')
									{
										lineNumber--;
									}
									p21--;
								}

								if (p21 > p1) //�ҵ�C++������
								{
									instance.className.end = p21;
									instance.className.fileOffsetOfEnd = instance.className.end - p11;
									instance.className.lineNumberOfEnd = lineNumber;

									//----��������----
									while (p21 >= p1)
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

									if (p21 > p1)
									{
										instance.className.start = p21 + 1;
										instance.className.fileOffsetOfStart = instance.className.start - p11;
										instance.className.lineNumberOfStart = lineNumber;
										instance.className.length = instance.className.end - instance.className.start + 1;

										instance.className.copyStrFromBuffer();
									}
								}
							}
						}
					}
				}
			}

			//---------���Һ������õĲ����б��������-----------
			p21 = instance.functionArgs.start;
			lineNumber = lineNumberTemp;

			while (p21 < p3 && *p21 != ')')
			{
				if (*p21 == '\n')
				{
					lineNumber++;
				}
				p21++;
			}

			if (p21 <= p3)
			{
				instance.functionArgs.end = p21;
				instance.functionArgs.fileOffsetOfEnd = instance.functionArgs.end - p11;
				instance.functionArgs.lineNumberOfEnd = lineNumber;
				instance.functionArgs.length = instance.functionArgs.end - instance.functionArgs.start + 1;
				instance.functionArgs.copyStrFromBuffer();

				funcsWhichInFunctionBody.push_back(instance);
			}

			p2 = p21; //���� p2 ��ֵ
		}
		
		if (*p2 == '\n')
		{
			lineNumber++;
		}

		p2++;
	}

end:

	return ret;
}


int CFuncRoute::macroExpand()
{
	int ret = 0;

	return ret;
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

					while (p21 <= p2)
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

					if (p21 <= p2 && p21 > p22)
					{
						MY_STRING myStrParentClass;
						memset(&myStrParentClass, 0, sizeof(MY_STRING));

						int len = MIN(p21 - p22, sizeof(myStrParentClass.str) - 1);
						memcpy(myStrParentClass.str, p22, len);
						myStrParentClass.str[len] = '\0';

						classParents.push_back(myStrParentClass);
					}
				}
			}
		}

		p2++;
	}

	return ret;
}


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
				std::string classInstanceName1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].classInstanceName.str;
				std::string className1 = vFunctions[i].funcs[j].funcsWhichInFunctionBody[k].className.str;
				std::string className12 = vFunctions[i].funcs[j].className;
				std::string classNameAlias12 = vFunctions[i].funcs[j].classNameAlias;

				if (className1 == "") //ʵ��û���������������£��ű�������
				{
					for (int i2 = 0; i2 < len1; ++i2)
					{
						int len4 = vFunctions[i2].classes.size();
						for (int j2 = 0; j2 < len4; ++j2)
						{
							int len4 = vFunctions[i2].classes[j2].memberVars.size();
							for (int m2 = 0; m2 < len4; ++m2)
							{
								std::string varName = vFunctions[i2].classes[j2].memberVars[m2].varName.str;
								std::string className41 = vFunctions[i2].classes[j2].className.str;

								if (varName == classInstanceName1) //C++��ĳ�Ա�����ͳ�Ա�������е�ĳ�����������
								{
									//--------���Բ��ұ����Ƿ��ڱ�����߸����������ģ�������ǣ��������ȫ�ֱ���----------------
									int flag = 0;

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
									}
								}
							}
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
						if (j == 14 && k == 3)
						{
							int a = 1;
						}
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

	//--------��ӡͳ����Ϣ-------------------
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
