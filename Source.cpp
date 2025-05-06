#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
using namespace std;

// Структура для хранения события (время, тип, имя клиента, номер стола)
struct Event {
    string time;
    int id;
    string client;
    int table = -1; // -1 если не указан
};

// Структура для хранения статистики по столу
struct TableStats {
    string clientname;        // Имя клиента за столом
    string sessionStart;      // Время начала текущей сессии
    int totalMinutes = 0;     // Суммарное время использования стола (в минутах)
    int totalRevenue = 0;     // Суммарная выручка по столу
};

// Структура для хранения информации о столе
struct Table {
    int id;                   // Номер стола
    string clientname;        // Имя клиента за этим столом (если есть)
};

// Класс, инкапсулирующий всю логику компьютерного клуба
class ComputerClub {
    int tableCount;                  // Количество столов
    string openTime, closeTime;      // Время открытия и закрытия клуба
    int pricePerHour;                // Стоимость часа
    vector<Event> events;            // Список всех событий
    vector<Table> tables;            // Список столов
    vector<TableStats> stats;        // Статистика по каждому столу
    queue<string> waitingQueue;      // Очередь ожидающих клиентов

public:
    ComputerClub() {}

    // Перевод времени "ЧЧ:ММ" в минуты от полуночи
    static int timeToMinutes(const string& t) {
        return stoi(t.substr(0, 2)) * 60 + stoi(t.substr(3, 2));
    }
    // Перевод минут в строку "ЧЧ:ММ"
    static string minutesToTime(int mins) {
        int h = mins / 60, m = mins % 60;
        string res;
        if (h < 10) res += '0';
        res += to_string(h);
        res += ':';
        if (m < 10) res += '0';
        res += to_string(m);
        return res;
    }
    // Расчет стоимости с округлением времени до часа вверх
    static int calcCost(int minutes, int price) {
        int hours = (minutes + 59) / 60;
        return hours * price;
    }

    // Подсчет разницы минут между start и end с учётом перехода через полночь и круглосуточного режима
    static int diffMinutes(const string& start, const string& end) {
        int s = timeToMinutes(start);
        int e = timeToMinutes(end);
        if (e == s) // круглосуточно
            return 24 * 60;
        else if (e > s)
            return e - s;
        else
            return (24 * 60 - s) + e;
    }

    // Проверка формата времени "ЧЧ:ММ"
    static bool isValidTime(const string& t) {
        return t.size() == 5 &&
            isdigit(t[0]) && isdigit(t[1]) &&
            t[2] == ':' &&
            isdigit(t[3]) && isdigit(t[4]);
    }
    // Проверка валидности имени клиента
    static bool isValidName(const string& name) {
        for (char c : name)
            if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') && c != '_' && c != '-')
                return false;
        return true;
    }

    // Чтение и проверка конфигурации клуба из файла
    bool readConfig(ifstream& inputFile) {
        inputFile >> tableCount;
        if (inputFile.fail() || tableCount <= 0) { //Некорректное количество столов
            cout << tableCount << endl;
            return false;
        }
        inputFile >> openTime >> closeTime;
        if (!isValidTime(openTime) || !isValidTime(closeTime)) { //Некорректный формат времени открытия/закрытия
            cout << openTime << " " << closeTime << endl;
            
            return false;
        }
        // Разрешаем круглосуточную работу (09:00 09:00) и переход через сутки
        inputFile >> pricePerHour;
        if (inputFile.fail() || pricePerHour <= 0) { //Некорректная стоимость часа
            cout << pricePerHour << endl; 
            
            return false;
        }
        inputFile.ignore(); // переход на следующую строку
        tables.resize(tableCount);
        stats.resize(tableCount);
        for (int i = 0; i < tableCount; ++i) tables[i].id = i + 1;
        return true;
    }

