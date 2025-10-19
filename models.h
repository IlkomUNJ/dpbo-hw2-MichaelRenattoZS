#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <ctime>

enum class TransactionStatus { PAID, COMPLETED, CANCELED };

struct BankTx {
    std::string date; // YYYY-MM-DD
    double amount;    // + deposit, - withdraw
    std::string note;
};

class BankAccount {
public:
    std::string accountID;
    std::string ownerName;
    double balance;
    std::vector<BankTx> txs;

    BankAccount() = default;
    BankAccount(const std::string& id, const std::string& owner, double initial = 0.0);

    void deposit(double amount, const std::string& date, const std::string& note = "");
    bool withdraw(double amount, const std::string& date, const std::string& note = "");
    std::string lastTransactionDate() const; // returns "" if none
};

struct Transaction {
    std::string transactionID;
    std::string date; // YYYY-MM-DD
    std::string buyerID;
    std::string sellerID;
    std::string itemID;
    std::string itemName;
    int quantity;
    double totalPrice;
    TransactionStatus status;

    Transaction() = default;
    Transaction(const std::string& tid, const std::string& d,
                const std::string& bid, const std::string& sid,
                const std::string& iid, const std::string& iname,
                int qty, double price, TransactionStatus s);
};

class Item {
public:
    std::string itemID;
    std::string name;
    double price;
    int stock;
    int soldCount;

    Item() = default;
    Item(const std::string& id, const std::string& n, double p, int s);

    bool canSell(int qty) const;
    bool sell(int qty);
    void replenish(int qty);
    void discard(int qty);
};

class User {
public:
    std::string userID;
    std::string username;
    std::string password;
    std::string role; // "buyer" or "seller"
    BankAccount account;

    User() = default;
    User(const std::string& id, const std::string& uname, const std::string& pass, const std::string& r);
};

class Buyer : public User {
public:
    std::vector<std::string> orderIDs; // store transaction IDs
    Buyer() = default;
    Buyer(const std::string& id, const std::string& uname, const std::string& pass);
};

class Seller : public User {
public:
    std::vector<std::string> itemIDs;   // IDs of items owned
    std::vector<std::string> saleTxIDs; // transaction IDs
    Seller() = default;
    Seller(const std::string& id, const std::string& uname, const std::string& pass);
};

#endif // MODELS_H
