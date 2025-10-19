#ifndef STORE_H
#define STORE_H

#include "models.h"
#include "bank.h"
#include <map>
#include <vector>
#include <string>

class Store {
public:
    std::map<std::string, Buyer> buyers;   // userID -> Buyer
    std::map<std::string, Seller> sellers; // userID -> Seller
    std::map<std::string, Item> items;     // itemID -> Item
    std::map<std::string, Transaction> transactions; // txID -> Transaction
    Bank* bank; // reference to bank for payments

    Store() : bank(nullptr) {}
    void setBank(Bank* b);

    bool registerBuyer(const std::string& id, const std::string& uname, const std::string& pass);
    bool registerSeller(const std::string& id, const std::string& uname, const std::string& pass);
    User* login(const std::string& username, const std::string& password);

    bool addItem(const std::string& sellerID, const std::string& itemID,
                 const std::string& name, double price, int stock);
    bool replenishItem(const std::string& sellerID, const std::string& itemID, int qty);
    bool discardItem(const std::string& sellerID, const std::string& itemID, int qty);
    bool setItemPrice(const std::string& sellerID, const std::string& itemID, double price);

    bool purchase(const std::string& buyerID, const std::string& itemID, int qty, const std::string& date);

    std::vector<Transaction> listTransactionsLastKDays(int k) const;
    std::vector<Transaction> listPaidNotCompleted() const;
    std::vector<std::pair<std::string,int>> mostFrequentItems(int m) const;

    std::vector<std::pair<std::string,int>> mostActiveBuyersPerDay(int topN) const;
    std::vector<std::pair<std::string,int>> mostActiveSellersPerDay(int topN) const;

    // helpers
    static std::string genID(const std::string& prefix);
};

#endif // STORE_H
