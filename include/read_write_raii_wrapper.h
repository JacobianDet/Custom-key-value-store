#pragma once

namespace Test {

template <typename Lock>
class RAII_RWLock {
	private:
		Lock &lk;
		bool write_lock;

	public:
		RAII_RWLock(Lock &lk_, bool write_lock_) : lk(lk_), write_lock(write_lock_) {
			if (write_lock)
				lk.write_lock();
			else
				lk.read_lock();
		}

		~RAII_RWLock() {
			if (write_lock)
				lk.write_unlock();
			else
				lk.read_unlock();
		}	
};

}
