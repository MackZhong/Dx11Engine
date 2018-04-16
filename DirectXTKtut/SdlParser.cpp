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
/**词法分析**/
void SdlParser::Analyse(char const* data, size_t size) {
	SdlStats stats;
	std::wostringstream msgStream;

	char ch;
	UINT32 pos = 0;
	string word;

	ch = data[0];
	int iLine = 1;
	while (pos < size) {	// 开始一行
		ch = data[pos++];
		if (!isascii(ch)) {
			throw "None ascii code not supported";
		}

		if (isspace(ch)) {			// 判断是否为分割字符
			if (stats.inword) {		// 一个单词结束
				if (!stats.keyget) {	// 关键词
					if (IsKeyword(word)) {
						msgStream << L"关键字：" << word.c_str() << endl;
						stats.keyget = true;
					}
					else {
						throw std::exception("Invalid keyword");
					}
				}
				else {			// 标识符
					msgStream << L"\t标识符：" << word.c_str() << endl;
				}

				word.clear();
				stats.inword = false;
			}
			else if (stats.indigit) {	// 数值
				msgStream << L"\t数值量：" << word.c_str() << endl;

				word.clear();
				stats.indigit = false;
				stats.ishex = false;
			}
			else if ('\r' == ch || '\n' == ch) {
				//EndLine();

				if (stats.quart) {
					throw std::exception("Multiline string not support");
				}
				stats.NewToken();		// 行末初始化，不支持分行语句
				word.clear();
				while (isspace(data[pos++ + 1])) {}	// 处理多个空行
			}
			continue;
		}
		else if (stats.quart == ch) {	// 字符串结束，先处理结束符，可能是空串。文本不应该有“0”值
			if (word.empty()) {
				msgStream << L"\t字符串为空" << endl;
			}
			else {
				msgStream << L"\t字符串：" << word.c_str() << "" << endl;
			}

			stats.quart = 0;
			word.clear();
			continue;
		}
		else if (stats.quart) {		// 读取字符串
			if ('\\' == ch) {		// 转义符
				ch = data[pos++];	// 默认跳过
			}
			word += ch;

			continue;
		}
		else switch (ch) {
		case '\r':		// 回车
		case '\n':		// 换行
		{
			//EndLine();

			if (stats.quart) {
				throw std::exception("Multiline string not support");
			}
			stats.NewToken();		// 行末初始化，不支持分行语句
			word.clear();
			while (isspace(data[pos++ + 1])) {}	// 处理多个空行

			continue;
		}
		case ';':	// 注释到行尾
		{
			if (stats.indigit || stats.inword || stats.quart) {
				//EndToken();
			}
			do {
				word += ch;
				ch = data[pos + 1];
				if ('\r' != ch || '\n' != ch)
					break;
				pos++;
			} while (true);
			continue;
		}
		case '\'':	// 单引号字符串
			stats.quart = '\'';
			continue;
		case '\"':	// 双引号字符串
			stats.quart = '\"';
			continue;
		case '{':	// 定义块开始
			stats.level++;
			stats.NewToken();
			continue;
		case '}':	// 定义块结束
			//if (stats.quart) {
			//	throw std::exception("Multiline string not support");
			//}
			//else if (stats.inword) {
			//	msgStream << L"关键字：" << word.c_str() << endl;
			//}
			//else if (stats.indigit) {
			//}
			//stats.NewToken();
			//EndToken();
			stats.level--;
			continue;
			//case '+':
			//case '-':
			//case '*':
			//case '/':
			//case '>':
			//case '<':
			//case '=':
			//case '!':
			//{
			//	arr += ch;
			//	//printf("%3d    ", value(SDL_OPERATOR, _countof(SDL_OPERATOR), *arr.data()));
			//	cout << arr << "  运算符" << endl;
			//	break;
			//}
			//case ',':
			//case '(':
			//case ')':
			//case '[':
			//case ']':
			//{
			//	arr += ch;
			//	printf("%3d    ", value(SDL_SEPARATER, 8, *arr.data()));
			//	cout << arr << "  分隔符" << endl;
			//	break;
			//}
		default:
			break;
		}

		if (stats.inword && (isalnum(ch) || '_' == ch)) {     // 判断是否为有效标识符
			word += ch;

			continue;
		}
		else if (!stats.inword && !stats.indigit && isalpha(ch)) {       // 判断是否为有效标识符
			stats.inword = true;
			word += ch;

			continue;
		}
		else if (stats.indigit) {
			if ((stats.ishex && isxdigit(ch)) ||
				(!stats.ishex && (isdigit(ch) || '.' == ch || (isspace(data[pos + 1]) && ('f' == ch || 'F' == ch))))) {
				word += ch;
			}
			else {
				throw std::bad_exception();
			}

			continue;
		}
		else if (isdigit(ch)) {           //判断是否为数字
			stats.ishex = ('0' == ch && (data[pos + 1] == 'x' || data[pos + 1] == 'X'));
			stats.indigit = true;

			word += ch;

			continue;
		}
		else {
			msgStream << L"无法识别的字符：\'" << ch << "\':！" << endl;
			OutputDebugStringW(msgStream.str().c_str());
			throw std::exception("Unknown character.");
		}
	}
	OutputDebugStringW(msgStream.str().c_str());
}

