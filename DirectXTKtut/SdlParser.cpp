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
void SdlParser::Analyse(char const* data, size_t size) {
	SdlStats stats;
	std::wostringstream msgStream;

	char ch;
	UINT32 pos = 0;
	string word;

	ch = data[0];
	int iLine = 1;
	while (pos < size) {	// ��ʼһ��
		ch = data[pos++];
		if (!isascii(ch)) {
			throw "None ascii code not supported";
		}

		if (isspace(ch)) {			// �ж��Ƿ�Ϊ�ָ��ַ�
			if (stats.inword) {		// һ�����ʽ���
				if (!stats.keyget) {	// �ؼ���
					if (IsKeyword(word)) {
						msgStream << L"�ؼ��֣�" << word.c_str() << endl;
						stats.keyget = true;
					}
					else {
						throw std::exception("Invalid keyword");
					}
				}
				else {			// ��ʶ��
					msgStream << L"\t��ʶ����" << word.c_str() << endl;
				}

				word.clear();
				stats.inword = false;
			}
			else if (stats.indigit) {	// ��ֵ
				msgStream << L"\t��ֵ����" << word.c_str() << endl;

				word.clear();
				stats.indigit = false;
				stats.ishex = false;
			}
			else if ('\r' == ch || '\n' == ch) {
				//EndLine();

				if (stats.quart) {
					throw std::exception("Multiline string not support");
				}
				stats.NewToken();		// ��ĩ��ʼ������֧�ַ������
				word.clear();
				while (isspace(data[pos++ + 1])) {}	// ����������
			}
			continue;
		}
		else if (stats.quart == ch) {	// �ַ����������ȴ���������������ǿմ����ı���Ӧ���С�0��ֵ
			if (word.empty()) {
				msgStream << L"\t�ַ���Ϊ��" << endl;
			}
			else {
				msgStream << L"\t�ַ�����" << word.c_str() << "" << endl;
			}

			stats.quart = 0;
			word.clear();
			continue;
		}
		else if (stats.quart) {		// ��ȡ�ַ���
			if ('\\' == ch) {		// ת���
				ch = data[pos++];	// Ĭ������
			}
			word += ch;

			continue;
		}
		else switch (ch) {
		case '\r':		// �س�
		case '\n':		// ����
		{
			//EndLine();

			if (stats.quart) {
				throw std::exception("Multiline string not support");
			}
			stats.NewToken();		// ��ĩ��ʼ������֧�ַ������
			word.clear();
			while (isspace(data[pos++ + 1])) {}	// ����������

			continue;
		}
		case ';':	// ע�͵���β
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
		case '\'':	// �������ַ���
			stats.quart = '\'';
			continue;
		case '\"':	// ˫�����ַ���
			stats.quart = '\"';
			continue;
		case '{':	// ����鿪ʼ
			stats.level++;
			stats.NewToken();
			continue;
		case '}':	// ��������
			//if (stats.quart) {
			//	throw std::exception("Multiline string not support");
			//}
			//else if (stats.inword) {
			//	msgStream << L"�ؼ��֣�" << word.c_str() << endl;
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
			//	cout << arr << "  �����" << endl;
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
			//	cout << arr << "  �ָ���" << endl;
			//	break;
			//}
		default:
			break;
		}

		if (stats.inword && (isalnum(ch) || '_' == ch)) {     // �ж��Ƿ�Ϊ��Ч��ʶ��
			word += ch;

			continue;
		}
		else if (!stats.inword && !stats.indigit && isalpha(ch)) {       // �ж��Ƿ�Ϊ��Ч��ʶ��
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
		else if (isdigit(ch)) {           //�ж��Ƿ�Ϊ����
			stats.ishex = ('0' == ch && (data[pos + 1] == 'x' || data[pos + 1] == 'X'));
			stats.indigit = true;

			word += ch;

			continue;
		}
		else {
			msgStream << L"�޷�ʶ����ַ���\'" << ch << "\':��" << endl;
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

	while ((isHex && isxdigit(pData[uPos]))		// ʮ������
		|| (!isHex && (('.' == pData[uPos] && !hasDot) || isdigit(pData[uPos])))		// ʮ����
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
		else if ('\'' == dataPtr[pos] || '\"' == dataPtr[pos]) {	// �ַ���
			word = ReadUntil(dataPtr, pos, dataSize - pos, dataPtr[pos]);
			OutputDebugStringA("String: \"");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA("\".\n");
		}
		else if (isdigit(dataPtr[pos])) {	// ��ֵ
			word = ReadDigit(dataPtr, pos, dataSize - pos);
			OutputDebugStringA("Digit: ");
			OutputDebugStringA(word.c_str());
			OutputDebugStringA(".\n");
		}
		else if (isalpha(dataPtr[pos]) || '_' == dataPtr[pos]) {	// �ؼ��ʻ��߱�ʶ��
			word = ReadWord(dataPtr, pos, dataSize - pos);
			if (bKeywordSet[level]) {	// ��ʶ��
				OutputDebugStringA("Symbols: \"");
				OutputDebugStringA(word.c_str());
				OutputDebugStringA("\".\n");
			}
			else {						// �ؼ���
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
		// ������ʽ����ͬʱ����Windows��Linux������CRLF��ʽ
		while (pos < dataSize && isspace(dataPtr[pos])) {
			pos++;
		}
	}

	return keywords;
}

void SdlParser::Analyse(FILE * fpin)
{

}
