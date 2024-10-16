# Contributing

Contributions are welcome! If you want to fix a bug or implement a feature yourself, feel free to make a development fork. Before you start working on a feature, you may first want to submit an issue where you describe the feature you want to add, to be sure that such a feature would be accepted.

## Dependencies

Apart from glank's n64 toolchain, you will need to install `clang-format` and `clang-tidy` of at least version 16.

Instuctions for adding the llvm apt repository for Ubuntu and Debian can be found [here](https://apt.llvm.org/).

If the only `clang-format` or `clang-tidy` versions available for you are older than 16, you will have to build clang from source. You can find everything you need for this at the [LLVM GitHub repository](https://github.com/llvm/llvm-project)

## Pull Requests

Before you create a pull request with your changes, make sure your code actually builds and anything you changed works. Next, run the `format.py` script to ensure that everything matches fp's code style. Once you've done that, you're ready to create a pull request! Just be sure that any commit messages and pull request names/descriptions accurately describe what your changes are to help us out while we're reviewing.
