# Changelog

<!-- All notable changes to this project will be documented in this file. -->

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [0.3.0] 2026-09-03

### Added

* Implemented the auto-range detection for inference bounding
* Added the CDF as a line in the output files
* BLP-autoscaling now allows per-dimensional assignment of lengthscales, and the setting of scales as fractions of the span
 
### Changed

* Updated the readme to reflect the changes induced in 0.2.0
* The -expert and -dep flags are now single values (the pair-args are now -expertRange and -depRange)
### Removed

* A handful of settings which had no effect on how the code functions

## [0.2.0] 2026-09-03

### Added

* Added version-querying support
* The positions of experts are now generated on a Latin Hypercube rather that a pure uniform random distribution 
* A nice visual display of the training progress 

### Changed

* Updated the build system to the more streamlined approach
* Updated to JSL 3.3.4
* Overhauled the training data to emphasis replicate importance: training data now innately allows clustering
* Improvements made to the optimisation algorithm 

## [0.1.0] 2026-08-06

This is the pre-release version of the code which is being tested by our students. This does not yet represent the full scope of the FADE Emulator (it is missing the Posterior Predictive and the associated Hessian-estimation) .

### Added

* A revised theory document, with a User Manual for the code
* A new probability model, based on parameter interpolation, rather than distributional interpolaiton
* A new GEM fitting model for the per-Expert parameters, and the mathematical theory to derive it 
* A new testing interface for validation 
* A set of github hooks for cross-platform validation. FADE is confirmed to compile on Ubuntu, macOS and Windows.
* Mean trend handled by an internal BLP model 
 
### Changed 

* Updated priors to be less spurious
* Updated the training model to explicitly support x-clustering
* Pegged the library to compile against [JSL Version 3.2.0](https://github.com/DrFraserGovil/JSL/releases/tag/v3.2.0)
* The FADE library now lives inside ``lib`` rather than ``FADE`` -- this is for avoidance with the compiled binaty (``fade``) on case-insensitive OSes
### Removed

* Parallelisation (will be added later, but removed for simplicity)

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

