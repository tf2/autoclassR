.onLoad <- function(lib, pkg){
   library.dynam("autoclassR", pkg, lib)
}
.onUnload <- function(libpath)
    library.dynam.unload("autoclassR", libpath)
