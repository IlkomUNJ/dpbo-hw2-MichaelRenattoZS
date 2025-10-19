#include "store.h"
#include <random>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <iomanip>

void Store::setBank(Bank* b) { bank = b; }

bool Store::registerBuyer(const std::string& id, const std::string& uname, const std::string& pass) {
    if (buyers.find(id) != buyers.end()) return false;
    Buyer b(id, uname, pass);
    buyers[id] = b;
    // create bank account for user too
    if (bank) bank->createAccount(id, uname, 0.0);
    return true;
}

bool Store::registerSeller(const std::string& id, const std::string& uname, const std::string& pass) {
    if (sellers.find(id) != sellers.end()) return false;
    Seller s(id, uname, pass);
    sellers[id] = s;
    if (bank) bank->createAccount(id, uname, 0.0);
    return true;
}

User* Store::login(const std::string& username, const std::string& password) {
    for (auto& [id, b] : buyers) {
        if (b.username == username && b.password == password)
            return &b;
    }
    for (auto& [id, s] : sellers) {
        if (s.username == username && s.password == password)
            return &s;
    }
    return nullptr;
}


bool Store::addItem(const std::string& sellerID, const std::string& itemID,
                    const std::string& name, double price, int stock) {
    auto sit = sellers.find(sellerID);
    if (sit == sellers.end()) return false;
    if (items.find(itemID) != items.end()) return false;
    Item it(itemID, name, price, stock);
    items[itemID] = it;
    sit->second.itemIDs.push_back(itemID);
    return true;
}

bool Store::replenishItem(const std::string& sellerID, const std::string& itemID, int qty) {
    auto sit = sellers.find(sellerID);
    if (sit == sellers.end()) return false;
    auto it = items.find(itemID);
    if (it == items.end()) return false;
    it->second.replenish(qty);
    return true;
}

bool Store::discardItem(const std::string& sellerID, const std::string& itemID, int qty) {
    auto sit = sellers.find(sellerID);
    if (sit == sellers.end()) return false;
    auto it = items.find(itemID);
    if (it == items.end()) return false;
    it->second.discard(qty);
    return true;
}

bool Store::setItemPrice(const std::string& sellerID, const std::string& itemID, double price) {
    auto sit = sellers.find(sellerID);
    if (sit == sellers.end()) return false;
    auto it = items.find(itemID);
    if (it == items.end()) return false;
    it->second.price = price;
    return true;
}

bool Store::purchase(const std::string& buyerID, const std::string& itemID, int qty, const std::string& date) {
    auto bit = buyers.find(buyerID);
    auto it = items.find(itemID);
    if (bit == buyers.end() || it == items.end()) return false;
    if (!it->second.canSell(qty)) return false;
    double total = it->second.price * qty;

    // check buyer bank balance
    if (!bank) return false;
    BankAccount* ba = bank->getAccount(buyerID);
    BankAccount* sa = bank->getAccount(it->second.itemID); // no, seller account is sellerID, not itemID
    // find seller id: item -> which seller? We don't store reverse mapping efficiently; scan sellers
    std::string sellerOfItem;
    for (auto &p : sellers) {
        auto &vec = p.second.itemIDs;
        if (std::find(vec.begin(), vec.end(), itemID) != vec.end()) {
            sellerOfItem = p.first;
            break;
        }
    }
    if (sellerOfItem.empty()) return false;
    sa = bank->getAccount(sellerOfItem);
    if (!ba || !sa) return false;
    if (ba->balance < total) return false;

    // withdraw from buyer, deposit to seller
    if (!bank->withdraw(buyerID, total, date, std::string("purchase ") + itemID)) return false;
    bank->deposit(sellerOfItem, total, date, std::string("sale ") + itemID);

    // update item sold
    it->second.sell(qty);

    // create transaction
    std::string txid = genID("TX");
    Transaction tx(txid, date, buyerID, sellerOfItem, itemID, it->second.name, qty, total, TransactionStatus::PAID);
    transactions[txid] = tx;

    // record in buyer/seller
    bit->second.orderIDs.push_back(txid);
    sellers[sellerOfItem].saleTxIDs.push_back(txid);

    return true;
}

std::vector<Transaction> Store::listTransactionsLastKDays(int k) const {
    std::vector<Transaction> out;
    std::string t = Bank::todayDate();
    for (auto &p : transactions) {
        if (Bank::daysBetween(t, p.second.date) <= k) out.push_back(p.second);
    }
    return out;
}

std::vector<Transaction> Store::listPaidNotCompleted() const {
    std::vector<Transaction> out;
    for (auto &p : transactions) {
        if (p.second.status == TransactionStatus::PAID) out.push_back(p.second);
    }
    return out;
}

std::vector<std::pair<std::string,int>> Store::mostFrequentItems(int m) const {
    std::vector<std::pair<std::string,int>> vec;
    for (auto &p : items) vec.emplace_back(p.second.name, p.second.soldCount);
    std::sort(vec.begin(), vec.end(), [](auto &a, auto &b){ return a.second > b.second; });
    if ((int)vec.size() > m) vec.resize(m);
    return vec;
}

std::vector<std::pair<std::string,int>> Store::mostActiveBuyersPerDay(int topN) const {
    std::map<std::string,int> counts; // buyer -> count today
    std::string t = Bank::todayDate();
    for (auto &p : transactions) {
        if (p.second.date == t) counts[p.second.buyerID]++;
    }
    std::vector<std::pair<std::string,int>> vec(counts.begin(), counts.end());
    std::sort(vec.begin(), vec.end(), [](auto &a, auto &b){ return a.second > b.second; });
    if ((int)vec.size() > topN) vec.resize(topN);
    return vec;
}

std::vector<std::pair<std::string,int>> Store::mostActiveSellersPerDay(int topN) const {
    std::map<std::string,int> counts; // seller -> count today
    std::string t = Bank::todayDate();
    for (auto &p : transactions) {
        if (p.second.date == t) counts[p.second.sellerID]++;
    }
    std::vector<std::pair<std::string,int>> vec(counts.begin(), counts.end());
    std::sort(vec.begin(), vec.end(), [](auto &a, auto &b){ return a.second > b.second; });
    if ((int)vec.size() > topN) vec.resize(topN);
    return vec;
}

std::string Store::genID(const std::string& prefix) {
    static std::mt19937_64 rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(1000, 9999);
    std::ostringstream ss;
    ss << prefix << dist(rng);
    return ss.str();
}
