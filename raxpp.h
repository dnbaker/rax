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

    size_t size() const {return core_.numele;}

    static void init_stack(raxStack &rs) {
        rs.stack = rs.static_items;
        rs.items = 0;
        rs.maxitems = RAX_STACK_STATIC_ITEMS;
        rs.oom = 0;
    }

    struct iterator: raxIterator {
        bool operator==(int o) {return this->flags & RAX_ITER_EOF;}
        bool operator!=(int o) {return !(this->flags & RAX_ITER_EOF);}
        iterator(rax &rt) {
            this->flags = RAX_ITER_EOF;
            this->rt = &rt;
            this->key_len = 0;
            this->key = this->key_static_string;
            this->key_max = RAX_ITER_STATIC_LEN;
            this->data = nullptr;
            this->node_cb = nullptr;
            init_stack(this->stack);
        }
        iterator &operator++() {
            int ret = raxNext(*this);
            if(!ret && errno == ENOMEM) throw std::bad_alloc();
        }
    };
    iterator begin() {return iterator(core_);}
    int end() const {return 0;}
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
    int raxNext(raxIterator &it) {
        if (!raxIteratorNextStep(&it,0)) {
            errno = ENOMEM;
            return 0;
        }
        if (it.flags & RAX_ITER_EOF) {
            errno = 0;
            return 0;
        }
        return 1;
    }

};

} // redis

namespace rs = redis;

#endif /* RAX_HPP_H__ */
