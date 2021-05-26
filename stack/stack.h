#ifndef _3_STACK_H
#define _3_STACK_H

#include <atomic>
#include <thread>
#include <algorithm>
#include <vector>
#include <sstream>

static const int BATCH_SIZE = 10;

template <typename T>
class stack {
    private:
        struct node {
            T data;
            node* next;
            explicit node(const T& data);
            ~node();
        };

        struct garbage_ptr {
            node* garb_node;
            garbage_ptr* next;

            ~garbage_ptr()
            {
                delete garb_node;
            }
        };

        typedef std::atomic<node*>& hazard_pointer;
        hazard_pointer get_hazard_pointer_for_current_thread();

        void add_to_garbage_list(garbage_ptr* garbage_node);

        std::atomic <node*> top;
        std::atomic <garbage_ptr*> garbage;

        std::atomic <std::size_t> current_thread_number;
        std::atomic <std::size_t> iteration;

        std::atomic <node*>* hazard_pointers;
        std::size_t n_threads_max;
        std::size_t batch_size;

        void add_to_garbage(node* deleted);
        void scan();
    public:
        stack() = delete;
        explicit stack(std::size_t n_threads_max);
        ~stack();

        void push(T const& value);
        bool pop(T& value);
};

template <typename T>
stack<T>::node::node(const T& data): data(data), next(nullptr) {};

template <typename T>
stack<T>::node::~node() {
    delete next;
};

template <typename T>
void stack<T>::add_to_garbage_list(garbage_ptr* garbage_node)
{
    garbage_node->next=garbage.load(std::memory_order_relaxed);
    int yield_cnt = 0;
    while(!garbage.compare_exchange_weak(garbage_node->next,garbage_node,
                                         std::memory_order_release,
                                         std::memory_order_relaxed
                                         )) {
        yield_cnt++;
        if (yield_cnt == 5) {
            yield_cnt = 0;
            std::this_thread::yield();
        }
    }
}

template <typename T>
typename stack<T>::hazard_pointer stack<T>::get_hazard_pointer_for_current_thread()
{
    thread_local static std::size_t hazard = current_thread_number.fetch_add(1, std::memory_order_relaxed);

    if (hazard >= n_threads_max) {
        throw std::runtime_error("Too many threads!!!");
    }
    return hazard_pointers[hazard];
}

template <typename T>
stack<T>::stack(std::size_t n_threads_max)
        : top(nullptr), garbage(nullptr),
          current_thread_number(0), iteration(0),
          n_threads_max(n_threads_max), batch_size(2 * n_threads_max),
          hazard_pointers(new std::atomic<node*>[n_threads_max]) {}


template <typename T>
stack<T>::~stack() {
    delete top.load();
    delete garbage.load();
    delete hazard_pointers;
}

template <typename T>
void stack<T>::add_to_garbage(node *deleted) {
    auto* ret_node = new garbage_ptr {
            deleted,
            nullptr
    };
    add_to_garbage_list(ret_node);

    std::size_t x = iteration.fetch_add(1, std::memory_order_relaxed);
    if (batch_size < x) {
        iteration.store(0, std::memory_order_relaxed);
        scan();
    }
}

template <typename T>
void stack<T>::scan() {
    garbage_ptr* scanned_node = garbage.exchange(nullptr, std::memory_order_acquire);
    std::vector <node*> hp_to_sort;
    hp_to_sort.reserve(n_threads_max);
    for (int i = 0; i < n_threads_max; ++i) {
        auto ptr = hazard_pointers[i].load(std::memory_order::memory_order_acquire);
        if (ptr) {
            hp_to_sort.push_back(ptr);
        } else {
            break;
        }
    }
    std::sort(hp_to_sort.begin(), hp_to_sort.end());

    while (scanned_node) {
        auto next = scanned_node->next;
        if (!std::binary_search(hp_to_sort.begin(), hp_to_sort.end(), scanned_node->garb_node)) {
            delete scanned_node;
        } else {
            add_to_garbage_list(scanned_node);
        }
        scanned_node = next;
    }
}

template <typename T>
void stack<T>::push(const T& value) {
    auto* new_node = new node (value);
    new_node->next = top.load(std::memory_order_relaxed);
    int yield_cnt = 0;
    while (!top.compare_exchange_weak(new_node->next, new_node,
                                      std::memory_order_release,
                                      std::memory_order_relaxed)) {
        yield_cnt++;
        if (yield_cnt == 4) {
            yield_cnt = 0;
            std::this_thread::yield();
        }
    }
}

template <typename T>
bool stack<T>::pop(T& value) {
    hazard_pointer hp = get_hazard_pointer_for_current_thread();
    node* old_head=top.load();
    int yield_cnt = 0;
    do
    {
        node* temp;
        do
        {
            temp=old_head;
            hp.store(old_head, std::memory_order_acq_rel);
            old_head=top.load(std::memory_order_relaxed);
            yield_cnt++;
            if (yield_cnt == 5) {
                yield_cnt = 0;
                std::this_thread::yield();
            }
        } while(old_head!=temp);
    }
    while(old_head &&
          !top.compare_exchange_weak(old_head,old_head->next, std::memory_order_acquire, std::memory_order_relaxed));
    if (old_head) {
        value=old_head->data;
        old_head->next = nullptr;
        add_to_garbage(old_head);
        return true;
    }
    return false;
}

#endif //_3_STACK_H
