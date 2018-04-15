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
void SdlParser::Analyse(byte const* data, size_t size) {
	SdlStats stats;
	std::wostringstream msgStream;

	char ch;
	UINT32 pos = 0;
	string word;

	ch = data[0];
	while (pos < size) {	// ��ʼһ��
		ch = data[pos++];

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
				EndLine();

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
			EndLine();

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

void SdlParser::Analyse(FILE * fpin)
{

}
