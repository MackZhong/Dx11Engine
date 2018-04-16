#include "EnginePCH.h"
#include "SdlParser.h"

//char SdlParser::SDL_SEPARATER[] = { '{', '}' };
//char SdlParser::SDL_FILTER[4] = { ' ','\t','\r','\n' };

SdlParser::SdlParser()
{
}


SdlParser::~SdlParser()
{
}

std::string SdlParser::MTL_KEYWORDS[KEYWORDS_COUNT] = { "material", "diffuse", "specular", "color", "blend", "tile", "alphatocoverage",
"AlphaChannel","bump","mask","envmap","MipMap","Axis","diffuse1","Modulate2x","Blend", "height","parallax_scale" };

/**�ʷ�����**/
Keywords::KwPtr SdlParser::Analyse(char const* setName, char const* dataPtr, size_t dataSize, ParseClient client) {
	std::string content(dataPtr, dataPtr + dataSize);
	OutputDebugStringA(content.c_str());
	OutputDebugStringA("\n\n");

	std::stack <Keywords::KwPtr> kwStack;
	Keywords::KwPtr curKeyset = std::make_shared<Keywords>(setName, Keywords::ROOT, nullptr);
	kwStack.push(curKeyset);

	const int MAX_LEVEL = 8;
	ULONG line = 0;
	bool isSymbol = false;
	ULONG pos = 0;
	std::string word;
	while (pos < dataSize) {
		if ('{' == dataPtr[pos]) {
			pos++;
			curKeyset.reset();
			isSymbol = false;
		}
		else if ('}' == dataPtr[pos]) {
			if (client) {
				client->Process(curKeyset->Name(), word, isSymbol);
			}
			pos++;
			curKeyset = kwStack.top();
			kwStack.pop();
		}
		else if (';' == dataPtr[pos]) {
			word = ReadUntil(dataPtr, pos, dataSize - pos, 0);
			OutputDebugStringA("Comment: \"");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA("\".\n");
		}
		else if ('\'' == dataPtr[pos] || '\"' == dataPtr[pos]) {	// �ַ���
			word = ReadUntil(dataPtr, pos, dataSize - pos, dataPtr[pos]);
			if (nullptr == curKeyset) {	// �ַ�������
				curKeyset = kwStack.top()->AddChild(word, Keywords::STR);
				kwStack.push(curKeyset);

				//bKeywordSet[level] = true;
				OutputDebugStringA("Property: \"");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA("\".\n");
			}
			else {						// �ַ���ֵ
				OutputDebugStringA("String: \"");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA("\".\n");
			}
		}
		else if ('(' == dataPtr[pos]) {
			word = ReadUntil(dataPtr, pos, dataSize - pos, ')');
			ProcessCommand(word);
		}
		else if ('.' == dataPtr[pos] || '-' == dataPtr[pos] || isdigit(dataPtr[pos])) {	// ��ֵ
			word = ReadDigit(dataPtr, pos, dataSize - pos);
			OutputDebugStringA("Digit: ");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA(".\n");
		}
		else if (isalpha(dataPtr[pos]) || '_' == dataPtr[pos]) {	// �ؼ��ʻ��߱�ʶ��
			word = ReadWord(dataPtr, pos, dataSize - pos);
			//if (bKeywordSet[level]) {	
			if (nullptr == curKeyset) {	// �ؼ���
				curKeyset = kwStack.top()->AddChild(word, Keywords::STR);
				kwStack.push(curKeyset);

				//bKeywordSet[level] = true;
				OutputDebugStringA("Keyword: ");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA(".\n");
			}
			else {						// ��ʶ��
				isSymbol = true;
				OutputDebugStringA("Symbols: ");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA(".\n");
			}
		}
		else if (isprint(dataPtr[pos])) {
			throw "Unknown syntax.";
		}

		if ('\r' == dataPtr[pos]) {
			pos++;
		}
		if ('\n' == dataPtr[pos])
		{
			pos++;
			line++;
		}
		// ������ʽ����ͬʱ����Windows��Linux������CRLF��ʽ
		while (pos < dataSize && isspace(dataPtr[pos])) {
			pos++;
		}
	}

	curKeyset = kwStack.top();
	kwStack.pop();
	return curKeyset;
}

std::string SdlParser::ReadUntil(char const* pData, ULONG& uPos, size_t stData, char chQuote)
{
	uPos++;
	ULONG uStart = uPos;
	std::stringstream stmWord;
	while ((chQuote != pData[uPos])
		&& ('\r' != pData[uPos])
		&& ('\n' != pData[uPos])) {
		if ('\\' == pData[uPos]) {	// ת���
			uPos++;
		}
		stmWord << pData[uPos++];
	}

	if (uPos - uStart > stData) {
		throw "Memory overflow";
	}
	//else if ('\n' == pData[uPos] || '\r' == pData[uPos]) {	// �س����з�������
	//	uPos--;
	//}
	else if (chQuote == pData[uPos]) {		// ������β�ַ�
		uPos++;
	}

	return stmWord.str();
}

