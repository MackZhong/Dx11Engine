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

public:
	using KwPtr = std::shared_ptr<Keywords>;
	using KwMap = std::map<std::string, KwPtr>;
	using KwMapPtr = std::shared_ptr<KwMap>;
	//using KwSetIterator = KwMap::iterator;

	Keywords(std::string const& kw, Keywords* const& p = nullptr) : m_Name(kw), m_Parent(p) {};
	KwPtr Parent() { return m_Parent; };
	//KwMapPtr Children() { return m_SubKeywords; };

	KwPtr Get(const std::string& kw) {
		if (kw.empty() || m_SubKeywords.empty())
			return nullptr;
		//auto find = ;
		//if (m_SubKeywords.end() == find)
		//	return nullptr;
		//return nullptr;
		return m_SubKeywords.find(kw)->second;
	}

	KwPtr AddChild(const std::string& kw) {
		if (kw.empty())
			return nullptr;
		//if (!m_SubKeywords)
		//	m_SubKeywords = std::make_shared<KwMap>();
		if (m_SubKeywords.find(kw) == m_SubKeywords.end()) {
			m_SubKeywords.emplace(kw, std::make_shared<Keywords>(kw, this));
		}
		return m_SubKeywords.at(kw);
	}

	void Combine(const KwPtr& other) {
		if (this->m_Name != other->m_Name) {
			throw "Can't combine different keyword set";
		}
		if (other->m_SubKeywords.empty())
			return;
		if (this->m_SubKeywords.empty()) {
			this->m_SubKeywords = other->m_SubKeywords;
			return;
		}

		for (auto k = other->m_SubKeywords.begin(); k != other->m_SubKeywords.end(); k++) {
			auto me = this->Get(k->first);
			if (nullptr == me) {
				auto added = this->m_SubKeywords.emplace(k->first, k->second);
				added.first->second->m_Parent = me;
			}
			else {
				me->Combine(k->second);
			}
		}
	}

	bool operator<(const Keywords& right) const { return m_Name < right.m_Name; }

	friend std::ostream& operator<<(std::ostream& os, const Keywords& me) {
		os << "{ " << me.m_Name;
		for (auto k = me.m_SubKeywords.begin(); k != me.m_SubKeywords.end(); k++) {
			os << *(k->second);
		}
		os << " }";
		return os;
	}

private:
	KwPtr m_Parent;
	std::string m_Name;
	KwMap m_SubKeywords;
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

	Keywords::KwPtr Analyse2(char const* setName, char const* dataPtr, size_t dataSize);
};
