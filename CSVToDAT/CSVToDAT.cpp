#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <unordered_map>
#include <tuple>

#define writeBin(x) write(reinterpret_cast<const char *>(&##x##), sizeof(##x##))
#define readBin(x) read(reinterpret_cast<char *>(&##x##), sizeof(##x##))

using uchar = unsigned char;
using ushort = unsigned short;
using uint = unsigned int;
using ulonglong = unsigned long long;

enum class TYPE : int {
	ERROR,
	BOOL,
	UCHAR,
	CHAR,
	USHORT,
	SHORT,
	UINT,
	INT,
	ULONGLONG,
	LONGLONG,
	FLOAT,
	DOUBLE,
	STRING,
	SKIP
};

//										name of enum		repre type  index of enum    map of value-name and value-values
using enum_database = std::unordered_map<std::string, std::tuple<TYPE, int, std::unordered_map<std::string, std::string>>>;
using enum_by_index = std::unordered_map<int, enum_database::iterator>;

uint64_t constexpr mix(char m, uint64_t s) {
	return ((s << 7) + ~(s >> 3)) + ~m;
}

uint64_t constexpr hash(const char* m) {
	return (*m) ? mix(*m, hash(m + 1)) : 0;
}

bool isValidTypeForEnum(TYPE type);
bool getNextLine(std::ifstream& stream, std::string& line, size_t& lineN);
void lowerString(std::string& string);
void upperString(std::string& string);
void cutLeadingSpaces(std::string& string);
void cutTrailingSpaces(std::string& string);
void cutBothSpaces(std::string& string);
TYPE stringToType(const std::string& string);

template<typename Target> Target stringToNumberCheck(const std::string& string, size_t lineN, size_t valueN) {
	Target t;
	std::stringstream ss{string};
	ss >> t;

	if (ss.fail()) {
		std::cout << "Cannot convert \"" << string << "\" to any number at cell [" << valueN << ", " << lineN << "]. Press enter to exit.";
		std::string pic;
		std::cin >> pic;
		std::exit(1);
	}
	return t;
};

template<typename Target, typename Current> bool tryConvert(Target& t, Current c, size_t lineN, size_t valueN) {
	constexpr auto min = std::numeric_limits<Target>::min();
	constexpr auto max = std::numeric_limits<Target>::max();
	if (c < min || c > max) {
		std::cout << "Cannot convert '" << c << "' to '" << typeid(Target).name() << "' at cell [" << valueN << ", " << lineN << "] - outside of range <" << (double)min << ", " << (double)max << ">.\n";
		std::cout << "Press enter to exit.";
		std::string pic;
		std::cin >> pic;
		std::exit(1);
		return false;
	}
	//Can convert safely
	t = (Target)c;
	return true;
};
void saveValue(TYPE type, const std::string& string, std::ofstream& stream, const enum_database& edtb, enum_by_index& ebi, size_t lineN, size_t valueN);


