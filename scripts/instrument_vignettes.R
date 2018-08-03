suppressPackageStartupMessages(library("dplyr"))
suppressPackageStartupMessages(library("tibble"))
suppressPackageStartupMessages(library("stringr"))
suppressPackageStartupMessages(library("compiler"))

# Given the name of the package retrieve a list of its vignettes, also including
# the information about the path of the vignettes' files and source code:
#
# > list_vignettes_of_package("stringr")
# # A tibble: 2 x 7
#     type     package item                source_directory data_directory  source_file                           loc                            
#     <chr>    <chr>   <chr>               <chr>            <chr>           <chr>                                 <int>                           
#   1 vignette stringr stringr             .../stringr/doc  ../stringr/data .../stringr/doc/stringr.R             142
#   2 vignette stringr regular-expressions .../stringr/doc  ../stringr/data .../stringr/doc/regular-expressions.R 152
list_vignettes_for_package <- function(package_name) {
  
  # Retrieves information about the package's vignettes and returns an
  # packageIQR object. This object contains a few fields: type, title, header,
  # results, and footer. These are used to display the object in rstudio etc.,
  # we only need results. The results are a matrix containing columns: Package,
  # LibPath, Item, and Title. Package is the name of the package, LibPath is the
  # path of the directory where the package is installed, Item contains the name
  # of the vignettes, and Title contains their titles.
  vignette_descriptions <- vignette(package=package_name)$results
  
  # Glue together a path where the vignettes' files should be. That is:
  # <LibPath>/<Package>/doc/ for the vignette sources and 
  # <LibPath>/<Package>/data/ for additional data files.
  source_directories <- file.path(vignette_descriptions[,"LibPath"], 
                                  vignette_descriptions[,"Package"], "doc")
  data_directories <- file.path(vignette_descriptions[,"LibPath"], 
                                vignette_descriptions[,"Package"], "data")
  
  # Glue together a path where the vignettes' sources should be. That is:
  # <LibPath>/<Package>/doc/<item>.R
  source_files <- file.path(source_directories, 
                            paste0(vignette_descriptions[,"Item"], ".R"))
  
  # Create a private little lambda to count lines of code. It opens up the file
  # and counts the number of lines which do not begin with a comment and which
  # are not blank.
  count_lines_of_code <- function(path) 
    if (file.exists(path)) 
      length(!grepl("^[ \t\n]*#", grepl("[^ \t\n]", readLines(path))))
    else 0
  
  # Apply the counting function to all the source files.
  lines_of_code <- sapply(source_files, count_lines_of_code)
  
  # Return a tibble containing the names of the package and vignettes, the path
  # to its directory and sources.
  tibble(type="vignette",
         package=package_name,
         item=vignette_descriptions[,"Item"],
         source_directory=source_directories,
         data_directory=data_directories,
         source_file=source_files,
         loc=lines_of_code)
}

# Vignettes can depend on files present in the vignette folder. These files may
# or may not be necessary for the vignettes to run. This creates a temporary
# folder and copies all those files so that the vignette has a private copy of
# each file.
prepare_working_directory <- function(working_directory, 
                                      source_directory, data_directory) {

  # Create a working directory to copy files to.
  dir.create(working_directory, recursive=TRUE)
  
  # Get the list of files in the vignette's directory. This should be recursive,
  # but should not include . and .. files. 
  source_files <- list.files(source_directory, all.files=TRUE,
                             include.dirs=TRUE, no..=TRUE, full.names=TRUE)
  
  # Get the list of files in the package's data directory. This should also be
  # recursive, but should not include . and .. files.
  data_files <- list.files(data_directory, all.files=TRUE,
                           include.dirs=TRUE, no..=TRUE, full.names=TRUE)
  
  # Combine the file lists and copy all files from the vignette's directory to
  # the working directory.
  sapply(c(source_files, data_files), function(file) {
    file.copy(from=file, to=working_directory, recursive=TRUE, 
              overwrite=TRUE)
  })
}

# Reads the source file pointed to by path and instruments all sections that
# expect errors by wrapping them in a try call.
read_file_and_instrument_error_blocks <- function(path) { 
  
  # Read in the entire source file from the specified path. Now we have a vector
  # of strings, each element is a single line.
  lines <- readLines(path)
  
  # We iterate through the source. With each line we either transform it in one
  # of several ways or return it as is. The transformation depends on the state
  # of the automaton (the prepend variable) and whether a line fits one of two
  # patterns expressed as regular expressions. We apply changes to the source
  # code in-place, to prevent expanding a vector over and over.
  i <- 0
  prepend <- FALSE
  for (line in lines) {
    i <- i + 1
    
    # Check whether the line fits an error on pattern. This means that we are
    # beginning a section that the vignette expects to error out. It's a mostly
    # case-insensitive comment that says:
    #   # error = TRUE 
    # We check this here because it's not cheap and we use it in several places.
    error_on_pattern_found <- 
      str_detect(line, "^[ \t]*##.*[eE][rR][rR][oO][rR] *= *T(RUE)?")
    
    # This case means we are encountering the start of an error section and it's
    # not been preceded by another error section. In response we set the prepend
    # flag to indicate that we are inside of an error section and we add the
    # begining of a try call to the code at the beginning of this line.
    if (!prepend && error_on_pattern_found) {
      prepend <- TRUE
      lines[i] <- paste0("try({", line)
      
    # This case means we are encountering the start of an error section while
    # exiting another error section. We react by adding code that end the
    # previous try call and starts another one.
    } else if (prepend && error_on_pattern_found) {
      lines[i] <- paste0("}, silent=TRUE); try({", line)
      
    # This means that we are encountering the start of a section that is not an
    # error. We also check if we were previously in an error section or not. If
    # we were not, then we don't care about this case. If we were in an error
    # section and encounter the start of a non-error section we have to finish
    # the previous try instruction and unset the prepend flag.
    } else if (prepend && !error_on_pattern_found && 
               str_detect(line, section_pattern)) {
      prepend <- FALSE
      lines[i] <- paste0("})", line)
    }
  }
  
  # If we finished looking through the entire file and the last section was an
  # error section, we need to finish the invocation of try, so we append it to
  # the end of the code.
  if (prepend) 
    lines[length(lines)] <- paste0(lines[length(lines)], "}, silent=TRUE)")
  
  # Return the modified code.
  lines
}

