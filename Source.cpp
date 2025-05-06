#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
using namespace std;

// ��������� ��� �������� ������� (�����, ���, ��� �������, ����� �����)
struct Event {
    string time;
    int id;
    string client;
    int table = -1; // -1 ���� �� ������
};

// ��������� ��� �������� ���������� �� �����
struct TableStats {
    string clientname;        // ��� ������� �� ������
    string sessionStart;      // ����� ������ ������� ������
    int totalMinutes = 0;     // ��������� ����� ������������� ����� (� �������)
    int totalRevenue = 0;     // ��������� ������� �� �����
};

// ��������� ��� �������� ���������� � �����
struct Table {
    int id;                   // ����� �����
    string clientname;        // ��� ������� �� ���� ������ (���� ����)
};

// �����, ��������������� ��� ������ ������������� �����
class ComputerClub {
    int tableCount;                  // ���������� ������
    string openTime, closeTime;      // ����� �������� � �������� �����
    int pricePerHour;                // ��������� ����
    vector<Event> events;            // ������ ���� �������
    vector<Table> tables;            // ������ ������
    vector<TableStats> stats;        // ���������� �� ������� �����
    queue<string> waitingQueue;      // ������� ��������� ��������

public:
    ComputerClub() {}

    // ������� ������� "��:��" � ������ �� ��������
    static int timeToMinutes(const string& t) {
        return stoi(t.substr(0, 2)) * 60 + stoi(t.substr(3, 2));
    }
    // ������� ����� � ������ "��:��"
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
    // ������ ��������� � ����������� ������� �� ���� �����
    static int calcCost(int minutes, int price) {
        int hours = (minutes + 59) / 60;
        return hours * price;
    }

    // ������� ������� ����� ����� start � end � ������ �������� ����� ������� � ��������������� ������
    static int diffMinutes(const string& start, const string& end) {
        int s = timeToMinutes(start);
        int e = timeToMinutes(end);
        if (e == s) // �������������
            return 24 * 60;
        else if (e > s)
            return e - s;
        else
            return (24 * 60 - s) + e;
    }

    // �������� ������� ������� "��:��"
    static bool isValidTime(const string& t) {
        return t.size() == 5 &&
            isdigit(t[0]) && isdigit(t[1]) &&
            t[2] == ':' &&
            isdigit(t[3]) && isdigit(t[4]);
    }
    // �������� ���������� ����� �������
    static bool isValidName(const string& name) {
        for (char c : name)
            if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') && c != '_' && c != '-')
                return false;
        return true;
    }

    // ������ � �������� ������������ ����� �� �����
    bool readConfig(ifstream& inputFile) {
        inputFile >> tableCount;
        if (inputFile.fail() || tableCount <= 0) { //������������ ���������� ������
            cout << tableCount << endl;
            return false;
        }
        inputFile >> openTime >> closeTime;
        if (!isValidTime(openTime) || !isValidTime(closeTime)) { //������������ ������ ������� ��������/��������
            cout << openTime << " " << closeTime << endl;
            
            return false;
        }
        // ��������� �������������� ������ (09:00 09:00) � ������� ����� �����
        inputFile >> pricePerHour;
        if (inputFile.fail() || pricePerHour <= 0) { //������������ ��������� ����
            cout << pricePerHour << endl; 
            
            return false;
        }
        inputFile.ignore(); // ������� �� ��������� ������
        tables.resize(tableCount);
        stats.resize(tableCount);
        for (int i = 0; i < tableCount; ++i) tables[i].id = i + 1;
        return true;
    }

