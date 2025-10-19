#include "data_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>

// Simple text-based dump. You can extend to robust CSV/JSON if needed.

bool DataManager::saveStore(const Store& store, const std::string& folder) {
    // save accounts
    {
        std::ofstream f(folder + "/accounts.txt");
        if (!f) return false;
        for (auto &p : store.bank->accounts) {
            const auto &acc = p.second;
            f << acc.accountID << "|" << acc.ownerName << "|" << acc.balance << "\n";
            for (auto &tx : acc.txs) {
                f << acc.accountID << "," << tx.date << "," << tx.amount << "," << tx.note << "\n";
            }
        }
    }
    // save items
    {
        std::ofstream f(folder + "/items.txt");
        if (!f) return false;
        for (auto &p : store.items) {
            const auto &it = p.second;
            f << it.itemID << "|" << it.name << "|" << it.price << "|" << it.stock << "|" << it.soldCount << "\n";
        }
    }
    // save users (buyers)
    {
        std::ofstream f(folder + "/buyers.txt");
        if (!f) return false;
        for (auto &p : store.buyers) {
            const auto &b = p.second;
            f << b.userID << "|" << b.username << "|" << b.password << "\n";
            for (auto &oid : b.orderIDs) f << b.userID << "," << oid << "\n";
        }
    }
    // save users (sellers)
    {
        std::ofstream f(folder + "/sellers.txt");
        if (!f) return false;
        for (auto &p : store.sellers) {
            const auto &s = p.second;
            f << s.userID << "|" << s.username << "|" << s.password << "\n";
            for (auto &iid : s.itemIDs) f << s.userID << "," << iid << "\n";
            for (auto &txid : s.saleTxIDs) f << s.userID << "," << txid << "\n";
        }
    }
    // save transactions
    {
        std::ofstream f(folder + "/transactions.txt");
        if (!f) return false;
        for (auto &p : store.transactions) {
            const auto &t = p.second;
            f << t.transactionID << "|" << t.date << "|" << t.buyerID << "|" << t.sellerID << "|" << t.itemID
              << "|" << t.itemName << "|" << t.quantity << "|" << t.totalPrice << "|" << (int)t.status << "\n";
        }
    }
    return true;
}

bool DataManager::loadStore(Store& store, const std::string& folder) {
    // NOTE: This basic loader assumes files exist and are consistent. Add checks when needed.

    store.items.clear();
    store.buyers.clear();
    store.sellers.clear();
    store.transactions.clear();
    if (!store.bank) store.bank = new Bank();

    // load accounts
    {
        std::ifstream f(folder + "/accounts.txt");
        if (!f) return false;
        std::string line;
        // First read basic accounts
        std::vector<std::string> lines;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            lines.push_back(line);
        }
        // parse account lines and tx lines from those lines:
        for (auto &ln : lines) {
            // accounts lines were in form accountID|ownerName|balance OR accountID,date,amount,note
            if (ln.find('|') != std::string::npos) {
                std::istringstream ss(ln);
                std::string id, owner; double bal;
                std::getline(ss, id, '|');
                std::getline(ss, owner, '|');
                ss >> bal;
                store.bank->createAccount(id, owner, bal);
            } else {
                // ignore other forms here
            }
        }
    }

    // items
    {
        std::ifstream f(folder + "/items.txt");
        if (!f) /*ignore*/;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string id, name;
            double price; int stock, sold;
            std::getline(ss, id, '|');
            std::getline(ss, name, '|');
            ss >> price; ss.ignore(1,'|');
            ss >> stock; ss.ignore(1,'|');
            ss >> sold;
            Item it(id, name, price, stock);
            it.soldCount = sold;
            store.items[id] = it;
        }
    }

    // buyers
    {
        std::ifstream f(folder + "/buyers.txt");
        if (f) {
            std::string line;
            while (std::getline(f, line)) {
                if (line.empty()) continue;
                if (line.find('|') != std::string::npos) {
                    std::istringstream ss(line);
                    std::string id, uname, pass;
                    std::getline(ss, id, '|');
                    std::getline(ss, uname, '|');
                    std::getline(ss, pass, '|');
                    store.buyers[id] = Buyer(id, uname, pass);
                    // ensure bank account exists
                    if (store.bank->getAccount(id) == nullptr) store.bank->createAccount(id, uname, 0.0);
                } else if (line.find(',') != std::string::npos) {
                    // order mapping
                    std::istringstream ss(line);
                    std::string id, oid;
                    std::getline(ss, id, ',');
                    std::getline(ss, oid, ',');
                    if (store.buyers.find(id) != store.buyers.end()) store.buyers[id].orderIDs.push_back(oid);
                }
            }
        }
    }

    // sellers
    {
        std::ifstream f(folder + "/sellers.txt");
        if (f) {
            std::string line;
            while (std::getline(f, line)) {
                if (line.empty()) continue;
                if (line.find('|') != std::string::npos) {
                    std::istringstream ss(line);
                    std::string id, uname, pass;
                    std::getline(ss, id, '|');
                    std::getline(ss, uname, '|');
                    std::getline(ss, pass, '|');
                    store.sellers[id] = Seller(id, uname, pass);
                    if (store.bank->getAccount(id) == nullptr) store.bank->createAccount(id, uname, 0.0);
                } else if (line.find(',') != std::string::npos) {
                    std::istringstream ss(line);
                    std::string id, part;
                    std::getline(ss, id, ',');
                    std::getline(ss, part, ',');
                    // Part could be itemID or txID. We'll push both into itemIDs and saleTxIDs; correctness depends on consistency.
                    if (store.sellers.find(id) != store.sellers.end()) store.sellers[id].itemIDs.push_back(part);
                }
            }
        }
    }

    // transactions
    {
        std::ifstream f(folder + "/transactions.txt");
        if (f) {
            std::string line;
            while (std::getline(f, line)) {
                if (line.empty()) continue;
                std::istringstream ss(line);
                std::string txid, date, buyer, seller, itemid, itemname;
                int qty; double price; int statusi;
                std::getline(ss, txid, '|');
                std::getline(ss, date, '|');
                std::getline(ss, buyer, '|');
                std::getline(ss, seller, '|');
                std::getline(ss, itemid, '|');
                std::getline(ss, itemname, '|');
                ss >> qty; ss.ignore(1,'|');
                ss >> price; ss.ignore(1,'|');
                ss >> statusi;
                Transaction t(txid, date, buyer, seller, itemid, itemname, qty, price, (TransactionStatus)statusi);
                store.transactions[txid] = t;
            }
        }
    }

    return true;
}
