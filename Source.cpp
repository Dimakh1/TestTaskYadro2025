#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
using namespace std;

// Ñòðóêòóðà äëÿ õðàíåíèÿ ñîáûòèÿ (âðåìÿ, òèï, èìÿ êëèåíòà, íîìåð ñòîëà)
struct Event {
    string time;
    int id;
    string client;
    int table = -1; // -1 åñëè íå óêàçàí
};

// Ñòðóêòóðà äëÿ õðàíåíèÿ ñòàòèñòèêè ïî ñòîëó
struct TableStats {
    string clientname;        // Èìÿ êëèåíòà çà ñòîëîì
    string sessionStart;      // Âðåìÿ íà÷àëà òåêóùåé ñåññèè
    int totalMinutes = 0;     // Ñóììàðíîå âðåìÿ èñïîëüçîâàíèÿ ñòîëà (â ìèíóòàõ)
    int totalRevenue = 0;     // Ñóììàðíàÿ âûðó÷êà ïî ñòîëó
};

// Ñòðóêòóðà äëÿ õðàíåíèÿ èíôîðìàöèè î ñòîëå
struct Table {
    int id;                   // Íîìåð ñòîëà
    string clientname;        // Èìÿ êëèåíòà çà ýòèì ñòîëîì (åñëè åñòü)
};

// Êëàññ, èíêàïñóëèðóþùèé âñþ ëîãèêó êîìïüþòåðíîãî êëóáà
class ComputerClub {
    int tableCount;                  // Êîëè÷åñòâî ñòîëîâ
    string openTime, closeTime;      // Âðåìÿ îòêðûòèÿ è çàêðûòèÿ êëóáà
    int pricePerHour;                // Ñòîèìîñòü ÷àñà
    vector<Event> events;            // Ñïèñîê âñåõ ñîáûòèé
    vector<Table> tables;            // Ñïèñîê ñòîëîâ
    vector<TableStats> stats;        // Ñòàòèñòèêà ïî êàæäîìó ñòîëó
    queue<string> waitingQueue;      // Î÷åðåäü îæèäàþùèõ êëèåíòîâ

public:
    ComputerClub() {}

    // Ïåðåâîä âðåìåíè "××:ÌÌ" â ìèíóòû îò ïîëóíî÷è
    static int timeToMinutes(const string& t) {
        return stoi(t.substr(0, 2)) * 60 + stoi(t.substr(3, 2));
    }
    // Ïåðåâîä ìèíóò â ñòðîêó "××:ÌÌ"
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
    // Ðàñ÷åò ñòîèìîñòè ñ îêðóãëåíèåì âðåìåíè äî ÷àñà ââåðõ
    static int calcCost(int minutes, int price) {
        int hours = (minutes + 59) / 60;
        return hours * price;
    }

    // Ïîäñ÷åò ðàçíèöû ìèíóò ìåæäó start è end ñ ó÷¸òîì ïåðåõîäà ÷åðåç ïîëíî÷ü è êðóãëîñóòî÷íîãî ðåæèìà
    static int diffMinutes(const string& start, const string& end) {
        int s = timeToMinutes(start);
        int e = timeToMinutes(end);
        if (e == s) // êðóãëîñóòî÷íî
            return 24 * 60;
        else if (e > s)
            return e - s;
        else
            return (24 * 60 - s) + e;
    }

    // Ïðîâåðêà ôîðìàòà âðåìåíè "××:ÌÌ"
    static bool isValidTime(const string& t) {
        return t.size() == 5 &&
            isdigit(t[0]) && isdigit(t[1]) &&
            t[2] == ':' &&
            isdigit(t[3]) && isdigit(t[4]);
    }
    // Ïðîâåðêà âàëèäíîñòè èìåíè êëèåíòà
    static bool isValidName(const string& name) {
        for (char c : name)
            if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') && c != '_' && c != '-')
                return false;
        return true;
    }