# Compiles the vignette source file into bytecode and creates another file that
# can be used to execute the compiled code. In other words, if the vignettes is
# compiled you should run the loader to execute it.
compile_vignette_to_bytecode_and_create_loader <- function(source_path, 
                                                           compiled_path, 
                                                           loader_path) {
  cmpfile(source_path, compiled_path)
  loader_source <- paste0("compiler::loadcmp('", compiled_path, "')")
  write(loader_source, file=loader_path)
}

# Lists all vignettes for a list of packages. If a list of packages is not
# provided, it lists vignettes for all installed packages. The list of vignettes
# includes additional information about each of them. See
# `list_vignettes_for_package` for more details.
list_vignettes_for_packages <- function(packages=installed.packages()[, 1]) 
  do.call(rbind, lapply(packages, list_vignettes_for_package))

# Take the information about vignettes we extract from R and add a few
# additional pieces of information that we will used to preapre files for
# execution and execute the vignettes. It adds the following columns:
#
#   working_directory               
#   <chr>                                                                                   
# 1 .../stringr/stringr             
# 2 .../stringr/regular-expressions 
#
#   instrumented_source_file
#   <chr>
# 1 .../stringr/stringr/stringr.instrumented.R
# 2 .../stringr/regular-expressions/regular-expressions.instrumented.R 
#
#   compiled_source_file                                          
#   <chr>                                                  
# 1 .../stringr/stringr/stringr.instrumented.Rc
# 2 .../stringr/regular-expressions/regular-expressions.instrumented.Rc
#
#   runnable_source_file      
#   <chr>
# 1 .../stringr/stringr/stringr.loader.R 
# 2 .../stringr/regular-expressions/regular-expressions.loader.R
#
provide_instrumentation_info <- function(info, 
                                         working_directory_root, 
                                         compile=TRUE) {
  
  # Every vignette has its own working directory. Vignette working directories
  # from the same package go in the same parent directory named after the
  # package in the working directory root:
  #     <working_directory_root>/<package>/<vignette>/
  working_directories <- file.path(working_directory_root, 
                                   info$package, info$item)
  
  # This is just auxiliar - we need bare file names of source files multiple
  # times: just filenames without paths or extensions.
  names <- basename(tools::file_path_sans_ext(info$source_file))
  
  # For every vignette we will spit out some instrumented source code. This will
  # go into the working directory into a file that has the same name as the
  # original source code file and the extension ".instrumented.R"
  instrumented_source_files <- file.path(working_directories, 
                                         paste0(names, ".instrumented.R"))
                                         
  # If we have compilation turned on, we will produce a byte-code compiled
  # version of the instrumented code. This will go to into the working directory
  # into a file with the same name as the source, but the extension
  # ".instrumented.Rc". If compilation is turned off this is an NA vector.
  compiled_source_files <- (
    if (compile) file.path(working_directories, paste0(names, ".instrumented.Rc"))
    else NA
  )
  
  # If we have compilation turned on, we need to also generate a small loader
  # script for the byte-code file and use that to run the vitgnette. This will
  # go to into the working directory into a file with the same name as the
  # source, but the extension ".loader.R". If compilation is turned off we
  # simply use the instrumented source file.
  runnable_source_files <- (
    if (compile) file.path(working_directories, paste0(names, ".loader.R"))
    else         instrumented_source_files
  )
  
  # Copy the existing information into a new structure and add the new
  # information as columns.
  add_column(info, working_directory=working_directories,
                   instrumented_source_file=instrumented_source_files,
                   compiled_source_file=compiled_source_files,
                   runnable_source_file=runnable_source_files)
}

# TODO
# prepare_working_directories <- function(extended_info)
#   lapply(extended_info)
#   prepare_working_directory(working_directory, source_directory, data_directory)
# 
# main <- function() {
#   info <- list_vignettes_for_packages(c("stringr", "dplyr"))
#   
#   info <- provide_instrumentation_info(info,
#                                        working_directory_root="/tmp/vignettes",
#                                        compile=TRUE)
}