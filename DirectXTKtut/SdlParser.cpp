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
void SdlParser::Analyse(byte const* data, size_t size) {
	SdlStats stats;
	std::wostringstream msgStream;

	char ch;
	UINT32 pos = 0;
	string word;

	ch = data[0];
	while (pos < size) {	// 开始一行
		ch = data[pos++];

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
				EndLine();

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
			EndLine();

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
				EndToken();
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
			EndToken();
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

void SdlParser::Analyse(FILE * fpin)
{

}
