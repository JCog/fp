# Contributing

## Dependencies

Apart from glank's n64 toolchain, you will need to install clang-format and clang-tidy of at least version 14.

Instuctions for adding the llvm apt repository for Ubuntu and Debian can be found [here](https://apt.llvm.org/).

If the only clang-format or clang-tidy versions available for you are older than 14, you will have to build clang from source. You can find everything you need for this at the [LLVM GitHub repository](https://github.com/llvm/llvm-project)

## Pull Requests

Before you create a pull request with your changes, make sure your code actually builds and anything you changed works. Next, run the `format.sh` script to ensure that everything matches fp's code style. Once you've done that, you're ready to create a [pull request!](https://github.com/pmret/papermario/pulls) When creating a pull request, make sure that your commits and pull request name accurately describe what your changes are.