    // ×òåíèå è ïðîâåðêà êîíôèãóðàöèè êëóáà èç ôàéëà
    bool readConfig(ifstream& inputFile) {
        inputFile >> tableCount;
        if (inputFile.fail() || tableCount <= 0) { //Íåêîððåêòíîå êîëè÷åñòâî ñòîëîâ
            cout << tableCount << endl;
            return false;
        }
        inputFile >> openTime >> closeTime;
        if (!isValidTime(openTime) || !isValidTime(closeTime)) { //Íåêîððåêòíûé ôîðìàò âðåìåíè îòêðûòèÿ/çàêðûòèÿ
            cout << openTime << " " << closeTime << endl;
            
            return false;
        }
        // Ðàçðåøàåì êðóãëîñóòî÷íóþ ðàáîòó (09:00 09:00) è ïåðåõîä ÷åðåç ñóòêè
        inputFile >> pricePerHour;
        if (inputFile.fail() || pricePerHour <= 0) { //Íåêîððåêòíàÿ ñòîèìîñòü ÷àñà
            cout << pricePerHour << endl; 
            
            return false;
        }
        inputFile.ignore(); // ïåðåõîä íà ñëåäóþùóþ ñòðîêó
        tables.resize(tableCount);
        stats.resize(tableCount);
        for (int i = 0; i < tableCount; ++i) tables[i].id = i + 1;
        return true;
    }

