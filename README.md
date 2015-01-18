A parser combinator library written in C++14.

Efficiency is not the goal of this library. After all, parsing is almost never a perfomrance bottleneck of a program analysis system. 

This project emphasize code clarity and ease of usage (at least for me) over everything else.

Besides code for parsing, the library also contains a StringRef implementation (which basically is a copy form llvm::StringRef) to avoid excessive std::string copies.

Library codes can be found on include/ and lib/ subdirectories. Some examples of using the combinator can be found on examples/ subdirectory.
