#ifndef RINGBUF_HPP
#define RINGBUF_HPP
#include <cstdlib>
#include <mutex>
template <class T>
class Ringbuf_t {
public:
    explicit Ringbuf_t(unsigned size) : \
        size_(size), in_(0), out_(0) {
        buf_ =  reinterpret_cast<T*>(aligned_alloc(8, size * sizeof(T)));
    }

    ~Ringbuf_t() {
       if (buf_) {
           free(buf_);
       }
    }

    // if error happens in reading and writing data, use reset to reset ring buffer.
    void reset() {
         std::lock_guard<std::mutex> lock(mtx_);
         in_ = 0;
         out_ = 0;
    }

    inline T* put(const T* obj_buf, unsigned int num_of_T, unsigned int* num_put = NULL) {
        // must lock for the multithread context
        std::lock_guard<std::mutex> lock(mtx_);
        unsigned int l = 0;
        num_of_T = std::min(num_of_T, size_ - in_ + out_);

        /* first put the data starting from in to buffer end */
        l = std::min(num_of_T, size_ - (in_ & (size_ - 1)));
        T* base_addr = buf_ + (in_ & (size_ - 1));
        if (obj_buf) {
            memcpy(base_addr, obj_buf, l * sizeof(T));

            /* then put the rest (if any) at the beginning of the buffer */
            memcpy(buf_, obj_buf + l, (num_of_T - l) * sizeof(T));
        }
        in_ += num_of_T;
        // used this option to check whether all the data has been put into ring buffer.
        if (num_put) {
            *num_put = num_of_T;
        }

        // usually used to check the data is valid or not when reading data from ringbuffer.
        return base_addr;
    }

    inline T* get(T* obj_buf, unsigned int num_of_T, unsigned int* num_get = NULL) {
        // must lock for the multithread context
        std::lock_guard<std::mutex> lock(mtx_);
        unsigned int l = 0;
        num_of_T = std::min(num_of_T, in_ - out_);

        /* first get the data from out until the end of the buffer */
        l = std::min(num_of_T, size_ - (out_ & (size_ - 1)));
        T* base_addr = buf_ + (out_ & (size_ - 1));
        if (obj_buf) {
            memcpy(obj_buf, base_addr, l * sizeof(T));
            /* then get the rest (if any) from the beginning of the buffer */
            memcpy(obj_buf + l, buf_, (num_of_T - l) * sizeof(T));
        }

        out_ += num_of_T;
        if (num_get) {
            *num_get = num_of_T;
        }

        return base_addr;
    }
    // get the number of objects contained in ring buffer
    inline unsigned int get_obj_number() {
        std::lock_guard<std::mutex> lock(mtx_);
        return (in_ - out_);
    }

    // get the number of objects that current ringbuffer can hold.
    inline unsigned int get_free_obj_number() {
        std::lock_guard<std::mutex> lock(mtx_);
        return size_ - (in_ - out_);
    }

    // get the current read address.
    inline const T* get_read_addr() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (in_ - out_ > 0) {
            return buf_ + (out_ & (size_ - 1));
        } else {
            return NULL;
        }
    }

    // find a given address is contained in ringbuffer or not
    // return offset from head.
    inline unsigned int find(T* addr) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (in_ - out_ == 0) {
            return 0;
        }
        // first search addr from read address to the end of the buffer.
        unsigned int l = size_ - (out_ & (size_ - 1));
        for (unsigned int i = 0; i < l; ++i) {
            if (addr == buf_ + (out_ & (size_ - 1)) + i) {
                return i;
            }
        }

        unsigned int s = (in_ - out_) - l;
        for (unsigned int i = 0; i < s; ++i) {
            if (addr == buf_ + i) {
                return i + l;
            }
        }
        // find none.
        return 0;
    }

    inline bool is_memory_overlap(T* addr, size_t len) {
        return (size_ - (addr - buf_) / sizeof(T)) < len;
    }
private:
    unsigned int size_;  // can hold size_ number of T
    unsigned int in_;  // (in % size)
    unsigned int out_;  // (out % size)
    T* buf_;
    std::mutex mtx_;
};

#endif  // RINGBUF_HPP
