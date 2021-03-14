#include "SpinLock.h"

// TODO


SpinLock::lock(){
	while(atomic_value.test_and_set());
}

SpinLock::unlock(){
	atomic_value.clear();
}
