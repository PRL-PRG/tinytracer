suppressPackageStartupMessages(library("dplyr"))
suppressPackageStartupMessages(library("tibble"))

# Given the name of the package retrieve a list of its vignettes, also including
# the information about the path of the vignettes' files and source code:
#
# > list_vignettes_of_package("stringr")
# # A tibble: 2 x 6
#     type     package item                directory       source                                loc                            
#     <chr>    <chr>   <chr>               <chr>           <chr>                                 <int>                           
#   1 vignette stringr stringr             .../stringr/doc .../stringr/doc/stringr.R             142
#   2 vignette stringr regular-expressions .../stringr/doc .../stringr/doc/regular-expressions.R 152
list_vignettes_of_package <- function(package_name) {
  
  # Retrieves information about the package's vignettes and returns an
  # packageIQR object. This object contains a few fields: type, title, header,
  # results, and footer. These are used to display the object in rstudio etc.,
  # we only need results. The results are a matrix containing columns: Package,
  # LibPath, Item, and Title. Package is the name of the package, LibPath is the
  # path of the directory where the package is installed, Item contains the name
  # of the vignettes, and Title contains their titles.
  vignette_descriptions <- vignette(package=package_name)$results
  
  # Glue together a path where the vignettes' files should be. That is:
  # <LibPath>/<Package>/doc/
  vignette_dir_locations <- paste(vignette_descriptions[,"LibPath"], 
                                  vignette_descriptions[,"Package"], 
                                  "doc", sep="/")
  
  # Glue together a path where the vignettes' sources should be. That is:
  # <LibPath>/<Package>/doc/<item>.R
  vignette_source_locations <- paste(vignette_dir_locations, 
                                     paste0(vignette_descriptions[,"Item"], ".R"), 
                                     sep="/")
  
  # Create a private little lambda to count lines of code. It opens up the file
  # and counts the number of lines which do not begin with a comment and which
  # are not blank.
  count_lines_of_code <- function(path) 
    if (file.exists(path)) 
      length(!grepl("^[ \t\n]*#", grepl("[^ \t\n]", readLines(path))))
    else 0
  
  # Apply the counting function to all the source files.
  vignette_lines_of_code <- sapply(vignette_source_locations, 
                                   count_lines_of_code)
  
  # Return a tibble containing the names of the package and vignettes, the path
  # to its directory and sources.
  tibble(type="vignette",
         package=package_name,
         item=vignette_descriptions[,"Item"],
         directory=vignette_dir_locations,
         source=vignette_source_locations,
         loc=vignette_lines_of_code)
}

#instrument_vignette <- function(element