*Code Quality Checks*

- Abstraction Go down one abstraction level per function call
- Multi step processes should be constructed in a way that makes it impossible to skip a step. For example, a read operation should only be available as a method of an object representing a successfully  opened-file.
- Every logging line is marked by both module and level
- Code has no side effects
- only one exit point from a function
- Initialize all variables
- Wrap calls to libraries for easier replacement
- All names in code (variables, functions, classes etc) are meaningful and used for what they are named for
- Function length is maximum 20 lines
- No more than one loop in a function
- Functions have maximum of three parameters
- Classes have few members and methods.
- All functions that can fail return a return-code structure.
- The code assumes that everything that can fail, will fail.
- All REST and replies calls are logged in full details
- All  input is checked for correctness
- Code is fully Doxygen-ed
- comment on what, not how.
- If an object can be declared as const, make it const.
- All code is covered with unit tests - 100% of code lines.


*Design Compliance Checks*

- All input data comes with:
  - Error specification in %
  - Units
  - Data Lineage record
- All output data is created with:
  - Error specification in %
  - Units
  - Data Lineage Record
- Error messages contain:
  - What the software tried to do
  - What went wrong
  - How to remedy the problem
  - As many relevant details as are at hand at the time the error occurred
- Numbers are either integers or double-precision. Do not use single precision.
- For anything that is not the novel core of the software, well-established external libraries with an appropriate license should always be used instead of writing new code.
- All file formats should be well established cloud oriented format (eg JSON)
- Heavy calculations are always checkpointed and restartable from a checkpoint
- The user can always get the state of a heavy calculation so that they at  least have a general idea how long it has been running and how long it has to run, in both wall time and percentage completed.
- All input, configuration, software versions and intermediary data is saved at the back end so that any calculation can be repeated exactly and produce the same result.

*Development Process Checks*
- All tests have passes
- Code is 100% covered
- Every Function or type or constand have a Doxygen documentation. Function 
documentation includes what the function does, documentation of each parameter and of the return value