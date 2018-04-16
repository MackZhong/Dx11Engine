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


class Keywords {
private:
	Keywords* Get(std::string kw) {
		if (kw.empty())
			return nullptr;
		if (!m_SubKeywords)
			return nullptr;
		auto find = m_SubKeywords->find(kw);
		if (m_SubKeywords->end() == find)
			return nullptr;
		//return nullptr;
		return (Keywords*)(&(*find));
	}

public:
	using KeywordsPtr = std::shared_ptr<Keywords>;
	using KwSet = std::set<Keywords>;
	using KwSetPtr = std::shared_ptr<KwSet>;
	using KwSetIterator = KwSet::iterator;

	Keywords(std::string kw, Keywords* p = nullptr) : m_Name(kw), m_Parent(p) {};
	KeywordsPtr Parent() { return m_Parent; };
	KwSetPtr Children() { return m_SubKeywords; };

	Keywords* AddChild(std::string kw) {
		if (!m_SubKeywords)
			m_SubKeywords = std::make_shared<KwSet>();
		if (kw.empty())
			return nullptr;
		m_SubKeywords->emplace(kw, this);
		return (Keywords*)(&(*m_SubKeywords->find(kw)));
	}

	void Combine(Keywords* const other) {
		if (this->m_Name != other->m_Name) {
			throw "Can't combine different keyword set";
		}
		if (nullptr == other->m_SubKeywords)
			return;
		if (nullptr == this->m_SubKeywords) {
			this->m_SubKeywords = other->m_SubKeywords;
			return;
		}

		for (auto k = other->m_SubKeywords->begin(); k != other->m_SubKeywords->end(); k++) {
			auto it = this->Get(k->m_Name);
			if (nullptr == it) {
				this->m_SubKeywords->emplace(*k);
			}
			else {
				it->Combine(other->Get(k->m_Name));
			}
		}
	}

	bool operator<(const Keywords& right) const { return m_Name < right.m_Name; }

	friend std::ostream& operator<<(std::ostream& os, const Keywords& me) {
		os << "{ " << me.m_Name;
		if (me.m_SubKeywords) {
			for (auto k = me.m_SubKeywords->begin(); k != me.m_SubKeywords->end(); k++) {
				os << *k;
			}
		}
		os << " }";
		return os;
	}

private:
	KeywordsPtr m_Parent;
	std::string m_Name;
	KwSetPtr m_SubKeywords;
};

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

	std::string ReadUntil(char const* pData, ULONG& uPos, size_t stData, char chQuote);
	std::string ReadDigit(char const* pData, ULONG& uPos, size_t stData);
	std::string ReadWord(char const* pData, ULONG& uPos, size_t stData);

public:
	SdlParser();
	~SdlParser();
	/**词法分析**/
	void Analyse(char const* data, size_t size);

	//using KeywordsSet = std::set<KeywordSet>;

	//using KeywordArray = std::vector<std::string>;
	//using LevelKeywords = std::map<USHORT, KeywordArray>;

	Keywords::KeywordsPtr Analyse2(char const* setName, char const* dataPtr, size_t dataSize);
};
