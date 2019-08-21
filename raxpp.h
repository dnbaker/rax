#ifndef RAX_HPP_H__
#define RAX_HPP_H__

#include "rax.h"
#include <cstdlib>
#include <stdexcept>

namespace redis {

struct DoNothing {template<typename...Args> void operator()(Args &&...args) const {} };

template<typename V=int>
class KVStore {
    rax core_;
public:
    KVStore(): core_{raxNewNode(0,0), 0, 1} {
        if(core_.head == nullptr) throw std::bad_alloc();
    }
    template<typename F>
    void clear_with_callback_recursive(raxNode *n, const F &f=F()) {
        int numchildren = n->iscompr ? 1: n->size;
        raxNode **cp = raxNodeLastChildPtr(n);
        while(numchildren--) {
            raxNode *child;
            memcpy(&child,cp,sizeof(child));
            clear_with_callback_recursive(child, f);
            cp--;
        }
        if (n->iskey && !n->isnull)
            f(get_data(n));
        std::free(n);
        --core_.numnodes;
    }
    void clear() {clear_with_callback_recursive<DoNothing>(core_.head);}


    ~KVStore() {
        clear();
    }

//Helper functions:
private:
    static constexpr raxNode **raxNodeLastChildPtr(raxNode *n) {
        return reinterpret_cast<raxNode **>((char *)(n)
               + raxNodeCurrentLength(n) 
               - sizeof(raxNode *)
               - (((n)->iskey && !(n)->isnull) ? sizeof(void *): 0)
        );
    }
    static constexpr size_t padding(size_t nodesize) {
        return ((sizeof(void*)-((nodesize+4) % sizeof(void*))) & (sizeof(void*)-1));
    }

    static constexpr size_t raxNodeCurrentLength(raxNode *n) {
        return sizeof(raxNode)+(n)->size +
            + padding((n)->size)
            + ((n)->iscompr ? sizeof(raxNode*) : sizeof(raxNode*)*(n)->size)
            + (((n)->iskey && !(n)->isnull)*sizeof(void*));
    }
    static void *get_data(raxNode *n) {
        if (n->isnull) return nullptr;
        void **ndata = reinterpret_cast<void**>((char*)n+raxNodeCurrentLength(n)-sizeof(void*));
        void *data;
        memcpy(&data,ndata,sizeof(data));
        return data;
    }
};

}

namespace rs = redis;

#endif /* RAX_HPP_H__ */
