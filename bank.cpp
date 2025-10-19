#include "bank.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

bool Bank::createAccount(const std::string& accountID, const std::string& ownerName, double initial) {
    if (accounts.find(accountID) != accounts.end()) return false;
    BankAccount a(accountID, ownerName, initial);
    if (initial > 0) a.txs.push_back({todayDate(), initial, "initial"});
    accounts[accountID] = a;
    return true;
}

BankAccount* Bank::getAccount(const std::string& accountID) {
    auto it = accounts.find(accountID);
    if (it == accounts.end()) return nullptr;
    return &it->second;
}

bool Bank::deposit(const std::string& accountID, double amount, const std::string& date, const std::string& note) {
    BankAccount* a = getAccount(accountID);
    if (!a) return false;
    a->deposit(amount, date, note);
    return true;
}

bool Bank::withdraw(const std::string& accountID, double amount, const std::string& date, const std::string& note) {
    BankAccount* a = getAccount(accountID);
    if (!a) return false;
    return a->withdraw(amount, date, note);
}

std::vector<std::string> Bank::listCustomers() const {
    std::vector<std::string> out;
    for (auto &p : accounts) out.push_back(p.second.ownerName + " (" + p.first + ")");
    return out;
}

std::vector<std::pair<std::string, double>> Bank::transactionsLastWeek() const {
    std::vector<std::pair<std::string, double>> out;
    std::string t = todayDate();
    for (auto &p : accounts) {
        for (auto &tx : p.second.txs) {
            if (daysBetween(t, tx.date) <= 7) {
                out.emplace_back(tx.date + " | " + p.second.ownerName, tx.amount);
            }
        }
    }
    std::sort(out.begin(), out.end(), [](auto &a, auto &b){ return a.first > b.first; });
    return out;
}

std::vector<std::string> Bank::dormantAccounts(int daysWithoutTx) const {
    std::vector<std::string> out;
    std::string t = todayDate();
    for (auto &p : accounts) {
        if (p.second.txs.empty()) {
            out.push_back(p.second.ownerName + " (" + p.first + ")");
        } else {
            std::string last = p.second.lastTransactionDate();
            if (daysBetween(t, last) > daysWithoutTx) out.push_back(p.second.ownerName + " (" + p.first + ")");
        }
    }
    return out;
}

std::vector<std::pair<std::string, int>> Bank::topNActiveToday(int n) const {
    std::map<std::string,int> counts;
    std::string t = todayDate();
    for (auto &p : accounts) {
        int c = 0;
        for (auto &tx : p.second.txs) if (tx.date == t) ++c;
        if (c>0) counts[p.second.ownerName + " (" + p.first + ")"] = c;
    }
    std::vector<std::pair<std::string,int>> vec(counts.begin(), counts.end());
    std::sort(vec.begin(), vec.end(), [](auto &a, auto &b){ return a.second > b.second; });
    if ((int)vec.size() > n) vec.resize(n);
    return vec;
}

int Bank::daysBetween(const std::string& d1, const std::string& d2) {
    // parse YYYY-MM-DD
    std::tm tm1 = {}, tm2 = {};
    std::istringstream ss1(d1), ss2(d2);
    ss1 >> std::get_time(&tm1, "%Y-%m-%d");
    ss2 >> std::get_time(&tm2, "%Y-%m-%d");
    std::time_t time1 = std::mktime(&tm1);
    std::time_t time2 = std::mktime(&tm2);
    if (time1== (std::time_t)-1 || time2 == (std::time_t)-1) return 0;
    double diff = std::difftime(time1, time2);
    return static_cast<int>(diff / (60*60*24));
}

std::string Bank::todayDate() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d");
    return ss.str();
}
