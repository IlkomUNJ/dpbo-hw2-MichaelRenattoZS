#include <iostream>
#include <string>
#include "models.h"
#include "bank.h"
#include "store.h"
#include "data_manager.h"

// ------------------------------
// Forward declarations
// ------------------------------
void buyerMenu(Store& store, Buyer* buyer);
void sellerMenu(Store& store, Seller* seller);

// ------------------------------
// Demo data setup
// ------------------------------
void seedDemo(Store &store) {
    store.registerSeller("S1", "alice_seller", "123");
    store.registerBuyer("B1", "bob_buyer", "123");
    store.addItem("S1", "I1", "Sword", 50.0, 10);
    store.addItem("S1", "I2", "Shield", 30.0, 5);

    std::string t = Bank::todayDate();
    store.bank->deposit("B1", 200.0, t, "topup demo");
    store.bank->deposit("S1", 0.0, t, "seller start");
}

// ------------------------------
// Main function
// ------------------------------
int main() {
    Store store;
    Bank bank;
    store.setBank(&bank);
    store.bank = &bank;

    std::string folder = "data_store";  // make sure this folder exists manually

    std::cout << "=== Online Store Simulation ===\n";
    std::cout << "Loading saved data (if any)...\n";
    DataManager::loadStore(store, folder);
    store.setBank(&bank);
    store.bank = &bank;

    bool running = true;
    while (running) {
        std::cout << "\n1) Register Buyer\n";
        std::cout << "2) Register Seller\n";
        std::cout << "3) Login\n";
        std::cout << "4) Seed demo data\n";
        std::cout << "5) Save & Exit\n";
        std::cout << "Choice: ";
        int choice; std::cin >> choice; std::cin.ignore();

        if (choice == 1) {
            std::string id, uname, pass;
            std::cout << "Buyer ID: "; std::cin >> id;
            std::cout << "Username: "; std::cin >> uname;
            std::cout << "Password: "; std::cin >> pass;
            if (store.registerBuyer(id, uname, pass))
                std::cout << "Buyer registered.\n";
            else std::cout << "Failed (maybe already exists).\n";
        }
        else if (choice == 2) {
            std::string id, uname, pass;
            std::cout << "Seller ID: "; std::cin >> id;
            std::cout << "Username: "; std::cin >> uname;
            std::cout << "Password: "; std::cin >> pass;
            if (store.registerSeller(id, uname, pass))
                std::cout << "Seller registered.\n";
            else std::cout << "Failed (maybe already exists).\n";
        }
        else if (choice == 3) {
            std::string uname, pass;
            std::cout << "Username: "; std::cin >> uname;
            std::cout << "Password: "; std::cin >> pass;
            User* user = store.login(uname, pass);
            if (!user) {
                std::cout << "Login failed.\n";
                continue;
            }
            std::cout << "Welcome, " << user->username << "!\n";
            if (user->role == "buyer")
                buyerMenu(store, static_cast<Buyer*>(user));
            else if (user->role == "seller")
                sellerMenu(store, static_cast<Seller*>(user));
        }
        else if (choice == 4) {
            seedDemo(store);
            std::cout << "Demo data created.\n";
        }
        else if (choice == 5) {
            DataManager::saveStore(store, folder);
            std::cout << "Saved data and exit.\n";
            running = false;
        }
        else {
            std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}

// ------------------------------
// Buyer Menu
// ------------------------------
void buyerMenu(Store& store, Buyer* buyer) {
    bool active = true;
    while (active) {
        std::cout << "\n=== Buyer Menu === (" << buyer->username << ")\n";
        std::cout << "1) View items\n";
        std::cout << "2) Purchase item\n";
        std::cout << "3) View orders\n";
        std::cout << "4) View spending (last k days)\n";
        std::cout << "5) Top-up\n";
        std::cout << "6) Withdraw\n";
        std::cout << "0) Logout\n";
        std::cout << "Choice: ";
        int c; std::cin >> c;

        if (c == 0) break;
        else if (c == 1) {
            for (auto& [id, item] : store.items)
                std::cout << id << " | " << item.name
                          << " | Price: " << item.price
                          << " | Stock: " << item.stock << "\n";
        }
        else if (c == 2) {
            std::string iid; int qty;
            std::cout << "Item ID: "; std::cin >> iid;
            std::cout << "Quantity: "; std::cin >> qty;
            std::string d = Bank::todayDate();
            if (store.purchase(buyer->userID, iid, qty, d))
                std::cout << "Purchase successful.\n";
            else std::cout << "Failed (balance/stock/id issue).\n";
        }
        else if (c == 3) {
            for (auto& tid : buyer->orderIDs) {
                auto it = store.transactions.find(tid);
                if (it != store.transactions.end()) {
                    const auto& t = it->second;
                    std::cout << t.transactionID << " | " << t.itemName
                              << " | " << t.totalPrice << " | "
                              << (t.status==TransactionStatus::PAID?"PAID":
                                  t.status==TransactionStatus::COMPLETED?"COMPLETE":"CANCELLED") << "\n";
                }
            }
        }
        else if (c == 4) {
            int k; std::cout << "Days: "; std::cin >> k;
            double total = 0;
            std::string today = Bank::todayDate();
            for (auto& tid : buyer->orderIDs) {
                auto it = store.transactions.find(tid);
                if (it != store.transactions.end()) {
                    const auto& t = it->second;
                    if (Bank::daysBetween(today, t.date) <= k)
                        total += t.totalPrice;
                }
            }
            std::cout << "Total spending in last " << k << " days: " << total << "\n";
        }
        else if (c == 5) {
            double amt; std::cout << "Top-up amount: "; std::cin >> amt;
            store.bank->deposit(buyer->userID, amt, Bank::todayDate(), "topup");
            std::cout << "Balance: " << store.bank->getAccount(buyer->userID)->balance << "\n";
        }
        else if (c == 6) {
            double amt; std::cout << "Withdraw amount: "; std::cin >> amt;
            if (store.bank->withdraw(buyer->userID, amt, Bank::todayDate(), "withdraw"))
                std::cout << "Done. Balance: " << store.bank->getAccount(buyer->userID)->balance << "\n";
            else std::cout << "Not enough balance.\n";
        }
    }
}

// ------------------------------
// Seller Menu
// ------------------------------
void sellerMenu(Store& store, Seller* seller) {
    bool active = true;
    while (active) {
        std::cout << "\n=== Seller Menu === (" << seller->username << ")\n";
        std::cout << "1) Add item\n";
        std::cout << "2) Replenish item\n";
        std::cout << "3) Discard item\n";
        std::cout << "4) Set price\n";
        std::cout << "5) View my items\n";
        std::cout << "0) Logout\n";
        std::cout << "Choice: ";
        int c; std::cin >> c;

        if (c == 0) break;
        else if (c == 1) {
            std::string iid, name; double price; int stock;
            std::cout << "Item ID: "; std::cin >> iid;
            std::cin.ignore();
            std::cout << "Item name: "; std::getline(std::cin, name);
            std::cout << "Price: "; std::cin >> price;
            std::cout << "Stock: "; std::cin >> stock;
            if (store.addItem(seller->userID, iid, name, price, stock))
                std::cout << "Item added.\n";
            else std::cout << "Failed.\n";
        }
        else if (c == 2) {
            std::string iid; int qty;
            std::cout << "Item ID: "; std::cin >> iid;
            std::cout << "Qty: "; std::cin >> qty;
            if (store.replenishItem(seller->userID, iid, qty))
                std::cout << "Replenished.\n";
            else std::cout << "Failed.\n";
        }
        else if (c == 3) {
            std::string iid; int qty;
            std::cout << "Item ID: "; std::cin >> iid;
            std::cout << "Discard qty: "; std::cin >> qty;
            if (store.discardItem(seller->userID, iid, qty))
                std::cout << "Discarded.\n";
            else std::cout << "Failed.\n";
        }
        else if (c == 4) {
            std::string iid; double p;
            std::cout << "Item ID: "; std::cin >> iid;
            std::cout << "New price: "; std::cin >> p;
            if (store.setItemPrice(seller->userID, iid, p))
                std::cout << "Updated.\n";
            else std::cout << "Failed.\n";
        }
        else if (c == 5) {
            for (auto& iid : seller->itemIDs) {
                auto it = store.items.find(iid);
                if (it != store.items.end()) {
                    auto& i = it->second;
                    std::cout << i.itemID << " | " << i.name
                              << " | Price: " << i.price
                              << " | Stock: " << i.stock << "\n";
                }
            }
        }
    }
}
