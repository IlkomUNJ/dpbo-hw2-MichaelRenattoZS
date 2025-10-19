#include "models.h"
#include <algorithm>

BankAccount::BankAccount(const std::string& id, const std::string& owner, double initial)
    : accountID(id), ownerName(owner), balance(initial) {}

void BankAccount::deposit(double amount, const std::string& date, const std::string& note) {
    balance += amount;
    txs.push_back({date, amount, note});
}

bool BankAccount::withdraw(double amount, const std::string& date, const std::string& note) {
    if (amount > balance) return false;
    balance -= amount;
    txs.push_back({date, -amount, note});
    return true;
}

std::string BankAccount::lastTransactionDate() const {
    if (txs.empty()) return "";
    return txs.back().date;
}

Transaction::Transaction(const std::string& tid, const std::string& d,
                         const std::string& bid, const std::string& sid,
                         const std::string& iid, const std::string& iname,
                         int qty, double price, TransactionStatus s)
    : transactionID(tid), date(d), buyerID(bid), sellerID(sid),
      itemID(iid), itemName(iname), quantity(qty), totalPrice(price), status(s) {}

Item::Item(const std::string& id, const std::string& n, double p, int s)
    : itemID(id), name(n), price(p), stock(s), soldCount(0) {}

bool Item::canSell(int qty) const {
    return stock >= qty;
}

bool Item::sell(int qty) {
    if (!canSell(qty)) return false;
    stock -= qty;
    soldCount += qty;
    return true;
}

void Item::replenish(int qty) {
    stock += qty;
}

void Item::discard(int qty) {
    stock = std::max(0, stock - qty);
}

User::User(const std::string& id, const std::string& uname, const std::string& pass, const std::string& r)
    : userID(id), username(uname), password(pass), role(r), account(id, uname, 0.0) {}

Buyer::Buyer(const std::string& id, const std::string& uname, const std::string& pass)
    : User(id, uname, pass, "buyer") {}

Seller::Seller(const std::string& id, const std::string& uname, const std::string& pass)
    : User(id, uname, pass, "seller") {}