    // Чтение и валидация событий из файла
    bool readEvents(ifstream& inputFile) {
        string line, prevTime = "";
        while (getline(inputFile, line)) {
            if (line.empty()) continue;

            Event e;
            e.table = -1;
            size_t pos = 0, prev = 0;
            int field = 0;
            string fields[4];
            // Разделение строки на части (время, id, имя, [стол])
            while (field < 4 && (pos = line.find(' ', prev)) != string::npos) {
                fields[field++] = line.substr(prev, pos - prev);
                prev = pos + 1;
            }
            if (field < 4 && prev < line.size())
                fields[field++] = line.substr(prev);

            if (field < 3) { //Недостаточно полей в событии
                cout <<  line << endl;
                return false;
            }

            string eventTime = fields[0];
            if (!isValidTime(eventTime)) { //некорректное время
                cout << line << endl;
                return false;
            }

            if (!prevTime.empty() && timeToMinutes(eventTime) < timeToMinutes(prevTime)) { //время события не неубывающее
                cout << line << endl;
                
            }
            prevTime = eventTime;

            int eventId = 0;
            try {
                eventId = stoi(fields[1]);
            }
            catch (...) { //некорректный id события
                cout <<  line << endl;
                return false;
            }
            if (eventId < 1 || eventId > 4) {
                cout << line << endl;
                return false;
            }

            string clientName = fields[2];
            if (!isValidName(clientName)) { // некорректное имя клиента 
                cout <<  line  << endl;
                return false;
            }

            int tableNum = -1;
            if (field == 4) {
                try {
                    tableNum = stoi(fields[3]);
                }
                catch (...) { // некорректный номер стола
                    cout <<  line << endl;
                    return false;
                }
                if (tableNum < 1 || tableNum > tableCount) { // номер стола вне диапазона в событии
                    cout <<  line << endl;
                    return false;
                }
            }

            e.time = eventTime;
            e.id = eventId;
            e.client = clientName;
            e.table = tableNum;
            events.push_back(e);
        }
        return true;
    }


    // Основной цикл обработки событий
    void run() {
        cout << openTime << endl; // Вывод времени открытия
        for (size_t i = 0; i < events.size(); ++i) {
            const Event& ev = events[i];
            // В зависимости от типа события вызываем нужный обработчик
            switch (ev.id) {
            case 1: processEnter(ev, i); break;
            case 2: processSit(ev, i); break;
            case 3: processWait(ev, i); break;
            case 4: processLeave(ev, i); break;
            }
        }
        closeClub();  // Завершение работы клуба: выгоняем всех оставшихся клиентов
        printStats(); // Вывод итоговой статистики
    }

private:
    // Обработка события "вход клиента"
    void processEnter(const Event& ev, int idx) {
        if (openTime != closeTime && timeToMinutes(ev.time) < timeToMinutes(openTime)) {
            cout << ev.time << " " << ev.id << " " << ev.client << endl;
            cout << ev.time << " 13 NotOpenYet" << endl;
            return;
        }

        int lastNotOpenYetIdx = -1;
        for (int k = idx - 1; k >= 0; --k) {
            if (events[k].client == ev.client) {
                if (events[k].id == 1 && timeToMinutes(events[k].time) < timeToMinutes(openTime)) {
                    lastNotOpenYetIdx = k;
                    break;
                }
            }
        }
        bool alreadyIn = false;
        // Проверка, не зашел ли клиент повторно без выхода
        for (int j = idx - 1; j > lastNotOpenYetIdx; --j) {
            if (events[j].client == ev.client) {
                if (events[j].id == 1) {
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    cout << ev.time << " 13 YouShallNotPass" << endl;
                    alreadyIn = true;
                    break;
                }
                else if (events[j].id == 4) {
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    alreadyIn = true;
                    break;
                }
            }
        }
        if (!alreadyIn) cout << ev.time << " " << ev.id << " " << ev.client << endl;
    }

    // Обработка события "клиент садится за стол"
    void processSit(const Event& ev, int idx) {
        bool clientnotinclub = false;
        // Проверка, находится ли клиент в клубе
        for (int j = idx - 1; j >= 0; --j) {
            if (events[j].client == ev.client) {
                if (events[j].id == 1) break;
                else if (events[j].id == 4) {
                    cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
                    cout << ev.time << " 13 ClientUnknown" << endl;
                    clientnotinclub = true;
                    break;
                }
            }
        }
        if (!clientnotinclub) {
            int idxTable = ev.table - 1;
            if (tables[idxTable].clientname.empty()) {
                // Если клиент уже сидит за другим столом - закрываем предыдущую сессию
                for (int j = 0; j < (int)tables.size(); ++j) {
                    if (tables[j].clientname == ev.client) {
                        if (!stats[j].sessionStart.empty()) {
                            int duration = diffMinutes(stats[j].sessionStart, ev.time);
                            stats[j].totalMinutes += duration;
                            stats[j].totalRevenue += calcCost(duration, pricePerHour);
                        }
                        tables[j].clientname.clear();
                        stats[j].clientname.clear();
                        stats[j].sessionStart.clear();
                    }
                }
                // Садим клиента за выбранный стол
                tables[idxTable].clientname = ev.client;
                stats[idxTable].clientname = ev.client;
                stats[idxTable].sessionStart = ev.time;
                // Если клиент был в очереди - удаляем из очереди
                if (!waitingQueue.empty() && ev.client == waitingQueue.front())
                    waitingQueue.pop();
                cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
            }
            else {
                // Стол занят
                cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
                cout << ev.time << " 13 PlaceIsBusy" << endl;
            }
        }
    }

