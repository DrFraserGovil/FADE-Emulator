# Changelog

<!-- All notable changes to this project will be documented in this file. -->

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).



## [0.0.1] 2026-07-13 

This is the initial set of stagings for the 'release version' of the FADE emulator, distinct from the scratch versions that have been used for benchmarking up until now

### Added

* Simulated Annealing-based optimisation for the 'simple case' 
* Submodels successfully contained within a larger model
* Established a full heirarchical JSL::Interface::Aggregator system with full help message integration 
* A centralised 'parameter vector' that enables efficient slicing and storage 
 
### Changed

* Split the code up into a library + interface system; allowing for external users to use their own C++ interface rather than relying on the CLI
* More sophisticated build system; integration with FetchContent to automatically configure against a chosen JSL version and Eigen 
    * Updated to work with JSL 3.1.x
* Template-headers for most of the major systems, allowing them to swap out doubles for more complex objects that support basic arithmetic (i.e. duals for automatic differentiation)

### Removed

* The somewhat hacked together and overcomplicated previous version has been excised 

