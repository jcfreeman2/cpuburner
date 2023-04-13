# CPUBurner a dummy DAQModule for testing CPU loading

 This module starts a worker thread that burns CPU by looping over and
summing a vector of floats for a configurable time before sleeping for
another configurable time. The size of the float vector can also be
configured to allow it to fit or not into CPU cache.