std::string SdlParser::ReadDigit(char const* pData, ULONG& uPos, size_t stData)
{
	ULONG uStart = uPos;
	bool isHex = false;
	bool hasDot = false;
	if ('.' == pData[uPos]) {
		hasDot = true;
		uPos++;
	}
	else if ('-' == pData[uPos])
	{
		uPos++;
	}
	else if ('0' == pData[uPos]) {
		if (('x' == pData[uPos + 1] || 'X' == pData[uPos + 1])) {
			isHex = true;
			uPos += 2;
		}
		else if ('.' == pData[uPos]) {
			hasDot = true;
			uPos += 2;
		}
	}

	while ((isHex && isxdigit(pData[uPos]))		// ʮ������
		|| (!isHex && (('.' == pData[uPos] && !hasDot) || isdigit(pData[uPos]) || 'e' == pData[uPos] || ('-' == pData[uPos] && 'e' == pData[uPos - 1])))		// ʮ����
		|| ((isspace(pData[uPos + 1]) || '}' == pData[uPos + 1]) && ('f' == pData[uPos] || 'F' == pData[uPos]))	// ĩβ���������
		) {
		if ('.' == pData[uPos] && !hasDot) {
			hasDot = true;
		}
		uPos++;
	}

	if (uPos - uStart > stData) {
		throw "Memory overflow";
	}
	else if (!isspace(pData[uPos]) && ('}' != pData[uPos])) {
		throw "Invalid syntax";
	}
	//else if ('\n' == pData[uPos] || '\r' == pData[uPos]) {	// �س����з�������
	//	uPos--;
	//}

	return std::string(pData + uStart, pData + uPos);
}

std::string SdlParser::ReadWord(char const* pData, ULONG& uPos, size_t stData)
{
	ULONG uStart = uPos;
	while (isalnum(pData[uPos])		// �ַ�������
		|| ('_' == pData[uPos])		// �»���
		) {
		uPos++;
	}

	if (uPos - uStart > stData) {
		throw "Memory overflow";
	}
	else if (!isspace(pData[uPos]) && ('}' != pData[uPos])) {
		throw "Invalid syntax";
	}
	//else if ('\n' == pData[uPos] || '\r' == pData[uPos]) {	// �س����з�������
	//	uPos--;
	//}

	return std::string(pData + uStart, pData + uPos);
}

void SdlParser::ProcessCommand(std::string const& command)
{
	// TODO:
	return;
}

Keywords::KwPtr SdlParser::Analyse2(char const* setName, char const* dataPtr, size_t dataSize)
{
	std::stack <Keywords::KwPtr> kwStack;
	Keywords::KwPtr curKeyset = std::make_shared<Keywords>(setName, Keywords::ROOT, nullptr);
	kwStack.push(curKeyset);

	const int MAX_LEVEL = 8;
	//USHORT level = 0;
	ULONG line = 0;
	//bool bKeywordSet[MAX_LEVEL]{ 0 };

	//auto keywords = std::make_shared<Keywords>(setName);

	ULONG pos = 0;
	std::string word;
	while (pos < dataSize) {
		if ('{' == dataPtr[pos]) {
			pos++;
			curKeyset.reset();
			//level++;
			//bKeywordSet[level] = false;
		}
		else if ('}' == dataPtr[pos]) {
			//if (!bKeywordSet[level]) {
			//	throw "Keyword not set";
			//}
			pos++;
			//for (int l = level; l < MAX_LEVEL; l++) {
			//	bKeywordSet[l] = false;
			//}
			//level--;
			//keywords = keywords->Parent();
			curKeyset = kwStack.top();
			kwStack.pop();
		}
		else if (';' == dataPtr[pos]) {
			word = ReadUntil(dataPtr, pos, dataSize - pos, 0);
			OutputDebugStringA("Comment: \"");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA("\".\n");
		}
		else if ('\'' == dataPtr[pos] || '\"' == dataPtr[pos]) {	// �ַ���
			word = ReadUntil(dataPtr, pos, dataSize - pos, dataPtr[pos]);
			if (nullptr == curKeyset) {	// �ַ�������
				curKeyset = kwStack.top()->AddChild(word, Keywords::STR);
				kwStack.push(curKeyset);

				//bKeywordSet[level] = true;
				OutputDebugStringA("Keyword: ");
				OutputDebugStringA(word.c_str());

				OutputDebugStringA(" appended");
				OutputDebugStringA(".\n");
			}
			else {						// �ַ���ֵ
				OutputDebugStringA("String: \"");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA("\".\n");
			}
		}
		else if (isdigit(dataPtr[pos])) {	// ��ֵ
			word = ReadDigit(dataPtr, pos, dataSize - pos);
			OutputDebugStringA("Digit: ");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA(".\n");
		}
		else if (isalpha(dataPtr[pos]) || '_' == dataPtr[pos]) {	// �ؼ��ʻ��߱�ʶ��
			word = ReadWord(dataPtr, pos, dataSize - pos);
			//if (bKeywordSet[level]) {	
			if (nullptr == curKeyset) {	// �ؼ���
				curKeyset = kwStack.top()->AddChild(word, Keywords::STR);
				kwStack.push(curKeyset);

				//bKeywordSet[level] = true;
				OutputDebugStringA("Keyword: ");
				OutputDebugStringA(word.c_str());

				OutputDebugStringA(" appended");
				OutputDebugStringA(".\n");
			}
			else {						// ��ʶ��
				OutputDebugStringA("Symbols: \"");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA("\".\n");
			}
		}

		if ('\r' == dataPtr[pos]) {
			pos++;
		}
		if ('\n' == dataPtr[pos])
		{
			pos++;
			line++;
		}
		// ������ʽ����ͬʱ����Windows��Linux������CRLF��ʽ
		while (pos < dataSize && isspace(dataPtr[pos])) {
			pos++;
		}
	}

	curKeyset = kwStack.top();
	kwStack.pop();
	return curKeyset;
}

void SdlParser::Analyse(FILE * fpin)
{

}

