/*
[Window]
fullscreen=true
width=1920
height=1080
[Graphics]
; настройки графики
quality=high ; качество графики
filtering=x16

Файл может содержать:
    - комментарии - от символа «;» до конца строки;
    - секции - строки, содержащие название секции в квадратных скобках;
    - значения - строки вида «ключ=значение»;
    - пустые строки;
    - ошибочные строки - секции без имени, значения без ключа, ключ без значения, множественные '[', ']', '=' и т.д.

Лишние пробелы по краям ключей/значений должны отбрасываться
Ошибочные строки не должны быть обработаны и включены в конфигурацию
Содержимое одинаковых секций объединяется в одну
При наличии одинаковых ключей - берется значение последнего ключа

Пример использования:
IniFile ini("config.ini");
bool fs = ini.readBool("Window", "fullscreen", false); // true from ini
int uiScale = ini.readInt("Window", "UIScale", 1); // 1 from default
*/


#include "IniFile.h"

//! Используется:
//using KeysMap = map<string, string>;
//using SectionsMap = map<string, KeysMap>;
//const vector<string> trueValues = { "true", "yes", "y", "1" };
//const std::string filePath;     //< Путь до файла конфигурации
//SectionsMap data;               //< Значения


//удаление проеблов
void trim(string &s) {
    int x;
    while (s.find(' ') != string::npos) {
        x = s.find(' ');
        s.replace(x, 1, "");
    }
}

//Удалениек коментов
void delete_comment(string &str) {
    int x = str.find(';');
    if (x != string::npos) {
        str.erase(x, str.size() - x);
    }
}

void check_section(string &str) {
    //Проверка, что секция не []
    if (str.length() <= 2) {
        str.erase(0, str.length());
    }

//    // Проверка на что-то[] или []что-то
    if ((str[0] != '[') || (str[str.length() - 1] != ']')) {
        str.erase(0, str.length());
    }
    //проверка на ][
    if (str.find(']') < str.find('[')) {
        str.erase(0, str.length());
    }

    //проверка на [[] []]
    int n1 = 1, n2 = 1;
    for (int i = 1; i < str.length() - 1; i++) {
        if (str[i] == ']') {
            n1++;
        } else if (str[i] == '[') {
            n2++;
        }
        if ((!isalpha(str[i])) && (!isdigit(str[i])) && (str[i] != '.')) {
            str.erase(0, str.length());
            break;
        }
        if ((n1 > 1) || (n2 > 1)) {
            str.erase(0, str.length());
            break;
        }
    }
}

void сheck_key(string &str) {
    //Проверка на существование ключ значение
    if ((str[0] == '=') || (str[str.length() - 1] == '=')) {
        str.erase(0, str.length());
    }
    int x = 0;
    for (int i = 0; i < str.length(); i++) {
        if (str[i] == '=') {
            x++;
        }
        if ((!isalpha(str[i])) && (!isdigit(str[i])) && (str[i] != '=') && (str[i] != '.')) {
            str.erase(0, str.length());
            break;
        }
    }
    if (x != 1) {
        str.erase(0, str.length());
    }
}

//проверка секция ли
int Is_section(const string s) {
    if ((s.find('[') != string::npos) || (s.find(']') != string::npos)) {
        return 1; //Секция
    }
    return 2; //внутри секции
}

int Checkeble(string &str) {
    trim(str); //удаление пробелов
    delete_comment(str); //удаление коментариев
    if (str.empty()) {
        return 0; //Пустая
    }
    int x = Is_section(str);

    if (x == 1) { // Это секция
        check_section(str);

    } else if (x == 2) { //Внутри секции
        сheck_key(str);
    }

    if (str.empty()) {
        return 0; //Пустая
    } else if (x == 1) {
        return 1; // Это секция и строка норм
    } else {
        return 2; // Внтури секции и норм
    }
}

string &get_section_name(string &str) {
    str.erase(0, 1);
    str.erase(str.size() - 1, 1);
    return str;
}

/// Конструктор
// fileName Путь до файла конфигурации

IniFile::IniFile(const string &path) : filePath(path) {
    ifstream f(filePath);
    if (f.is_open()) {
        string line, section, key, value, str;
        map<string, string> keys;
        map<string, KeysMap> sections;
        bool flag = false;
        int lot;
        while (getline(f, str)) {
            line = str;
            lot = Checkeble(line);
            if (lot == 1) {
                if (flag) {
                    //Когда есть повторение
                    if (data.find(section) != data.end()) {

                        map<string, string> temp = data[section];

                        for (const auto &element: data[section]) {
                            //Присваиваем новые значения при data, при повторении, если
                            //старых нет в keys, то добавляем
                            if (keys.find(element.first) == keys.end()) {
                                keys[element.first] = element.second;
                            }
                        }

                    }
                    data[section] = keys;
                    keys.clear();
                }

                section = get_section_name(line);
                if (!flag) { flag = true; } //Для пропуска первой итерации

            }
            //внутри секции
            if (lot == 2) {
                int x = line.find('=');

                key = line.substr(0, x);
                value = line.substr(x + 1, line.length());

                keys[key] = value;

                key.erase(0, key.length());
                value.erase(0, value.length());

            }
        }
        //Найденно повторение
        if (data.find(section) != data.end()) {
            map<string, string> temp = data[section];
            for (const auto &element: data[section]) {
                //Присваиваем новые значения при data, при повторении, если
                //старых нет в keys, то добавляем
                if (keys.find(element.first) == keys.end()) {
                    keys[element.first] = element.second;
                }
            }

        }
        data[section] = keys;
        keys.clear();
    } else {
        throw "Error";
    }
}

