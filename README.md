# pynuitrack
**pynuitrak** is a python wrapper for Nuitrack skeleton tracking library.

> **Important:** Although this wrapper is free software, Nuitrack _is not_.
> You can acquire a _trial_ or _pro_ license at
> [Nuitrack website](https://nuitrack.com/).

## Build (GNU/Linux)

pynuitrack depends on the
[Nuitrack SDk](http://download.3divi.com/Nuitrack/doc/Installation_page.html)
and [Boost C++ libraries](https://www.boost.org/), version 1.63 or higher.
Nuitrack SDK must be [downloaded](http://download.3divi.com/Nuitrack/) and
extracted on your workspace, while Boost can be downloaded via your Operating
System (OS) package manager (e.g. `apt`) or on
[boost website](https://www.boost.org/users/download/). If your OS supports an
older version of Boost (e.g. Ubuntu 16.04 ships with Boost 1.58), you will need
to download and compile Boost from source.

### Step 1: Set-up Nuitrack and Nuitrack SDK

[Download](http://download.3divi.com/Nuitrack/platforms/) and install Nuitrack
for your platform. Make sure that you have acquired a _trial_ or _pro_ license
before using it.

After installing Nuitrack, make sure that the environment variables 
`NUITRACK_HOME` and `LD_LIBRARY_PATH` are set correctly by running:

```bash
$ echo $NUITRACK_HOME   # Should be /usr/etc/nuitrack
$ echo $LD_LIBRARY_PATH # Should include /usr/local/lib/nuitrack
```

If those variables are not set, open your `~/.bashrc` file in a text editor and
add the following lines:

```bash
export NUITRACK_HOME="/usr/etc/nuitrack"
export LD_LIBRARY_PATH={$LD_LIBRARY_PATH}:"/usr/local/lib/nuitrack"
```

Run `nuitrack_license_tool` to register your copy of nuitrack:

```bash
$ nuitrack_license_tool
```

Now download [Nuitrack SDK](http://download.3divi.com/Nuitrack/NuitrackSDK.zip)
and decompress it to your working directory. Herein your working directory is
assumed to be your home folder `~/`. Please change it according to your needs.

```bash
$ cd ~
$ mkdir NuitrackSDK
$ wget http://download.3divi.com/Nuitrack/NuitrackSDK.zip
$ unzip NuitrackSDK.zip
```

### Step 2: Install Boost

You probably already have Boost installed on your system. You can check that by
running:

```bash
$ ldconfig -p | grep libboost
```

If Boost is not installed, you can install it by running:

```bash
$ sudo apt-get update
$ sudo apt-get install libboost*
```

If the version of the Boost library provided by your distribution is lower than
1.63, pynuitrack won't be able to compile, as it depends on `libboost_numpy`.
To download and install boost from source, follow **Steps 1** and **5** of the
[official instructions](https://www.boost.org/doc/libs/release/more/getting_started/unix-variants.html).
If you do not want to install the newer version library to avoid breaking your
system (which is a reasonable decision), you can use:

```bash
$ ./b2 stage
```

instead of `./b2 install` as suggested by the official instructions.

### Step 3: Cloning this repository and building

Finally, clone this directory:

```bash
$ cd ~
$ git clone https://github.com/silasalves/pynuitrack.git
```

Now open `CMakeLists.txt` file and edit line 6 to point to the Nuitrack SDK
directory. If you are using a custom version of Boost, uncomment and edit lines
10 and 11 as well.

Now, create the build folder, run cmake and build the project:

```bash
$ cd ~/pynuitrack
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Now copy the `pynuitrack.so` library found in your build directory to the same
folder of your Python scripts or add this line to your `~/.bashrc` file:

```bash
export PYTHONPATH=$PYTHONPATH:"~/pynuitrack/build"
```