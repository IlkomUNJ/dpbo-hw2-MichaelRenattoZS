#ifndef BANK_H
#define BANK_H

#include "models.h"
#include <map>
#include <string>
#include <vector>

class Bank {
public:
    std::map<std::string, BankAccount> accounts; // accountID -> account

    Bank() = default;

    bool createAccount(const std::string& accountID, const std::string& ownerName, double initial = 0.0);
    BankAccount* getAccount(const std::string& accountID);
    bool deposit(const std::string& accountID, double amount, const std::string& date, const std::string& note = "");
    bool withdraw(const std::string& accountID, double amount, const std::string& date, const std::string& note = "");

    std::vector<std::string> listCustomers() const;
    std::vector<std::pair<std::string, double>> transactionsLastWeek() const;
    std::vector<std::string> dormantAccounts(int daysWithoutTx = 30) const;
    std::vector<std::pair<std::string, int>> topNActiveToday(int n) const;

    // helper
    static int daysBetween(const std::string& d1, const std::string& d2); // d1-d2
    static std::string todayDate();
};

#endif // BANK_H
