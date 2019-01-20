//
// Created by leon on 1/13/19.
//

#ifndef WHOCARES_MACRO_UTILS_H
#define WHOCARES_MACRO_UTILS_H
// THIS IS TODO ignore, change it to (void) 0 and we
// can find todos through warning
#define TODO_IGNORE(x) (void) x
#if __cplusplus >= 201703L
#define UNUSED [[maybe_unused]]
#define  IGNORE(x) (void) 0
#elif defined(__GNUC__)
#define UNUSED __attribute__((unused))
#define  IGNORE(x) (void) 0
#else
#define UNUSED
#define  IGNORE(x) (void) x
#endif
#endif //WHOCARES_MACRO_UTILS_H
