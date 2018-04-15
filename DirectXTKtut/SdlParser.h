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
	//const int IDENTIFIER = 100;         //��ʶ��ֵ  
	//const int CONSTANT = 101;           //����ֵ  
	//const int FILTER_VALUE = 102;       //�����ַ�ֵ

	/**�ж��Ƿ�Ϊ�ؼ���**/
	bool IsKeyword(string word) {
		for (int i = 0; i < KEYWORDS_COUNT; i++) {
			if (MTL_KEYWORDS[i] == word) {
				return true;
			}
		}
		return false;
	}

	///**�ж��Ƿ�Ϊ�ָ���**/
	//bool IsSeparater(char ch) {
	//	for (int i = 0; i < _countof(SDL_SEPARATER); i++) {
	//		if (SDL_SEPARATER[i] == ch) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	///**�ж��Ƿ�Ϊ�����**/
	//bool IsOperator(char ch) {
	//	for (int i = 0; i < 8; i++) {
	//		if (OPERATOR[i] == ch) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	///**�ж��Ƿ�Ϊ���˷�**/
	//bool IsFilter(char ch) {
	//	for (int i = 0; i < _countof(SDL_FILTER); i++) {
	//		if (SDL_FILTER[i] == ch) {
	//			return true;
	//		}
	//	}
	//	return false;
	//}

	///**�ж��Ƿ�Ϊ��д��ĸ**/
	//bool IsUpLetter(char ch) {
	//	return (ch >= 'A' && ch <= 'Z');
	//}

	///**�ж��Ƿ�Ϊ����**/
	//bool IsDigit(char ch) {
	//	return (ch >= '0' && ch <= '9');
	//}

	///**����ÿ���ֵ�ֵ**/
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
	/**�ʷ�����**/
	void Analyse(byte const* data, size_t size);
};

