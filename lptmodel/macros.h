#ifndef MACROS_H_
#define MACROS_H_

// Macro for creating getters and setters
#define GETSET(TYPE, NAME) \
TYPE & NAME() { return NAME ## _; } \
const TYPE & NAME() const { return NAME ## _; }



#endif /* MACROS_H_ */
