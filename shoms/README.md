SHOMS is a microbenchmark for evaluating the performance of an OpenSHMEM implementation.
The design goal was to have a simple benchmark capable of testing the performance of all of the functions of the latest OpenSHMEM specification.
It is designed to iterate over each function with multiple message lengths.
It will produce the minimum, maximum, and average overhead for issuing a message with each function.
It will also calculate the bandwidth for functions where this is appropriate.

SHOMS is also designed to be highly modular and configurable.
Extending SHOMS is simplistic and allows the addition of new functions with minimal effort.
This reduces the effort for testing new experimental features.