    // ������ � ��������� ������� �� �����
    bool readEvents(ifstream& inputFile) {
        string line, prevTime = "";
        while (getline(inputFile, line)) {
            if (line.empty()) continue;

            Event e;
            e.table = -1;
            size_t pos = 0, prev = 0;
            int field = 0;
            string fields[4];
            // ���������� ������ �� ����� (�����, id, ���, [����])
            while (field < 4 && (pos = line.find(' ', prev)) != string::npos) {
                fields[field++] = line.substr(prev, pos - prev);
                prev = pos + 1;
            }
            if (field < 4 && prev < line.size())
                fields[field++] = line.substr(prev);

            if (field < 3) { //������������ ����� � �������
                cout <<  line << endl;
                return false;
            }

            string eventTime = fields[0];
            if (!isValidTime(eventTime)) { //������������ �����
                cout << line << endl;
                return false;
            }

            if (!prevTime.empty() && timeToMinutes(eventTime) < timeToMinutes(prevTime)) { //����� ������� �� �����������
                cout << line << endl;
                
            }
            prevTime = eventTime;

            int eventId = 0;
            try {
                eventId = stoi(fields[1]);
            }
            catch (...) { //������������ id �������
                cout <<  line << endl;
                return false;
            }
            if (eventId < 1 || eventId > 4) {
                cout << line << endl;
                return false;
            }

            string clientName = fields[2];
            if (!isValidName(clientName)) { // ������������ ��� ������� 
                cout <<  line  << endl;
                return false;
            }

            int tableNum = -1;
            if (field == 4) {
                try {
                    tableNum = stoi(fields[3]);
                }
                catch (...) { // ������������ ����� �����
                    cout <<  line << endl;
                    return false;
                }
                if (tableNum < 1 || tableNum > tableCount) { // ����� ����� ��� ��������� � �������
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


    // �������� ���� ��������� �������
    void run() {
        cout << openTime << endl; // ����� ������� ��������
        for (size_t i = 0; i < events.size(); ++i) {
            const Event& ev = events[i];
            // � ����������� �� ���� ������� �������� ������ ����������
            switch (ev.id) {
            case 1: processEnter(ev, i); break;
            case 2: processSit(ev, i); break;
            case 3: processWait(ev, i); break;
            case 4: processLeave(ev, i); break;
            }
        }
        closeClub();  // ���������� ������ �����: �������� ���� ���������� ��������
        printStats(); // ����� �������� ����������
    }

private:
    // ��������� ������� "���� �������"
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
        // ��������, �� ����� �� ������ �������� ��� ������
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

    // ��������� ������� "������ ������� �� ����"
    void processSit(const Event& ev, int idx) {
        bool clientnotinclub = false;
        // ��������, ��������� �� ������ � �����
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
                // ���� ������ ��� ����� �� ������ ������ - ��������� ���������� ������
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
                // ����� ������� �� ��������� ����
                tables[idxTable].clientname = ev.client;
                stats[idxTable].clientname = ev.client;
                stats[idxTable].sessionStart = ev.time;
                // ���� ������ ��� � ������� - ������� �� �������
                if (!waitingQueue.empty() && ev.client == waitingQueue.front())
                    waitingQueue.pop();
                cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
            }
            else {
                // ���� �����
                cout << ev.time << " " << ev.id << " " << ev.client << " " << ev.table << endl;
                cout << ev.time << " 13 PlaceIsBusy" << endl;
            }
        }
    }

    // ��������� ������� "������ ���� � �������"
    void processWait(const Event& ev, int idx) {
        // ���� ������� ��������� - ������ ������
        if (waitingQueue.size() >= (size_t)tableCount) {
            cout << ev.time << " " << ev.id << " " << ev.client << endl;
            cout << ev.time << " 11 " << ev.client << endl;
        }
        else {
            // ��������� ������� � �������
            waitingQueue.push(ev.client);
            bool emptyplace = false;
            // ���������, ���� �� ��������� ����
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

    // ��������� ������� "������ ������"
    void processLeave(const Event& ev, int idx) {
        bool clientnotinclub = false, clientwasonpc = false;
        // ��������, ��������� �� ������ � �����
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
                    // ������� ����� � ������� �� ������
                    int duration = diffMinutes(stats[j].sessionStart, ev.time);
                    stats[j].totalMinutes += duration;
                    stats[j].totalRevenue += calcCost(duration, pricePerHour);
                    tables[j].clientname.clear();
                    stats[j].clientname.clear();
                    stats[j].sessionStart.clear();
                    cout << ev.time << " " << ev.id << " " << ev.client << endl;
                    // ���� ���� ��������� ������, ������ ��� �� ����
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

    // ���������� ������ �����: ���� ���������� �������� "��������", ������� �������
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

    // �������� ������ ������� � ������� �� ������� �����
    void printStats() {
        cout << closeTime << endl;
        for (int j = 0; j < (int)tables.size(); ++j)
            cout << (j + 1) << " " << stats[j].totalRevenue << " " << minutesToTime(stats[j].totalMinutes) << endl;
    }
};

int main() {
    setlocale(LC_ALL, "Ru");
    ifstream inputFile("test_file.txt");  // �������� ����
    if (!inputFile.is_open()) {
        cout << "�� ������� ������� ����" << endl;
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