int main(int argc, char* argv[]) {
	std::string stem;
	std::cout << "Enter CSV file name stem (no extension): " << std::endl;
	std::cin >> stem;

	std::ifstream inputfile(stem + ".csv", std::ios::in);
	if (inputfile.fail()) {
		std::cout << "Input file not found - press enter to exit.";
		std::cin >> stem;
		return 1;
	}
	std::ofstream outputfile(stem + ".dat", std::ios::binary | std::ios::out);
	if (outputfile.fail()) {
		std::cout << "Failed to create output file - press enter to exit.";
		std::cin >> stem;
		return 1;
	}

	std::string line;
	std::stringstream lineStream;
	std::string cell;
	std::vector<TYPE> types;
	size_t counter;
	size_t lineN = 0u;
	size_t enumIndex = 100;
	enum_database enums;
	enum_by_index enumByIndex;

	//Reading tables
	while (getNextLine(inputfile, line, lineN)) {
		if (line == "") {
			continue;//Empty lines
		}
		//Check for enum declaration
		lineStream = std::stringstream(line);
		std::getline(lineStream, cell, ',');
		cutBothSpaces(cell);
		upperString(cell);
		if (cell == "ENUM" || cell == "ENUMERATION") {
			//Enum declaration found!
			std::getline(lineStream, cell, ',');
			cutBothSpaces(cell);
			std::string name = cell;
			if (enums.count(name) != 0u) {
				std::cout << "Enum '" << name << "' was already defined, now redefining at line " << lineN << ". Cannot convert - press enter to exit.";
				std::cin >> name;
				return 1;
			}
			enums[name];//Name
			auto it = enums.find(name);
			std::get<1>(it->second) = enumIndex;
			enumByIndex.insert(std::make_pair(enumIndex++, it));
			if (std::getline(lineStream, cell, ',')) {//Representation type
				TYPE type = stringToType(cell);
				if (type != TYPE::ERROR) {
					if (isValidTypeForEnum(type)) {
						std::get<0>(enums[name]) = type;
					} else {
						std::cout << "Enum can  be represented by any integral (both signed and unsigned) types.\n";
						std::cout << "Tried to represent enum '" << name << "' declared at line " << lineN << " by illegal type.\n";
						std::cout << "Cannot convert - press enter to exit.";
						std::cin >> name;
						return 1;
					}
				} else {
					std::cout << "Erroneous representation type '" << cell << "' of enum '" << name << "' declared at line " << lineN << ". Cannot convert - press enter to exit.";
					std::cin >> name;
					return 1;
				}
			} else {
				std::get<0>(enums[name]) = TYPE::INT;//Default
			}
			//Reading possible values of this enum
			long long lastVal = -1;
			while (getNextLine(inputfile, line, lineN)) {
				if (line == "") {
					break;//End of possible values
				}
				lineStream = std::stringstream(line);
				std::getline(lineStream, cell, ',');
				cutBothSpaces(cell);
				std::string valueName = cell;
				if (std::getline(lineStream, cell, ',')) {
					lastVal = stringToNumberCheck<long long>(cell, lineN, 2u);
				} else {
					lastVal++;
				}
				std::get<2>(enums[name]).insert(std::make_pair(valueName, std::to_string(lastVal)));
			}
			continue;
		}


		lineStream = std::stringstream(line);
		//Reading types
		types.clear();
		while (std::getline(lineStream, cell, ',')) {
			TYPE type = stringToType(cell);
			if (type != TYPE::ERROR) {
				types.push_back(type);
			} else {
				//It is not a basic type, still can be one of enums
				std::string enumName = cell;
				cutBothSpaces(enumName);
				auto it = enums.find(enumName);
				if (it != enums.end()) {
					//It is an enum
					types.push_back((TYPE)std::get<1>(it->second));
				} else {
					std::cout << "Erroneous type '" << cell << "' at cell [" << (types.size() + 1) << ", " << lineN << "]. Cannot convert - press enter to exit.";
					std::cin >> stem;
					return 1;
				}
			}
		}
		//Converting values
		while (getNextLine(inputfile, line, lineN)) {
			if (line == "") {
				break;//End of table
			}
			counter = 0u;
			lineStream = std::stringstream{line};
			while (std::getline(lineStream, cell, ',') && counter < types.size()) {
				saveValue(types[counter], cell, outputfile, enums, enumByIndex, lineN, counter + 1);
				counter++;
			}
			if (counter != types.size()) {
				std::cout << "Missing values at line #" << (lineN) << ". Cannot convert - press enter to exit.";
				std::cin >> stem;
				return 1;
			}
		}
	}
	return 0;
}

bool isValidTypeForEnum(TYPE type) {
	switch (type) {
	case TYPE::BOOL:
	case TYPE::UCHAR:
	case TYPE::CHAR:
	case TYPE::USHORT:
	case TYPE::SHORT:
	case TYPE::UINT:
	case TYPE::INT:
	case TYPE::ULONGLONG:
	case TYPE::LONGLONG:
	case TYPE::SKIP:
		return true;
	default:
		return false;
	}
}

bool getNextLine(std::ifstream& stream, std::string& line, size_t& lineN) {
	do {
		if (!std::getline(stream, line)) {
			return false;
		}
		lineN++;
	} while ((line.size() >= 2u) ? (line[0] == '/' && line[1] == '/') : false);

	return true;
}

void lowerString(std::string& string) {
	for (auto& character : string) {
		character = tolower(character);
	}
};

void upperString(std::string& string) {
	for (auto& character : string) {
		character = toupper(character);
	}
};

void cutLeadingSpaces(std::string& string) {
	uint pos = 0u;
	for (size_t i = 0u; i < string.size(); ++i) {
		if (string[i] == ' ' || string[i] == '\t') {
			pos = i + 1;
		} else {
			break;
		}
	}
	string = string.substr(pos);
};

void cutTrailingSpaces(std::string& string) {
	if (string.empty()) { return; }
	uint pos = string.size();
	for (size_t i = string.size() - 1; i >= 0; --i) {
		if (string[i] == ' ' || string[i] == '\t') {
			pos = i;
		} else {
			break;
		}
	}
	string = string.substr(0u, pos);
};

void cutBothSpaces(std::string& string) {
	cutLeadingSpaces(string);
	cutTrailingSpaces(string);
};

TYPE stringToType(const std::string& string) {
	std::string str = string;
	cutBothSpaces(str);
	upperString(str);

	switch (hash(str.c_str())) {
	case hash("BOOL"):
	case hash("BOOLEAN"):
		return TYPE::BOOL;
	case hash("UCHAR"):
	case hash("UBYTE"):
	case hash("U8"):
		return TYPE::UCHAR;
	case hash("CHAR"):
	case hash("BYTE"):
	case hash("S8"):
		return TYPE::CHAR;
	case hash("USHORT"):
	case hash("U16"):
		return TYPE::USHORT;
	case hash("SHORT"):
	case hash("S16"):
		return TYPE::SHORT;
	case hash("UINT"):
	case hash("ULONG"):
	case hash("U32"):
		return TYPE::UINT;
	case hash("INT"):
	case hash("LONG"):
	case hash("S32"):
		return TYPE::INT;
	case hash("ULONG LONG"):
	case hash("ULONGLONG"):
	case hash("U64"):
		return TYPE::ULONGLONG;
	case hash("LONG LONG"):
	case hash("LONGLONG"):
	case hash("S64"):
		return TYPE::LONGLONG;
	case hash("FLOAT"):
	case hash("SINGLE"):
	case hash("F32"):
		return TYPE::FLOAT;
	case hash("DOUBLE"):
	case hash("REAL"):
	case hash("F64"):
		return TYPE::DOUBLE;
	case hash("STRING"):
	case hash("TEXT"):
		return TYPE::STRING;
	case hash("SKIP"):
	case hash("COMMENT"):
		return TYPE::SKIP;
	default:
		return TYPE::ERROR;
	}
}