std::string SdlParser::ReadUntil(char const* pData, ULONG& uPos, size_t stData, char chQuote)
{
	uPos++;
	ULONG uStart = uPos;
	std::stringstream stmWord;
	while ((chQuote != pData[uPos])
		&& ('\r' != pData[uPos])
		&& ('\n' != pData[uPos])) {
		if ('\\' == pData[uPos]) {	// 转义符
			uPos++;
		}
		stmWord << pData[uPos++];
	}

	if (uPos - uStart > stData) {
		throw "Memory overflow";
	}
	//else if ('\n' == pData[uPos] || '\r' == pData[uPos]) {	// 回车换行符不处理
	//	uPos--;
	//}
	else if (chQuote == pData[uPos]) {		// 跳过结尾字符
		uPos++;
	}

	return stmWord.str();
}

std::string SdlParser::ReadDigit(char const* pData, ULONG& uPos, size_t stData)
{
	ULONG uStart = uPos;
	bool isHex = false;
	bool hasDot = false;
	if ('0' == pData[uPos]) {
		if (('x' == pData[uPos + 1] || 'X' == pData[uPos + 1])) {
			isHex = true;
			uPos += 2;
		}
		else if ('.' == pData[uPos]) {
			hasDot = true;
			uPos += 2;
		}
	}

	while ((isHex && isxdigit(pData[uPos]))		// 十六进制
		|| (!isHex && (('.' == pData[uPos] && !hasDot) || isdigit(pData[uPos])))		// 十进制
		|| ((isspace(pData[uPos + 1]) || '}' == pData[uPos + 1]) && ('f' == pData[uPos] || 'F' == pData[uPos]))	// 末尾浮点数标记
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
	//else if ('\n' == pData[uPos] || '\r' == pData[uPos]) {	// 回车换行符不处理
	//	uPos--;
	//}

	return std::string(pData + uStart, pData + uPos);
}

std::string SdlParser::ReadWord(char const* pData, ULONG& uPos, size_t stData)
{
	ULONG uStart = uPos;
	while (isalnum(pData[uPos])		// 字符和数字
		|| ('_' == pData[uPos])		// 下划线
		) {
		uPos++;
	}

	if (uPos - uStart > stData) {
		throw "Memory overflow";
	}
	else if (!isspace(pData[uPos]) && ('}' != pData[uPos])) {
		throw "Invalid syntax";
	}
	//else if ('\n' == pData[uPos] || '\r' == pData[uPos]) {	// 回车换行符不处理
	//	uPos--;
	//}

	return std::string(pData + uStart, pData + uPos);
}

Keywords::Keywords* SdlParser::Analyse2(char const* setName, char const* dataPtr, size_t dataSize)
{
	const int MAX_LEVEL = 8;
	USHORT level = 0;
	ULONG line = 0;
	bool bKeywordSet[MAX_LEVEL]{ 0 };

	auto keywords = std::make_shared<Keywords>(setName);
	Keywords* currentKey = nullptr;

	ULONG pos = 0;
	std::string word;
	while (pos < dataSize) {
		if ('{' == dataPtr[pos]) {
			pos++;
			level++;
			bKeywordSet[level] = false;
		}
		else if ('}' == dataPtr[pos]) {
			if (!bKeywordSet[level]) {
				throw "Keyword not set";
			}
			pos++;
			for (int l = level; l < MAX_LEVEL; l++) {
				bKeywordSet[l] = false;
			}
			level--;
			currentKey = currentKey->Parent();
		}
		else if (';' == dataPtr[pos]) {
			word = ReadUntil(dataPtr, pos, dataSize - pos, 0);
			OutputDebugStringA("Comment: \"");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA("\".\n");
		}
		else if ('\'' == dataPtr[pos] || '\"' == dataPtr[pos]) {	// 字符串
			word = ReadUntil(dataPtr, pos, dataSize - pos, dataPtr[pos]);
			OutputDebugStringA("String: \"");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA("\".\n");
		}
		else if (isdigit(dataPtr[pos])) {	// 数值
			word = ReadDigit(dataPtr, pos, dataSize - pos);
			OutputDebugStringA("Digit: ");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA(".\n");
		}
		else if (isalpha(dataPtr[pos]) || '_' == dataPtr[pos]) {	// 关键词或者标识符
			word = ReadWord(dataPtr, pos, dataSize - pos);
			if (bKeywordSet[level]) {	// 标识符
				OutputDebugStringA("Symbols: \"");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA("\".\n");
			}
			else {						// 关键词
				bKeywordSet[level] = true;
				OutputDebugStringA("Keyword: ");
				OutputDebugStringA(word.c_str());

				currentKey = currentKey->AddChild(word);

				OutputDebugStringA(" appended");
				OutputDebugStringA(".\n");
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
		// 上述方式可以同时处理Windows和Linux的两种CRLF方式
		while (pos < dataSize && isspace(dataPtr[pos])) {
			pos++;
		}
	}

	return keywords;
}

void SdlParser::Analyse(FILE * fpin)
{

}
