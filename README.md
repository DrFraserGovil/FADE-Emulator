# FADE Emulator

The Flexible-Anisotropic Department-of-Expert is a full distributional emulator, designed to predict $p(y | \mathbf{x})$, conditioned on a set of data which may be both intrinsically stochastic, or have stochasticity induced by a hidden set of latent variables. 

## Installation & Compilation

The code can be copied locally by cloning it:

```
git clone https://github.com/DrFraserGovil/FADE-Emulator
```

The code must then be compiled, which requires:
1. A C++20-compliant compiler (such as `cc-13` `lang-16` `pple-clang-15` `svc-2019`
2. `make >= 3.15`
3. An internet connection (for `etchContent`calls, which grab [Eigen](https://libeigen.gitlab.io/) and [a library by the FADE author](https://github.com/DrFraserGovil/JSL))

The build system can be activated by calling:

```
cd FADE-Emulator
make
```
 
This has been [validated to compile on Ubuntu, macOS and Windows](https://github.com/DrFraserGovil/JSL/actions/). macOS and linux users will find the `ade`binary appearing in their current working directory; MSVC users will find `ade.exe`inside a directory depending on their default compiler options -- probably `ebug/fade.exe`



## Using the Code

FADE is available for use as either a linked C++ library, or as a standalone binary. This documentation is for using the binary.

### Logging

FADE uses the [custom JSL Logger](https://jack-standard-library.readthedocs.io/en/stable/docfiles/Log.html#log) to write the output. This allows some of the output to be suppressed when you want the code to be `quiet', or output much more data when you want to know what has gone wrong.

* `/fade ...`gives the normal output level
* `/fade --quiet ...`suppresses all output except error messages
* `/fade --verbose ...`produces lots of output which can be used for diagnostics, but it unsuitable for general use


### Configuration

The FADE Emulator has a large number of parameters that can be modified at runtime. You may view them by running:

```
./fade help
```

This activates the help menu, and details which values which can be changed by the user, and a description of what they do. Most settings have a similar name to the names used in this document (though not always). These settings are grouped into named groups, but this has no meaning for the user, other than helping to group parameters that do similar things together.

Changing the value is done in one of two ways:

#### Command Line Arguments

Running the `/fade`executable and adding extra arguments after it:

```
./fade train -expert 3,3 -model my_file.fde -file training_data.dat
```

This will attempt to launch the training module with:

* Only 3 experts (the range $3 \to 3$, see below for multi-model training syntax)
* Beginning the training at the previous model defined by my_file.fde
* Using the training data stored in training_data.dat
* All other values will use their default value.

Note that some parameters have multiple aliases which can be used: `input`and `-file`have the same meaning. This is detailed in the help menu.

#### Configuration files

As you tweak more and more values, the number of commands given to run the code will get longer and longer. Instead, you can write your data to a config file:

```
// test.cfg
expert 3,3
model my_file.fde
file training_data.dat
(...)

>> ./fade --config test.cfg
```

This loads the data in the config file, and reads it as if it were passed as arguments to the command line.

#### Mixed-Mode

It is possible to use both a config file and command line arguments:

```
 ./fade --config test.cfg -model other_model.fde
```

In this case, the config file is loaded first, and then the command line values are applied, overwriting any values previously set: in the example above, the value passed to model is other_model.fde: the one given at the command line and not my_file.fde (the one in the config file).

**To dash or not two dash?**

When specifying command line variables, any number of dashes are allowed: --flag and -flag are always equivalent. The general advice is to use one dash for short commands (-f) and two for long ones (--model), but this is not a requirement.

When specifying variables in a config-file, do not use leading dashes.
### Filetypes

FADE is largely indifferent to the file extensions which are given to it. By default, models are saved with the custom extension "`fde`, though this is not mandated. `fde`files are [tar archives](https://en.wikipedia.org/wiki/Tar_(computing)), and may be extracted either with an external manager (though on Windows, they may not like the lack of proper file extensions). Alternatively, the code comes with a custom unpacker:

```
fade unpack model.fde
```

This will create a directory called `odel` which contains all of the internal model files.

## Training Models

In order to create a new model, the user must provide **training data**. The expected format is as follows (assuming $\mathbf{x}$ is multidimensional, and $y$ is unidimensional):
 
```
x_1 x_2 (...) x_n z_1 y_1
w_1 w_2 (...) w_n z_2 y_2
(...)
```

This represents the observation tuples $(\mathbf{x}, \zeta_1, y_1)$ and ($\mathbf{w}, \zeta_2, y_2)$, where $\mathbf{x}$ is the point in emulation space, $\zeta_i$ is the **prior weighting** (if you don't know what this means, use $\zeta = 1$), and $y$ is the observed value. Values of $\mathbf{x}$ are not required to be unique; in the case of stochastic models or latent variables, it will most likely be the case that there are manu such values.
 
 Once the training data is in the correct format, the model may be trained:

```
./fade train --file training.data --save my_model.fde --expert 3,6 --dep 2,4
```

This will generate a model called `y_model.fde`which has been trained on the provided data. This will also simultaneously train the set of submodels for $3 \leq N_e \leq 6$ and $2 \leq N_d \leq 4$ (for a total of 12 individual models). At present, this is merely for convenience, as no posterior predictive is generated.

## Using Models


Once a model has been trained, it can be used to make predictions of the distribution function $p(y| \mathbf{x})$. In order to achieve this, the user must write a file with the following syntax:

```
//qfile.dat
x_0 x_1 x_2 (...) x_n y_1 y_2 y_3 (...) y_m
w_0 w_1 w_2 (...) w_n u_1 u_2 u_3 (...) u_m
(...)
```

As with the training data, the `\_0 ...x\_n`specify the values of $\mathbf{x}$, a point in emulation space to be queried. The values of $y_1...y_m$ form an array of samples of the density that the model will be evaluated on: so specifying `\_0 y\_1 y\_2 ..`means `t position $\mathbf{x}$, tell me the probability density at $y_0$, then $y_1$, then $y_2$.'' There is no requirement that $\{y_i\}$ be sorted or uniform.

Multiple $\mathbf{x}$-queries can be entered, with each new $\mathbf{x}-y$ prediction entered on a new line.

The prediction is then made by calling:

```
./fade predict --query qfile.dat --model my_model.fde
```

The output of a prediction is a second data file (with name configured by `query-out`). If the model which was queried was trained on multiple $N_e, N_d$, then a separate query file (denoted by `[name]\_Nd\_Ne.dat`) is written for each pair.

For each $\mathbf{x}-y$ specification in the query file, the output file takes the form:

```
New query: x_0 x_1 (...) x_n
y_1 y_2 y_3 (...) y_m
p_1 p_2 p_3 (...) p_m
//repeat for each x-y in query
```
Where:
$$ p_i = p^\text{fade}(y_i | \mathbf{x},\mathbf{\theta}) $$
Where $\mathbf{\theta}$ is determined by the model which has been loaded, and is the MAP estimate of the model parameters.