void saveValue(TYPE type, const std::string& string, std::ofstream& stream, const enum_database& edtb, enum_by_index& ebi, size_t lineN, size_t valueN) {
	static bool BOOL;
	static uint8_t UCHAR;
	static int8_t CHAR;
	static uint16_t USHORT;
	static int16_t SHORT;
	static uint32_t UINT;
	static int32_t INT;
	static uint64_t ULONGLONG;
	static int64_t LONGLONG;
	static float FLOAT;
	static double DOUBLE;
	static std::string STRING;

	std::string str = string;
	switch (type) {
	case TYPE::SKIP:
		//Not saving skip values
		return;
	case TYPE::ERROR:
		//Should not get here
		return;
	case TYPE::BOOL:
		cutBothSpaces(str);
		upperString(str);
		switch (hash(str.c_str())) {
		case hash("TRUE"):
		case hash("T"):
		case hash("YES"):
		case hash("Y"):
		case hash("1"):
			BOOL = true;
			stream.writeBin(BOOL);
			return;
		case hash("FALSE"):
		case hash("F"):
		case hash("NO"):
		case hash("N"):
		case hash("0"):
			BOOL = false;
			stream.writeBin(BOOL);
			return;
		default:
			std::cout << "Cannot convert \"" << string << "\" to 'bool' at cell [" << valueN << ", " << lineN << "]. Press enter to exit.";
			std::cin >> STRING;
			std::exit(1);
		}
	case TYPE::UCHAR:
		LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
		tryConvert<uchar, decltype(LONGLONG)>(UCHAR, LONGLONG, lineN, valueN);
		stream.writeBin(UCHAR);
		return;
	case TYPE::CHAR:
		cutBothSpaces(str);
		if (str.size() == 1u && !isdigit(str[0u])) {
			stream.writeBin(str[0u]);
		} else {
			LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
			tryConvert<int8_t, decltype(LONGLONG)>(CHAR, LONGLONG, lineN, valueN);
			stream.writeBin(CHAR);
		}
		return;
	case TYPE::USHORT:
		LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
		tryConvert<decltype(USHORT), decltype(LONGLONG)>(USHORT, LONGLONG, lineN, valueN);
		stream.writeBin(USHORT);
		return;
	case TYPE::SHORT:
		LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
		tryConvert<decltype(SHORT), decltype(LONGLONG)>(SHORT, LONGLONG, lineN, valueN);
		stream.writeBin(SHORT);
		return;
	case TYPE::UINT:
		LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
		tryConvert<decltype(UINT), decltype(LONGLONG)>(UINT, LONGLONG, lineN, valueN);
		stream.writeBin(UINT);
		return;
	case TYPE::INT:
		LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
		tryConvert<decltype(INT), decltype(LONGLONG)>(INT, LONGLONG, lineN, valueN);
		stream.writeBin(INT);
		return;
	case TYPE::LONGLONG:
		LONGLONG = stringToNumberCheck<long long>(str, lineN, valueN);
		stream.writeBin(LONGLONG);
		return;
	case TYPE::ULONGLONG:
		ULONGLONG = stringToNumberCheck<ulonglong>(str, lineN, valueN);
		stream.writeBin(ULONGLONG);
		return;
	case TYPE::FLOAT:
		FLOAT = stringToNumberCheck<float>(str, lineN, valueN);
		stream.writeBin(FLOAT);
		return;
	case TYPE::DOUBLE:
		DOUBLE = stringToNumberCheck<double>(str, lineN, valueN);
		stream.writeBin(DOUBLE);
		return;
	case TYPE::STRING:
		stream.write(str.c_str(), str.size());
		return;
	}
	//If we got here, we are converting enum value
	auto itt = ebi.find((int)type);
	if (itt == ebi.end()) {
		//Should not ever happen
		std::cout << "How could this happen? Press enter to exit.";
		std::cin >> STRING;
		std::exit(1);
	}
	cutBothSpaces(str);
	auto it = std::get<2>(itt->second->second).find(str);
	if (it != std::get<2>(itt->second->second).end()) {
		//We found this enum value
		saveValue(std::get<0>(itt->second->second), it->second, stream, edtb, ebi, lineN, valueN);
	} else {
		std::cout << "Expression '" << str << "' at cell [" << valueN << ", " << lineN << "] is not a valid value of enum '" << itt->first << "'.\nPress enter to exit.";
		std::cin >> STRING;
		std::exit(1);
	}
};