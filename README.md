This is the repository containing my work on Senior Project at Cal Poly.

## Project Information
__Author:__ J. Ethan Adams (jadams30@calpoly.edu)

__Project Advisor:__ Dr. John Bellardo (http://users.csc.calpoly.edu/~bellardo/index.php)

----------

## Project Background
This project is a fork of my `csc454` project where I developed much of the basic functionality of a simple `x86_64` Operating System. Work for senior project will pick up where I left off in this class (to see a description of my previous work, see https://github.com/jadams41/Senior-Project/blob/master/archived/CPE454_README.md)

----------

## Project Scope and Overview
The goal of this to develop a custom, functional `x86_64` *unix-like* Operating System.

#### Project Objectives
1. OS includes robust implementation of all functionality outlined in the project for `csc454`.
2. Fully implemented and functional networking stack (capable of communicating across layer-3 networks via `TCP/IP`).
3. OS can be built for physical hardware targets. All implemented OS features will be supported and verifiably functional for each supported hardware target.

_NOTE: more objectives will likely be added once `Objective 2` has been accomplished._

---------

## Project Planning

### Milestones
#### Milestone 0: *Set up Senior Project and Document Project Scope*
  1. Take care of administrative tasks for setting up Senior Project
  
    - [ ] Create project proposal documenting scope and objectives.
    - [ ] Procure project advisor.
    - [ ] Submit Pre-Enrollment Proposal to officially enroll in Senior Project I.
    
  2. Set up Project
  
    - [ ] Create GitHub Repository for Documenting project information
    - [ ] Populate project's `README` with initial information regarding project background, scope&objectives, and tentative timeline of critical work.
    - [ ] Forward pdf of Project proposal + GitHub repository to project advisor.

#### Milestone 1: *Necessary Maintenance and Preparations for New Development*
  1. Revise previous work
  
    - [ ] Review all of previous work - make sure functionality and limitations are understood and documented.
    - [ ] Fix identified erroneous/limited implementation.
    
  2. Create a more robust and stable development environment.
  
    - [ ] Review and document functionality of current build system (make).
    - [ ] Migrate build of project (operating system) to use `bazel`.
    - [ ] Document how to build and run project with new build system.
    - [ ] Research, implement, and document project testing strategy.
    - [ ] _Explore possibility of migrating project from `C` to `C++`._
    
  3. Complete all `cpe454` functionality milestones
  
    - [ ] Review and document initial project's accomplishment of milestones.
    - [ ] Fix all partially implemented functionality milestones.
    - [ ] Implement remaining non-attempted milestones.
    
#### Milestone 2: *Implementation of Network Stack*

#### _Potential Future Milestones_
  1. Implement better graphics support.
  2. Improve process control capabilities.
  3. Implement disk-encryption support.

----------
