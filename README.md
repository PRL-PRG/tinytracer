# tinytracer

A tiny tracer for figuring out the composition of SEXPs.

How to build:

```bash
./build
```

How to run:

```bash
SEXP_INSPECTOR_TYPES="sexp_types.csv" bin/R    # type analysis
```

# install Ubuntu packages

```bash
sudo apt-get update

sudo apt-get install subversion calibre rsync anacron xvfb xkb-data build-essential gfortran \
                     libreadline-dev time libglu1-mesa-dev xorg-dev bwidget fftw-dev graphviz-dev \
                     lam-runtime lam4-dev libboost-dev libgdal-dev libgmp3-dev libgsl-dev libperl-dev \
                     portaudio19-dev libquantlib0-dev pvm-dev libgtk2.0-dev libxml2-dev libhdf5-serial-dev \
                     imagemagick ttf-sjfonts ggobi libsprng2-dev libproj-dev dieharder libdieharder-dev \
                     libgd2-xpm-dev wordnet-dev poppler-utils locate libglade2-dev texlive-full ess auctex \
                     r-base r-recommended aasc-texmf-site jags libicu-dev libspreadsheet-parseexcel-perl \
                     libtext-csv-xs-perl libqt4-dev libxslt1-dev ant kdelibs5-dev xclip libmpfr-dev \
                     libtiff5-dev libitpp-dev xserver-xorg iwidgets4 libdb-dev noweb libfftw3-dev \
                     libprotobuf-dev libprotoc-dev protobuf-compiler libarmadillo-dev libmagickwand-dev \
                     libmpich-dev libopenmpi-dev openmpi-bin libudunits2-dev coinor-libsymphony-dev \
                     libscalapack-mpi-dev pari-gp qpdf libgeos-dev lua5.1 liblua5.2-dev libnetcdf-dev \
                     netcdf-bin libmpc-dev coinor-libclp-dev libjpeg-dev libboost-program-options-dev \
                     highlight libraptor2-dev default-jdk libglpk-dev openbugs libc6-dev-i386 libev-dev \
                     cmake mosek tk-table heirloom-mailx liboctave-dev libsndfile-dev libapparmor-dev \
                     coinor-libcgl-dev coinor-libosi-dev python-dev libpoppler-glib-dev python-pygments \
                     rst2pdf python-mechanize mongodb gdal-bin biber tcl-dev tk-dev python-numpy \
                     libhiredis-dev libnlopt-dev pandoc libv8-dev libzmq3-dev curl libapt-pkg-dev \
                     libsasl2-dev pandoc-citeproc librdf-dev ipython libsbml-dev saga libboost-regex-dev \
                     libboost-system-dev libsodium-dev libboost-iostreams-dev libpoppler-cpp-dev \
                     libhunspell-dev libimage-exiftool-perl libmagick++-dev libmagic-dev

sudo apt-get install ttf-kochi-gothic

sudo apt-get install libmysqlclient-dev libmariadb-client-lgpl-dev-compat libmariadb-dev

sudo apt-get install libssl-dev libssh2-1-dev gdal-bin libgdal-dev 

sudo apt-get install libboost-all-dev
```

# Downloading CRNA packages

The directory where we want to download CRAN is $CRAN_DIR. This is not the directory where it is going to be installed, just where our local repository will be located.

```bash
export CRAN_SOURCES="/data/kondziu/R/CRAN"
mkdir -p "$CRAN_SOURCES"
rsync -rtlzv --delete cran.r-project.org::CRAN/src/contrib "$CRAN_SOURCES"
```

# Installing Packages (CRAN and BioConductor)


Before running R, make sure you have DISPLAY set for an available X server, some packages need it; so if you are running on a remote server, you can do:

```bash
nohup Xvfb :6 -screen 0 1280x1024x24 >/dev/null 2>&1 &
export DISPLAY=:6
```

Create a fresh directory where packages should be installed, eg. /data/R/libs/. Set your R_LIBS to point to that directory:

```bash
export R_LIBS=/data/kondziu/R/installed
mkdir -p $R_LIBS
```

(TODO: set this variable in run_vignettes.sh etc. and add it to metadata)

Set all relevant environmental variables:

```bash
export R_COMPILE_PKGS=1
export R_DISABLE_BYTECODE=0
export R_ENABLE_JIT=0
export R_KEEP_PKG_SOURCE=yes
```

Then open up R and:

```R
dir.create("/tmp/install_R_pkg")
setwd("/tmp/install_R_pkg")

contrib.url <- paste("file://", "/home/kondziu/R/sources/CRAN" ,"/contrib/", sep="")

install_all_of_these <- function(packages) {
    already.installed <- installed.packages()
    
    installed_packages <- setdiff(packages, already.installed[,"Package"])

    install.packages(
        pkgs=installed_packages,             # which packages to install
        contriburl=contrib.url,              # where to install packages from
        # lib=R_LIBS,                        # where to install packages to
        Ncpus=8,                             # run in parallel using N cores
        #keep_outputs=T,                     # keeps outputs in ".out" files in current directory
        INSTALL_opts=c(
            "--byte-compile",                # byte-compile packages
            "--example",                     # extract and keep examples
            "--install-tests",               # copy and retain test directory for the package
            "--with-keep.source",            # keep line numbers
            "--no-multiarch"),
        dependencies = c("Depends")          # also install packages this package requires
        #    "Imports",
        #    "LinkingTo",
        #    "Suggests",
        #    "Enhances"))                
    )
}

source("https://bioconductor.org/biocLite.R")
library(BiocInstaller)

install_all_of_these_bioc <- function(packages) {
    already.installed <- installed.packages()
    installed_packages <- setdiff(packages, already.installed[,"Package"])
    biocLite(
        installed_packages,                  # which packages to install
        Ncpus=8,                             # run in parallel using N cores
        INSTALL_opts=c(
            "--byte-compile",                # byte-compile packages
            "--example",                     # extract and keep examples
            "--install-tests",               # copy and retain test directory for the package
            "--with-keep.source",            # keep line numbers
            "--no-multiarch"),
        dependencies = c("Depends")          # also install packages this package requires
    )
}

packages_status <- function() {
    already_installed_packages <- installed.packages()[, "Package"]

    all_cran_packages <- available.packages(contriburl=contrib.url)[, "Package"]
    already_installed_cran_packages <- intersect(all_cran_packages, already_installed_packages)

    all_bioc_packages <- available.packages(repos = biocinstallRepos()[1])[, "Package"]
    already_installed_bioc_packages <- intersect(all_bioc_packages, already_installed_packages)

    all_packages <- union(all_cran_packages, all_bioc_packages)

    data.frame(
        all_packages=length(all_packages), 
        all_already_installed_packages=length(already_installed_packages),
        cran_packages=length(all_cran_packages),
        cran_already_installed_packages=length(already_installed_cran_packages),
        bioc_packages=length(all_bioc_packages),
        bioc_already_installed_packages=length(already_installed_bioc_packages)
    )
}

get_packages_to_install_cran <- function () {
    all.packages <- available.packages(contriburl=contrib.url)
    already.installed <- installed.packages()
    setdiff(all.packages[,"Package"], already.installed[,"Package"])
}


get_packages_to_install_bioc <- function () {
    all.packages <- available.packages(repos = biocinstallRepos()[1])
    already.installed <- installed.packages()
    setdiff(all.packages[,"Package"], already.installed[,"Package"])
}


install_all_of_these(get_packages_to_install_cran())
install_all_of_these_bioc(get_packages_to_install_cran())
install_all_of_these_bioc(get_packages_to_install_bioc())
```
