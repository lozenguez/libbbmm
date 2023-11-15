BbMm - a librayry for Bayesian-based Markov-models
======================================================================

![](./resources/logo-BbMm-128.png)


BbMm, aims to become a multi-language library to create deliberative agent based on Decision Making under Uncertainty.
The implemented approach for that relies on Markov Decision Process with transition based on Bayesian Network model.

BbMm is an experimental toolbox, it comes with no more guaranty than the devotion of the developers.

## Concurency:

**BbMm** is not what you looking for ? Look at those solutions:

- SaLinA : <https://arxiv.org/pdf/2110.07910.pdf> - <https://github.com/facebookresearch/salina>
- MDPToolBox :
- [LibDAI](https://github.com/dbtsai/libDAI) : C++ lib for Discrete Approximate Inference
- [gobnilp](https://www.cs.york.ac.uk/aig/sw/gobnilp) : Bayesian Network learning using Integer Linear Programming
- [torchhd](https://github.com/hyperdimensional-computing/torchhd) : Python lib for Hyperdimensional Computing (or Vector Symbolic Architectures) Artticle: https://jmlr.org/papers/v24/23-0300.html
- [farama](https://farama.org/) : à fouiller.


## Installation

_BbMm_ is mainly a C librairie with wrapers in differents languages, nativelly Python and (soon) Pharo.
It requires the compilation of the _c_ librairie (core).

The instruction and support is providing only for Linux based machine.
For conveignance, command is provided for debian like distribution.


### Dependancies :

The construction of the librairy relies on `Cmake`. Then `Check` is used for test-driven developments.

```sh
sudo apt install python3 build-essential cmake check
pip install pytest
```

### Compilation

As said compilation relies on `cmake`.
The `build.sh` script permits to build _BbMm_ localy.

From _BbMm_ directory:

```sh
./bin/build.sh
```

A `build` directory, the `libbbmm.so` librairy and the `test-bemage` executable should be generated.
You can run the `test-bemage`.

```sh
./test-BbMm
```

### Python

Python wrapper suppose that you built `libbbbmm.so` (cf. section: **Compilation**) . 
Then the python wrapper can be tested using pytest

```sh
pytest python-src
```

### Pharo

comming soon


## Get-in



## Contribution guidelines

* Writing tests
* Code review
* Other guidelines








### How do I get set up?

* Summary of set up
* Configuration
* Dependencies
* Database configuration
* How to run tests
* Deployment instructions

### Who do I talk to?

* Repo owner or admin
* Other community or team contact



