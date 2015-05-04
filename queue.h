#include <pthread.h>
#include <list>
using namespace std;

template <typename T> class queue
    {
    list<T> m_queue;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_condv;
    int max_queue_size;

    public:
        queue(int max_queue_size) { //constructor
            pthread_mutex_init(&m_mutex, NULL);
            pthread_cond_init(&m_condv, NULL);
            this->max_queue_size = max_queue_size;
            }

        ~queue() { //destructor
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_condv);
            }

        void setQueueSize(int n) {
            this->max_queue_size = n;
            }

        int add(T item) {
            int success = 1;
            pthread_mutex_lock(&m_mutex);
            if(m_queue.size() < max_queue_size) {
                m_queue.push_back(item); 
                pthread_cond_signal(&m_condv);
                }
            else {
                success = 0;
                }
            pthread_mutex_unlock(&m_mutex);
            return success;
            }

        T pop() {
            pthread_mutex_lock(&m_mutex);
            while(m_queue.size()==0){
                pthread_cond_wait(&m_condv, &m_mutex);
                }
            T item = m_queue.front();
            m_queue.pop_front();
            pthread_mutex_unlock(&m_mutex);
            return item;
            }

        int size() {
            pthread_mutex_lock(&m_mutex);
            int size = m_queue.size();
            pthread_mutex_unlock(&m_mutex);
            return size;
            }
    };
