#pragma once

using namespace std;

#pragma pack(push, 1)
struct SdlStats {
	USHORT level{ 0 };
	char quart{ 0 };
	bool inword{ false };
	bool indigit{ false };
	bool ishex{ false };
	bool keyget{ false };
	void NewToken() {
		ishex = inword = indigit = keyget = false;
	}
};
#pragma pack(pop)

class SdlParser
{
private:
	const static int KEYWORDS_COUNT = 18;
	static string MTL_KEYWORDS[KEYWORDS_COUNT];
	//static char SDL_SEPARATER[];
	//static char SDL_FILTER[];
	//const int IDENTIFIER = 100;         //标识符值  
	//const int CONSTANT = 101;           //常数值  
	//const int FILTER_VALUE = 102;       //过滤字符值

	/**判断是否为关键字**/
	bool IsKeyword(string word) {
		for (int i = 0; i < KEYWORDS_COUNT; i++) {
			if (MTL_KEYWORDS[i] == word) {
				return true;
			}
		}
		return false;
	}

	///**判断是否为分隔符**/
	//bool IsSeparater(char ch) {
	//	for (int i = 0; i < _countof(SDL_SEPARATER); i++) {
	//		if (SDL_SEPARATER[i] == ch) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	///**判断是否为运算符**/
	//bool IsOperator(char ch) {
	//	for (int i = 0; i < 8; i++) {
	//		if (OPERATOR[i] == ch) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	///**判断是否为过滤符**/
	//bool IsFilter(char ch) {
	//	for (int i = 0; i < _countof(SDL_FILTER); i++) {
	//		if (SDL_FILTER[i] == ch) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	///**判断是否为大写字母**/
	//bool IsUpLetter(char ch) {
	//	return (ch >= 'A' && ch <= 'Z');
	//}

	///**判断是否为数字**/
	//bool IsDigit(char ch) {
	//	return (ch >= '0' && ch <= '9');
	//}

	///**返回每个字的值**/
	//template <class T>
	//int value(T *a, int n, T str) {
	//	for (int i = 0; i < n; i++) {
	//		if (a[i] == str) return i + 1;
	//	}
	//	return -1;
	//}

	void Analyse(FILE * fpin);

public:
	SdlParser();
	~SdlParser();
	/**词法分析**/
	void Analyse(byte const* data, size_t size);
};

