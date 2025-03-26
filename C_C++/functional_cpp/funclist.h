#ifndef FUNCLIST_H_
#define FUNCLIST_H_

#include <sstream>
#include <functional>

namespace flist
{
    auto empty = [](auto, auto a) {
        return a;
    }; 

    namespace detail {
        auto _of_range(auto &r, auto it, auto f, auto a) {
            if (it == r.end()) {
                return a;
            } else {
                return f(*it, _of_range(r, std::next(it), f, a));
            }
        };
    }

    auto rev = [](auto l) {
        return [=](auto f, auto a) {
            using A = decltype(a);
            using A2A = std::function<A(A)>;
            return l(
                [=](auto x, A2A g) {
                    return static_cast<A2A>([=](A a_placeholder) {
                        return g(f(x, a_placeholder));
                    });
                },
                static_cast<A2A> ([=](A a_placeholder) -> A {
                    return a_placeholder;
                })
            )(a); 
        };
    };

    auto cons = [](auto x, auto l) {  
        return [=](auto f, auto a) {
            return f(x, l(f, a));
        };
    };

    auto create = [](auto... args) {
        return rev([=](auto f, auto a) {
            (..., (a = f(args, a)));
            return a;
        });
    };

    auto of_range = [](auto r) {
        using Unwrapped = typename std::unwrap_reference<decltype(r)>::type;
        Unwrapped unwrapped_r = r;

        return [=](auto f, auto a) {
            auto it = unwrapped_r.begin();
            return (it == unwrapped_r.end()) ? a : f(*it, detail::_of_range(unwrapped_r, std::next(it), f, a));
        };
    };

    auto concat = [](auto l, auto k) {
        return [=](auto f, auto a) {
            return l(f, k(f, a));
        };
    };

    auto map = [](auto m, auto l) {
        return [=](auto f, auto a) {
            return l(
                [=](auto x, auto a_placeholder) {
                    return f(m(x), a_placeholder);
                }
                , a);
        };
    };

    auto filter = [](auto p, auto l) {
        return [=](auto f, auto a) {
            return l(
                [=](auto x, auto a_placeholder) {
                    return (p(x) == true) ? f(x, a_placeholder) : a_placeholder;
                },
                a
            );
        };
    };

    auto flatten = [](auto l) {
        return [=](auto f, auto a) {
            return l( 
                [=](auto x, auto a_placeholder) {
                    return x(f, a_placeholder);
                }
                ,
                a 
            );            
        };
    };

    auto as_string = [](const auto& l) {
        auto r = rev(l);
        std::ostringstream os;
        os << "[";
        bool first = true;
        auto f = [&](auto& x, auto a) {
            if (first) {
                first = false;
                os << x;
            } else {
                os << ";" << x;
            }
            return a;
        };
        r(f, NULL);
        os << "]";
        return os.str();
    };
    
} // namespace flist

#endif  // FUNCLIST_H_