    // ×òåíèå è âàëèäàöèÿ ñîáûòèé èç ôàéëà
    bool readEvents(ifstream& inputFile) {
        string line, prevTime = "";
        while (getline(inputFile, line)) {
            if (line.empty()) continue;

            Event e;
            e.table = -1;
            size_t pos = 0, prev = 0;
            int field = 0;
            string fields[4];
            // Ðàçäåëåíèå ñòðîêè íà ÷àñòè (âðåìÿ, id, èìÿ, [ñòîë])
            while (field < 4 && (pos = line.find(' ', prev)) != string::npos) {
                fields[field++] = line.substr(prev, pos - prev);
                prev = pos + 1;
            }
            if (field < 4 && prev < line.size())
                fields[field++] = line.substr(prev);

            if (field < 3) { //Íåäîñòàòî÷íî ïîëåé â ñîáûòèè
                cout <<  line << endl;
                return false;
            }

            string eventTime = fields[0];
            if (!isValidTime(eventTime)) { //íåêîððåêòíîå âðåìÿ
                cout << line << endl;
                return false;
            }

            if (!prevTime.empty() && timeToMinutes(eventTime) < timeToMinutes(prevTime)) { //âðåìÿ ñîáûòèÿ íå íåóáûâàþùåå
                cout << line << endl;
                
            }
            prevTime = eventTime;

            int eventId = 0;
            try {
                eventId = stoi(fields[1]);
            }
            catch (...) { //íåêîððåêòíûé id ñîáûòèÿ
                cout <<  line << endl;
                return false;
            }
            if (eventId < 1 || eventId > 4) {
                cout << line << endl;
                return false;
            }

            string clientName = fields[2];
            if (!isValidName(clientName)) { // íåêîððåêòíîå èìÿ êëèåíòà 
                cout <<  line  << endl;
                return false;
            }

            int tableNum = -1;
            if (field == 4) {
                try {
                    tableNum = stoi(fields[3]);
                }
                catch (...) { // íåêîððåêòíûé íîìåð ñòîëà
                    cout <<  line << endl;
                    return false;
                }
                if (tableNum < 1 || tableNum > tableCount) { // íîìåð ñòîëà âíå äèàïàçîíà â ñîáûòèè
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


    // Îñíîâíîé öèêë îáðàáîòêè ñîáûòèé
    void run() {
        cout << openTime << endl; // Âûâîä âðåìåíè îòêðûòèÿ
        for (size_t i = 0; i < events.size(); ++i) {
            const Event& ev = events[i];
            // Â çàâèñèìîñòè îò òèïà ñîáûòèÿ âûçûâàåì íóæíûé îáðàáîò÷èê
            switch (ev.id) {
            case 1: processEnter(ev, i); break;
            case 2: processSit(ev, i); break;
            case 3: processWait(ev, i); break;
            case 4: processLeave(ev, i); break;
            }
        }
        closeClub();  // Çàâåðøåíèå ðàáîòû êëóáà: âûãîíÿåì âñåõ îñòàâøèõñÿ êëèåíòîâ
        printStats(); // Âûâîä èòîãîâîé ñòàòèñòèêè
    }

private:
    // Îáðàáîòêà ñîáûòèÿ "âõîä êëèåíòà"
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
        // Ïðîâåðêà, íå çàøåë ëè êëèåíò ïîâòîðíî áåç âûõîäà
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

    // Îáðàáîòêà ñîáûòèÿ "êëèåíò ñàäèòñÿ çà ñòîë"
    void processSit(const Event& ev, int idx) {
        bool clientnotinclub = false;
        // Ïðîâåðêà, íàõîäèòñÿ ëè êëèåíò â êëóáå
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
                // Åñëè êëèåíò óæå ñèäèò çà äðóãèì ñòîëîì - çàêðûâàåì ïðåäûäóùóþ ñåññèþ
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
                // Ñàäèì êëèåíòà çà âûáðàííûé ñòîë
                tables[idxTable].clientname = ev.client;
                stats[idxTable].clientname = ev.client;
                stats[idxTable].sessionStart = ev.time;
                // Åñëè êëèåíò áûë â î÷åðåäè - óäàëÿåì èç î÷åðåäè
                if (!waitingQueue.empty() && ev.client == waitingQueue.front())
                    waitingQueue.pop();
                cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
            }
            else {
                // Ñòîë çàíÿò
                cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
                cout << ev.time << " 13 PlaceIsBusy" << endl;
            }
        }
    }

    // Îáðàáîòêà ñîáûòèÿ "êëèåíò æäåò â î÷åðåäè"
    void processWait(const Event& ev, int idx) {
        // Åñëè î÷åðåäü çàïîëíåíà - êëèåíò óõîäèò
        if (waitingQueue.size() >= (size_t)tableCount) {
            cout << ev.time << " " << ev.id << " " << ev.client << endl;
            cout << ev.time << " 11 " << ev.client << endl;
        }
        else {
            // Äîáàâëÿåì êëèåíòà â î÷åðåäü
            waitingQueue.push(ev.client);
            bool emptyplace = false;
            // Ïðîâåðÿåì, åñòü ëè ñâîáîäíûé ñòîë
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

    // Îáðàáîòêà ñîáûòèÿ "êëèåíò óõîäèò"
    void processLeave(const Event& ev, int idx) {
        bool clientnotinclub = false, clientwasonpc = false;
        // Ïðîâåðêà, íàõîäèòñÿ ëè êëèåíò â êëóáå
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
                    // Ñ÷èòàåì âðåìÿ è âûðó÷êó çà ñåññèþ
                    int duration = diffMinutes(stats[j].sessionStart, ev.time);
                    stats[j].totalMinutes += duration;
                    stats[j].totalRevenue += calcCost(duration, pricePerHour);
                    tables[j].clientname.clear();
                    stats[j].clientname.clear();
                    stats[j].sessionStart.clear();
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    // Åñëè åñòü îæèäàþùèé êëèåíò, ñàæàåì åãî çà ñòîë
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

    // Çàâåðøåíèå ðàáîòû êëóáà: âñåõ îñòàâøèõñÿ êëèåíòîâ "âûãîíÿåì", ñ÷èòàåì âûðó÷êó
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

    // Èòîãîâàÿ ïå÷àòü âûðó÷êè è âðåìåíè ïî êàæäîìó ñòîëó
    void printStats() {
        cout << closeTime << endl;
        for (int j = 0; j < (int)tables.size(); ++j)
            cout << (j + 1) << " " << stats[j].totalRevenue << " " << minutesToTime(stats[j].totalMinutes) << endl;
    }
};

int main() {
    setlocale(LC_ALL, "Ru");
    ifstream inputFile("test_file.txt");  // Òåñòîâûé ôàéë
    if (!inputFile.is_open()) {
        cout << "Íå óäàëîñü îòêðûòü ôàéë" << endl;
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