    // Обработка события "клиент ждет в очереди"
    void processWait(const Event& ev, int idx) {
        // Если очередь заполнена - клиент уходит
        if (waitingQueue.size() >= (size_t)tableCount) {
            cout << ev.time << " " << ev.id << " " << ev.client << endl;
            cout << ev.time << " 11 " << ev.client << endl;
        }
        else {
            // Добавляем клиента в очередь
            waitingQueue.push(ev.client);
            bool emptyplace = false;
            // Проверяем, есть ли свободный стол
            for (const auto& t : tables)
                if (t.clientname.empty()) {
                    emptyplace = true;
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    cout << ev.time << " 13 ICanWaitNoLonger!" << endl;
                    break;
                }
            if (!emptyplace)
                cout << ev.time << " " << ev.id << " " << ev.client << endl;
        }
    }

    // Обработка события "клиент уходит"
    void processLeave(const Event& ev, int idx) {
        bool clientnotinclub = false, clientwasonpc = false;
        // Проверка, находится ли клиент в клубе
        for (int j = idx - 1; j >= 0; --j) {
            if (events[j].client == ev.client) {
                if (events[j].id == 1) break;
                else if (events[j].id == 4) {
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    cout << ev.time << " 13 ClientUnknown" << endl;
                    clientnotinclub = true;
                    break;
                }
            }
        }
        if (!clientnotinclub) {
            for (int j = 0; j < (int)tables.size(); ++j) {
                if (tables[j].clientname == ev.client) {
                    clientwasonpc = true;
                    // Считаем время и выручку за сессию
                    int duration = diffMinutes(stats[j].sessionStart, ev.time);
                    stats[j].totalMinutes += duration;
                    stats[j].totalRevenue += calcCost(duration, pricePerHour);
                    tables[j].clientname.clear();
                    stats[j].clientname.clear();
                    stats[j].sessionStart.clear();
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    // Если есть ожидающий клиент, сажаем его за стол
                    if (!waitingQueue.empty()) {
                        tables[j].clientname = waitingQueue.front();
                        stats[j].clientname = waitingQueue.front();
                        stats[j].sessionStart = ev.time;
                        cout << ev.time << " 12 " << waitingQueue.front() << " " << tables[j].id << endl;
                        waitingQueue.pop();
                    }
                  
                }
            }
            if (!clientwasonpc)
                cout << ev.time << " " << ev.id << " " << ev.client << endl;
        }
    }

    // Завершение работы клуба: всех оставшихся клиентов "выгоняем", считаем выручку
    void closeClub() {
        for (int j = 0; j < (int)tables.size(); ++j) {
            if (!stats[j].clientname.empty()) {
                int duration = diffMinutes(stats[j].sessionStart, closeTime);
                stats[j].totalMinutes += duration;
                stats[j].totalRevenue += calcCost(duration, pricePerHour);
                cout << closeTime << " 11 " << stats[j].clientname << endl;
            }
        }
    }

    // Итоговая печать выручки и времени по каждому столу
    void printStats() {
        cout << closeTime << endl;
        for (int j = 0; j < (int)tables.size(); ++j)
            cout << (j + 1) << " " << stats[j].totalRevenue << " " << minutesToTime(stats[j].totalMinutes) << endl;
    }
};

int main() {
    setlocale(LC_ALL, "Ru");
    ifstream inputFile("test_file.txt");  // Тестовый файл
    if (!inputFile.is_open()) {
        cout << "Не удалось открыть файл" << endl;
        return 1;
    }
    ComputerClub club;
    if (!club.readConfig(inputFile)) {
        return 1;
    };
    if (!club.readEvents(inputFile)) {
        return 1;
    }
    club.readEvents(inputFile);
    club.run();
    return 0;
}
