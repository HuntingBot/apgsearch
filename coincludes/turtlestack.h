#include <vector>

namespace cgold {

    template<typename T>
    struct TurtleStack {
        /*
        * There are default elements all the way down...
        */

        public:
        std::vector<T> stack;
        T default_element;

        TurtleStack(T de) {
            default_element = de;
        }

        T operator[](size_t i) {
            if (i < stack.size()) {
                return stack[stack.size() - (1 + i)];
            } else {
                return default_element;
            }
        }

        T pop() {
            if (stack.empty()) {
                return default_element;
            } else {
                T x = stack.back();
                stack.pop_back();
                return x;
            }
        }

        void push(const T &x) { stack.push_back(x); }

    };

}
