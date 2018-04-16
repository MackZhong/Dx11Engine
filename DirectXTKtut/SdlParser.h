#pragma once

using namespace std;

class IParseClient {
public:
	virtual void Process(std::string const& keyword, std::string const& value = "", bool isSymbol = false) = 0;
};

using ParseClient = std::shared_ptr<IParseClient>;

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
	Keywords() {};

public:
	enum KeyType {
		ROOT = 'r',
		KEY = 'k',
		STR = 'p'
	};
	enum ValueType {
		STRING = 0x0,
		DIGIT = 0x0,
		OBJECT = 0x0,
	};
	using KwPtr = std::shared_ptr<Keywords>;
	using KwMap = std::map<std::string, KwPtr>;
	using KwMapPtr = std::shared_ptr<KwMap>;
	//using KwSetIterator = KwMap::iterator;

	Keywords(std::string const& kw, KeyType kt, Keywords* const& p = nullptr) : m_Name(kw), m_KeyType(kt), m_Parent(p) {};
	Keywords* Parent() { return m_Parent; };
	std::string Name() { return m_Name; };

	KwPtr Get(const std::string& kw) {
		if (kw.empty() || m_SubKeywords.empty())
			return nullptr;
		//auto find = ;
		//if (m_SubKeywords.end() == find)
		//	return nullptr;
		//return nullptr;
		return m_SubKeywords.find(kw)->second;
	}

	KwPtr AddChild(const std::string& kw, KeyType kt) {
		if (kw.empty())
			return nullptr;
		//if (!m_SubKeywords)
		//	m_SubKeywords = std::make_shared<KwMap>();
		if (m_SubKeywords.find(kw) == m_SubKeywords.end()) {
			m_SubKeywords.emplace(kw, std::make_shared<Keywords>(kw, kt, this));
		}
		return m_SubKeywords.at(kw);
	}

	void Combine(const KwPtr& other) {
		if (this->m_Name != other->m_Name) {
			throw "Can't combine different keyword set";
		}
		//if (other->m_SubKeywords.empty())
		//	return;
		//if (this->m_SubKeywords.empty()) {
		//	this->m_SubKeywords = other->m_SubKeywords;
		//	return;
		//}

		for (auto k = other->m_SubKeywords.begin(); k != other->m_SubKeywords.end(); k++) {
			auto me = this->m_SubKeywords.find(k->first);
			if (this->m_SubKeywords.end() == me) {
				auto ist = this->m_SubKeywords.insert(*k);
				auto sec = ist.first->second;
				sec->m_Parent = this;
				//auto added = this->m_SubKeywords.emplace(k->first, k->second);
				//added.first->second->m_Parent = me;
			}
			else {
				//me->Combine(k->second);
				me->second->Combine(k->second);
			}
		}
	}

	bool operator<(const Keywords& right) const { return m_Name < right.m_Name; }

	friend std::ostream& operator<<(std::ostream& os, const Keywords& me) {
		os << "{ " << me.m_Name << "(" << (char)me.m_KeyType << ") " << std::endl;
		for (auto k = me.m_SubKeywords.begin(); k != me.m_SubKeywords.end(); k++) {
			os << "\t" << *(k->second);
		}
		os << " }" << std::endl;
		return os;
	}

	std::string toClass(int level = 0) {
		std::stringstream stm;
		std::string preSpace;
		for (int l = 0; l < level; l++) { preSpace += '\t'; }
		stm << preSpace;

		if (m_SubKeywords.empty())
		{
			stm << "int " << m_Name << ";" << std::endl;
			return stm.str();
		}

		if (ROOT == m_KeyType) {
			stm << "namespace ";
		}
		else {
			stm << "class ";
		}
		stm << m_Name << " {" << std::endl;

		for (auto k = m_SubKeywords.begin(); k != m_SubKeywords.end(); k++) {
			stm << k->second->toClass(level + 1);
		}

		stm << preSpace << "};" << std::endl;

		return stm.str();
	}

private:
	Keywords * m_Parent;
	KeyType m_KeyType;
	std::string m_Name;
	KwMap m_SubKeywords;
};

class SdlParser
{
private:
	const static int KEYWORDS_COUNT = 18;
	static string MTL_KEYWORDS[KEYWORDS_COUNT];

	void Analyse(FILE * fpin);

	std::string ReadUntil(char const* pData, ULONG& uPos, size_t stData, char chQuote);
	std::string ReadDigit(char const* pData, ULONG& uPos, size_t stData);
	std::string ReadWord(char const* pData, ULONG& uPos, size_t stData);

	void ProcessCommand(std::string const& command);
public:
	SdlParser();
	~SdlParser();

	/**´Ê·¨·ÖÎö**/
	Keywords::KwPtr Analyse(char const* setName, char const* dataPtr, size_t dataSize, ParseClient client = nullptr);

	Keywords::KwPtr Analyse2(char const* setName, char const* dataPtr, size_t dataSize);
};
