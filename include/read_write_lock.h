#pragma once
#include <mutex>
#include <condition_variable>

namespace Test {

class RWLock {

  protected:
    int readers = 0;
    std::mutex rd_mutex, rd_wt_mtx, rd_wt_tst_mtx;
    std::condition_variable rd_mutex_cv, rd_wt_mtx_cv, rd_wt_tst_mtx_cv;

    void read_proc_lock() {
        std::unique_lock<std::mutex> rd_mutex_lk(rd_mutex);
        rd_mutex_cv.wait(rd_mutex_lk, []{ return true; });
        readers++;
        if (readers == 1) {
            std::unique_lock<std::mutex> rd_wt_mtx_lk(rd_wt_mtx);
            rd_wt_mtx_cv.wait(rd_wt_mtx_lk, []{ return true; });
        }
        rd_mutex_cv.notify_one();
    }

    void read_proc_unlock() {
        std::unique_lock<std::mutex> rd_mutex_lk(rd_mutex);
        rd_mutex_cv.wait(rd_mutex_lk, []{ return true; });
        readers--;
        if (readers == 0)
            rd_wt_mtx_cv.notify_one();
        rd_mutex_cv.notify_one();
    }

  public:
    void write_lock() {
        std::unique_lock<std::mutex> rd_wt_tst_mtx_lk(rd_wt_tst_mtx), rd_wt_mtx_lk(rd_wt_mtx);
        rd_wt_tst_mtx_cv.wait(rd_wt_tst_mtx_lk, []{ return true; });
        rd_wt_mtx_cv.wait(rd_wt_mtx_lk, []{ return true; });
    }

    void write_unlock() {
        rd_wt_tst_mtx_cv.notify_one();
        rd_wt_mtx_cv.notify_one();
    }

    void read_lock() {
        std::unique_lock<std::mutex> rd_wt_tst_mtx_lk(rd_wt_tst_mtx);
        rd_wt_tst_mtx_cv.wait(rd_wt_tst_mtx_lk, []{ return true; });
        rd_wt_tst_mtx_cv.notify_one();

        read_proc_lock();
    }

    void read_unlock() {
        read_proc_unlock();
    }
};

}