//// Должен перезаписывать файл кешированой конифгурацией
IniFile::~IniFile() {
    ofstream out(filePath);
    for (const auto &section: data) {
        out << "[" << section.first << "]" << "\n";
        for (const auto &key: section.second) {
            out << key.first << "=" << key.second << "\n";
        }
    }
    data.clear();
}

//
///// Запись кешированной конфигурации в файл
void IniFile::save() {
    ofstream out(filePath);
    if (out.is_open()) {
        for (const auto &sections: data) {
            out << "[" << sections.first << "]" << "\n";
            for (const auto &keys: sections.second) {
                out << keys.first << "=" << keys.second << "\n";
            }
        }
    } else {
        throw "Error";
    }
}

//
///// Чтение значения типа int
/////  section Секция ini-файла
/////  key Ключ в секции
/////  def Значение по умолчанию, возвращаемое в случае отсутсвия ключа/секции
int IniFile::readInt(const string &section, const string &key, int def) {
    if ((isSectionExist(section) == true) && (isKeysExist(section, key) == true)) {
        return stoi(data[section][key]);
    }
    return def;
}

/////// Чтение значения типа double
double IniFile::readDouble(const string &section, const string &key, double def) {
    if ((isSectionExist(section) == true) && (isKeysExist(section, key) == true)) {
        return stod(data[section][key]);
    }
    return def;
}

///// Чтение значения типа std::string
string IniFile::readString(const std::string &section, const std::string &key, const std::string &def) {
    if ((isSectionExist(section) == true) && (isSectionExist(section) == true)) {
        return data[section][key];
    }
    return def;
}

///// Чтение значения типа bool
///// При чтении строка должна сопоставляться со всеми значениям из trueValues, независимо от регистра
///// @note В остальные случаях - вовзращается false
bool IniFile::readBool(const std::string &section, const std::string &key, bool def) {
    if (isSectionExist(section) && isKeysExist(section, key)) {
        string str = data[section][key];
        for (auto element : trueValues){
            if (str == element){
                return true;
            }
        }
    }
    return def;
    //const vector<string> trueValues = { "true", "yes", "y", "1" };
}


///// Запись значения типа int //+
void IniFile::writeInt(const std::string &section, const std::string &key, int value) {
    ofstream out(filePath);
    data[section][key] = to_string(value);
    out.close();
}
//Жесткий Хардкор:
//    if (!isSectionExist(section)) {
//        pair<string, string> m;
//        m.first = key;
//        m.second = to_string(value);
//        map<string, string> h;
//        h.insert(make_pair(m.first, m.second));
//        data[section] = h;
//        h.clear();
//    } else {
//        //Если существует и секция и ключ, то есть обновить данные
//        if (isKeysExist(section, key)) {
//            for (auto &element: data[section]) {
//                if (element.first == key) {
//                    element.second = to_string(value);
//                    break;
//                }
//            }
//        } else { //Существует секция, но нет значения
//            map<string, string> newKey = data[section];
//            pair<string, string> temp;
//            temp.first = key;
//            temp.second = to_string(value);
//            newKey.insert(make_pair(temp.first, temp.second));
//            data[section] = newKey;
//            newKey.clear();
//        }
//    }


//
///// Запись значения типа double //+
void IniFile::writeDouble(const std::string &section, const std::string &key, double value) {
    ofstream out(filePath);
    data[section][key] = to_string(value);
    out.close();
}

//
///// Запись значения типа std::string //+
void IniFile::writeString(const std::string &section, const std::string &key, const std::string &value) {
    ofstream out(filePath);
    data[section][key] = value;
    out.close();
}

string boolToString(bool value) {
    return value ? "true" : "false";
}
//
///// Запись значения типа bool
void IniFile::writeBool(const std::string &section, const std::string &key, bool value) {
    ofstream out(filePath);
    data[section][key] = boolToString(value);
    out.close();
}

///// Проверка существования секции
bool IniFile::isSectionExist(const std::string &section) {
//    if (data.count(section) != 0 ){
//        return true;
//    }return false;
    return data.find(section) != data.end();
}

/// Проверка существования ключа в секции
bool IniFile::isKeysExist(const std::string &section, const std::string &key) {
    if (data.find(section) != data.end()) {
        return data[section].find(key) != data[section].end();
    }
    return false;
}

/// Получение количества секций
size_t IniFile::getSectionsCount() {
    return data.size();
}

// Получение количества ключей в секции
size_t IniFile::getKeysCount(const std::string &section) {
    if (isSectionExist(section)) {
        return data[section].size();
    } else {
        return 0;
    }
}

///// Пол;учение значений всех секций
SectionsMap IniFile::getSections() const {
    return data;
}

///// Удаление секции
bool IniFile::deleteSection(const string &section) {
    //The function returns 1 if the key element is found in the map else returns 0
    if (data.find(section) == data.end()) {
        return false;
    }
    return data.erase(section) == 1;
}

///// Удаление ключа из секции
bool IniFile::deleteKey(const std::string &section, const std::string &key) {
    if (isSectionExist(section)) {
        return data[section].erase(key) == 1;
    } else {
        return false;
    }
}

///// Добавление новой секции
bool IniFile::addNewSection(const std::string &section) {
    if (isSectionExist(section) == false) {
        data[section] = {};
        return true;
    }
    return false;
}