#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "store.h"
#include <string>

class DataManager {
public:
    static bool saveStore(const Store& store, const std::string& folder);
    static bool loadStore(Store& store, const std::string& folder);
};

#endif // DATA_MANAGER_H
