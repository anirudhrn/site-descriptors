# site-descriptor

This program provides an interface into the [```CASM```](https://github.com/prisms-center/CASMcode) software package to calculate site-centric configurational descriptors. Site-centric configurational descriptors are calculated as :

<img src="https://latex.codecogs.com/gif.latex?G^{i}_{\delta}&space;=&space;\left(\frac{\sum_{\alpha\in\Omega_{\delta}^{i}}\prod_{j\in\alpha}&space;\sigma_{j}}{m_{\Omega_{\delta}^{i}}}\right)" title="G^{i}_{\delta} = \left(\frac{\sum_{\alpha\in\Omega_{\delta}^{i}}\prod_{j\in\alpha} \sigma_{j}}{m_{\Omega_{\delta}^{i}}}\right)" />

where ![equation](https://latex.codecogs.com/gif.latex?%5Cinline%20G%5E%7Bi%7D_%7B%5Cdelta%7D) is the site-centric configurational descriptor for site i and a prototype cluster labeled ![equation](https://latex.codecogs.com/gif.latex?%5Cinline%20%5Cdelta). ![equation](https://latex.codecogs.com/gif.latex?%5Cinline%20%5COmega_%7B%5Cdelta%7D%5E%7Bi%7D) is the set of symmetrically equivalent clusters, ![equation](https://latex.codecogs.com/gif.latex?%5Cinline%20m_%7B%5COmega_%7B%5Cdelta%7D%5E%7Bi%7D%7D) is the multiplicity of the prototype cluster and ![equation](https://latex.codecogs.com/gif.latex?%5Cinline%20%5Csigma_%7Bj%7D) is the occupation function value at site j.

## Installation

The build system here depends heavily on ```autotools```. The following packages are required to enable building this repository with ```autotools```:

* autoconf
* automake
* libtool
* autoconf-archive

These should be readily available via ```brew``` and ```apt```.

### Installing ```CASM``` with ```autotools```

Building and installing the ```CASM``` software package with ```autotools``` is the preferred method. You may need to do additional work if you want to link this tool with the ```CASM``` library that is installed through ```anaconda``` or other means. Start building by first cloning the ```CASM``` github repository into your computer. Next run the following commands within the cloned ```CASM``` directory:

``` sh
python make_Makemodule.py
./bootstrap.sh
```
If you do not see any errors, make a build directory and configure your ```CASM``` build:

``` sh
mkdir build
cd build
.././configure --prefix=$INSTALLDIR CXXFLAGS='-O3 -DNDEBUG'
```

where ```$INSTALLDIR``` is an environment variable containing the location where you want to install ```CASM```. This is an important variable to keep around, as we will later link to this directory while building ```site-descriptor```. Configuring your ```CASM``` build may take a few seconds but if you have all the software dependencies available you should not see any error messages and you will be ready to compile the library. To compile and install your library run:

``` sh
make -j4 install
```
where you should replace ```-j4``` with an appropriate number of processors over which to parallelize the build process. The compilation and installation may take a few minutes depending on the number of processors you are using. 

### Building this repository

First clone this repository. Assuming that ```CASM``` was built without any errors you should be able to build this tool in a similar manner. Begin by running the following command in the root of the directory:
```
./bootstrap.sh
```
This will generate a bunch of files needed by ```make``` and the configure script. Similar to the ```build``` directory for ```CASM```, create a separate directory:
```
mkdir build
cd build
```
At this point you are ready to run ```../configure```, but there's a few flags you want to be aware of:
* ```--prefix``` will determine where you want your executable installed. For example, I like keeping my own executables away from the root folders that require admin privileges, so I tend to do ```--prefix=$HOME/.local```.
* ```LDFLAGS``` will set the ```-L``` flags for the compiler. You'll need to specify directories with this flag if you want the compiler to look anywhere for libraries. Unless you've installed the casm libraries in a particular manner, the configure script won't be able to find them. You'll probably need to do something like ```LDFLAGS=-L$HOME/.local/lib```
* ```CPPFLAGS``` will set preprocessor flags for you, namely ```-I``` for the compiler. Unless you've got the casm headers in a standard location, or have defined their path in ```CPLUS_INCLUDE_PATH```, ```make``` will be unable to include any headers. You'll need something like ```CPPFLAGS=-I$HOME/.local/include```.
* ```--program-transform-name``` will let you rename the resulting executable, which by default is called ```site-descriptors```. If you'd rather it be named ```casm-fantastic``` you can do ```--program-transform-name='s/site-descriptors/casm-fantastic```

Once you've decided what flags to use you can run the configure script:
```
CPPFLAGS=-I$INSTALLDIR/include LDFLAGS=-L$INSTALLDIR/lib ../configure --prefix=$HOME/.local
```
Depending on your setup, you may need ```LDFLAGS``` to also include ```-ldl``` for the configure script to find the casm libraries.
For convenience, I recommend you save the command above for when you inevitably come back later and forget what was typed during compilation.
An additional caveat may involve the executable saying something along the lines of ```cannot find libcasm.so``` after compilation is installed.
One solution seems to be do add ```$INSTALLDIR/lib``` (or wherever the lib folder with ```casmlib.so``` is) to the ```LD_LIBRARY_PATH``` environment variable.

You can make and install your executable with:
```
make && make install
```

## Running and using ```site-descriptors```

To run the executable you need to ensure that you have completed the following three steps:
* Initialize a ```CASM``` project with a ```PRIM``` or ```prim.json```
* Enumerate the ```clexulator``` with the ```casm bspecs -u``` command and an appropriate ```bspecs.json``` file in the ```$CASMROOT/basis_sets/bset.default``` folder. Ensure that this file is compiled without any errors when you run this command
* Select all the configurations that you want to calculate site correlations for with the ```casm select``` command. Only selected configurations will be listed in the final output of this program

Once you have completed the above steps, you are ready to calculate site-centric correlations. Assuming the executable is in your path, run:

``` sh
site-descriptors <location of CASM project> <location and name of the output file>
```

The output file has the following schema:

``` json
{
"<configuration name>": {
      "corr_matrix": "This matrix contains as many rows as the number of basis functions 
                      in your project and as many columns as the number of sites in the 
                      configuration unit cell",
      "b":"This list contains as many entries as the number of sites in the configuration supercell. 
           Each entry is the index of the supercell site into the primitive cell."
    }
}
```

The ```corr_matrix``` is likely the entry you are looking for while developing site-centric configurational descriptors. The entries in the field labeled ```b``` can be useful while trying to identify which sites are symmetrically distinct. For example consider a primitive cell containing 5 sites. Let these sites be labeled ```[0,1,2,3,4]```. In a supercell with a volume of 2 relative to the primitive cell you will find an output ```b``` vector that looks like:

``` json
{
    "b":[0,0,1,1,2,2,3,3,4,4]
}
```
This helps identify how the columns of the ```corr_matrix``` may be mapped on to specific sites in the primitive unit cell. These indices can be very useful while constructing energy models that describe the effect of configurational disorder. Primitive basis sites that belong to the same asymmetric index are readily identified by investigating the point clusters within the ```clexulator```. While constructing energy models, asymmetric sites that are the same can be represented with a single energy model while asymmetric sites that are symmetrically different require a different energy model.


## Citing
If you find this tool useful please cite the following paper in addition to the papers listed in main [```CASM```](https://github.com/prisms-center/CASMcode) repository.

```
Natarajan, A.R., Van der Ven, A. Machine-learning the configurational energy of multicomponent crystalline solids. npj Comput Mater 4, 56 (2018). https://doi.org/10.1038/s41524-018-0110-y
```

``` bibtex
@article{natarajan2018,
  title = {Machine-Learning the Configurational Energy of Multicomponent Crystalline Solids},
  author = {Natarajan, Anirudh Raju and {Van der Ven}, Anton},
  year = {2018},
  volume = {4},
  issn = {2057-3960},
  doi = {10.1038/s41524-018-0110-y},
  journal = {npj Computational Materials},
  number = {1}
}
```

