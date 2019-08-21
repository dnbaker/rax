#include "raxpp.h"
#include <random>
#include <cstring>

int main() {
    rs::KVStore<int> ks;
    std::string ls("hello i am the world now");
    std::fprintf(stderr, "ls: '%s'\n", ls.data());
    std::mt19937_64 mt;
    for(size_t in = 0; in < 100; ++in) {
        auto v = mt();
        int c = 9;
        assert(ls[ls.size()] == '\0');
        for(size_t i = 0; i < ls.size(); ++i) {
            const char *s = "abcdefghijklmnopqrstuvwxyz";
            assert(std::strlen(s) == 26);
            ls[i] = s[v%26];
            assert(ls[i] <= 'z');
            v /= 26;
            if(--c == 0)
                v = mt(), c = 9;
            assert(ls[ls.size()] == '\0');
        }
        std::fprintf(stderr, "Emplacing: %s\n", ls.data());
        ks.emplace(ls, in);
    }
    ks.emplace(std::string("hello"), 14);
    ks.emplace(std::string("hello2"), 15);
    auto it = ks.begin();
    while(it != ks.end()) {
        std::fprintf(stderr, "Key: %s. value: %d\n", it->key, *(int *)it->data);
        ++it;
    }
    return 0;
}
