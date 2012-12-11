acwritedatafile <- function(filename, d) {
	filename = paste(filename, ".db2", sep="")
	write.table(d, filename, sep="\t", row.names=F, col.names=F, quote=F)
}

acwriteheaderfile <- function(filename, nattributes) {
	filename = paste(filename, ".hd2", sep="")
	write.table('num_db2_format_defs 1', filename, sep="\t", row.names=F, col.names=F, quote=F);
	write.table(paste('number_of_attributes',nattributes, sep=" "), filename, row.names=F, col.names=F, quote=F, append=T);
	for (x in 0:(nattributes-1)) { 
  		row = paste(x, ' real location a', x, ' error 0.00001', sep="")
  		write.table(row, filename, row.names=F, col.names=F, quote=F, append=T);
  	}
}

acwritemodelfile <- function(filename, nattributes) {
	filename = paste(filename, ".model", sep="")
	write.table('model_index 0 1', filename, sep="\t", row.names=F, col.names=F, quote=F);
	satt = NULL
	for(x in 0:(nattributes-1)) {
		satt=paste(satt, x, sep=" ")
	}
	write.table(paste('multi_normal_cn', satt, sep=""), filename, row.names=F, col.names=F, quote=F, append=T);
}

# just example for moment - add all available parameters as list into function shortly - also file writing will be redundant soon anyway - all parameters and data passed directly as objects to C
acwritesparamsfile <- function(filename, max_n_tries=10) {
	filename = paste(filename, ".s-params", sep="")
	write.table('save_compact_p = false', filename, sep="\t", row.names=F, col.names=F, quote=F);
	write.table(paste('max_n_tries = ', max_n_tries, sep=""), filename, row.names=F, col.names=F, quote=F, append=T);
	write.table('force_new_search_p = true', filename, row.names=F, col.names=F, quote=F, append=T);
}

acwriterparamsfile <- function(filename,sigmacontours) {
	filename = paste(filename, ".r-params", sep="")
	write.table('report_mode = "data"', filename, sep="\t", row.names=F, col.names=F, quote=F);
	write.table('order_attributes_by_influence_p = false', filename, row.names=F, col.names=F, quote=F, append=T);
	satt = NULL
	for(x in 1:length(sigmacontours-1)) {
		satt=paste(satt, sigmacontours[x], sep=",")
	}
	satt = substr(satt,2,nchar(satt))
	write.table(paste('sigma_contours_att_list = ', satt, sep=""), filename, row.names=F, col.names=F, quote=F, append=T);
}

acgetsearchresults <- function(filename) {
	filename = paste(filename, ".results", sep = "")
    d = read.table(filename, sep = "\t")
    ind = which(d[, 1] == "n_data, n_atts, input_n_atts")
    N = as.numeric(unlist(strsplit(as.character(d[ind + 1, ]), " "))[2])
    ind = (which(d[, 1] == "n_classes")[1]) + 1
    K = as.numeric(as.character(d[ind, 1]))
    indcov = which(d[, 1] == "emp_covar")
    class_weight = matrix(nrow = 1, ncol = K)
    class_mean = matrix(nrow = N, ncol = K)
    class_Sigma = array(dim = c(N, N, K))
    wind = which(d[, 1] == "w_j, pi_j") + 1
    mind = which(d[, 1] == "emp_means") + 1
    for (k in 1:K) {
        class_weight[k] = as.numeric(unlist(strsplit(as.character(d[wind[k], ]), " "))[2])
		v = ceiling(N/10)
        men = NULL
        for(x in 1:v) {
			ii = x-1
        	p= as.numeric(unlist(strsplit(as.character(d[mind[k]+ii, ]), " ")))
        	men=c(men,p)
        }
        class_mean[, k] =men
        part = d[indcov[k]:indcov[k + 1], ]
        p = grep("row", part) + 1
		for (n in 1:N) {
        	ppp=NULL
			for(x in 1:v) {
				ii = x-1
				pp = as.numeric(unlist(strsplit(as.character(part[p[n]+ii]), " ")))
				ppp=c(ppp,pp)
			}
			class_Sigma[, n, k] = ppp
		}
    }	
	return(list("class_weight"=class_weight, "class_mean" = class_mean, "class_Sigma"=class_Sigma))
}

acgetsigmaresults <- function(filename) {
	reportfile = paste(filename, ".influ-no-data-1", sep="")
	d = read.table(reportfile, sep="\t")
	wind = grep("PROBABILITY", d[,1])
	f = d[wind[1],1]
	K = as.numeric(unlist(strsplit(as.character(f), " "))[7])
	ind = grep("SIGMA CONTOURS", d[,1])+2
	class_sc_mean_x = vector()
	class_sc_sigma_x = vector()
	class_sc_mean_y = vector()
	class_sc_sigma_y = vector()
	class_sc_rot = vector()
	for(k in 1:K) {
		lin = as.numeric(unlist(strsplit(as.character(d[ind[k],]), " ")))
		lin = lin[complete.cases(lin)]
		class_sc_mean_x[k] = lin[3]
		class_sc_sigma_x[k] = lin[4]
		class_sc_mean_y[k] = lin[5]
		class_sc_sigma_y[k] = lin[6]
		class_sc_rot[k] = lin[7]
	}
	return(list("class_sc_mean_x"=class_sc_mean_x, "class_sc_sigma_x"=class_sc_sigma_x, "class_sc_mean_y"=class_sc_mean_y, "class_sc_sigma_y"=class_sc_sigma_y, "class_sc_rot"=class_sc_rot))
}

acsearch <- function(filename) {
	#library.dynam("autoclassR", "autoclassR", paste(system.file(package="autoclassR"), "/../", sep=""))
	datafile= paste(filename, ".db2", sep="")
	headerfile= paste(filename, ".hd2", sep="")
	modelfile= paste(filename, ".model", sep="")
	sparamfile= paste(filename, ".s-params", sep="")
	Sys.sleep(2)
	.C("autoSearch",
		"f1" = as.character(datafile) 
		,"f2" = as.character(headerfile) 
		,"f3" = as.character(modelfile) 
		,"f4" = as.character(sparamfile)
		,"PACKAGE" = "autoclassR")
	#library.dynam.unload("autoclassR", system.file(package="autoclassR"))
}

acsigmacontours <- function(filename) {
	#library.dynam("autoclassR", "autoclassR", paste(system.file(package="autoclassR"), "/../", sep=""))
	resultfile= paste(filename, ".results", sep="")
	searchfile= paste(filename, ".search", sep="")
	rparamfile= paste(filename, ".r-params", sep="")
	Sys.sleep(2)
	.C("autoReport",
		"f1" = as.character(resultfile) 
		,"f2" = as.character(searchfile) 
		,"f3" = as.character(rparamfile) 
		,"PACKAGE" = "autoclassR")
	#library.dynam.unload("autoclassR", system.file(package="autoclassR"))
}

overlay.elipse <- function(x0, y0, a, b, alpha) {
	theta <- seq(0, 2 * pi, length=(200))
	x <- x0 + (3*a) * cos(theta) * cos(alpha) - (3*b) * sin(theta) * sin(alpha)
	y <- y0 + (3*a) * cos(theta) * sin(alpha) + (3*b) * sin(theta) * cos(alpha)
	matplot(x, y, type = "l", col="blue", add=T)
}

clean <- function(filename) {
	files = c(paste(filename, ".db2", sep=""),
		paste(filename, ".hd2", sep=""),
		paste(filename, ".model", sep=""),
		paste(filename, ".s-params", sep=""),
		paste(filename, ".results", sep=""),
		paste(filename, ".search", sep=""),
		paste(filename, ".r-params", sep=""),
		paste(filename, ".log", sep=""),
		paste(filename, ".case-data-1", sep=""),
		paste(filename, ".class-data-1", sep=""),
		paste(filename, ".influ-no-data-1", sep=""),
		paste(filename, ".rlog", sep="")
	)
	for(x in 1:length(files)) {
		command = paste("rm ", files[x], sep="")
		system(command)
	}
}

autoclassR <- function(d=NULL, filename=NULL, clean=T) {
		
	if(is.null(filename)) {
		filename = paste(getwd(), "/thisisanautoclassRestimation", sep="")	
	}
	acwritedatafile(filename, d)
	acwriteheaderfile(filename, dim(d)[2])
	acwritemodelfile(filename, dim(d)[2])
	acwritesparamsfile(filename)
	
	acsearch(filename)
	res = acgetsearchresults(filename)
	
	sigmacontours=c(0,1)
	acwriterparamsfile(filename, sigmacontours)
	
	acsigmacontours(filename)
	sigcontrs = acgetsigmaresults(filename)
	
	nclasses = length(res$class_weight)

	plot(d[,1],d[,2])
	matplot(res$class_mean[1,], res$class_mean[2,], add=T, pch="+", col="red", cex=2)

	for(k in 1:nclasses) {
		overlay.elipse(sigcontrs$class_sc_mean_x[k], sigcontrs$class_sc_mean_y[k], sigcontrs$class_sc_sigma_x[k], sigcontrs$class_sc_sigma_y[k], sigcontrs$class_sc_rot[k])
	}
	
	if(clean) {
		clean(filename)
	}
	
	return(list("searchresult"=res, "sigcontrs"=sigcontrs))
}

example_autoclass <- function(filename=NULL, clean=T) {
		
	data("testdata")
	if(is.null(filename)) {
		filename = paste(getwd(), "/thisisanautoclassRexample", sep="")	
	}
	acwritedatafile(filename, d)
	acwriteheaderfile(filename, dim(d)[2])
	acwritemodelfile(filename, dim(d)[2])
	acwritesparamsfile(filename)
	
	acsearch(filename)
	res = acgetsearchresults(filename)
	
	sigmacontours=c(0,1)
	acwriterparamsfile(filename, sigmacontours)
	
	acsigmacontours(filename)
	sigcontrs = acgetsigmaresults(filename)
	
	nclasses = length(res$class_weight)

	plot(d[,1],d[,2])
	matplot(res$class_mean[1,], res$class_mean[2,], add=T, pch="+", col="red", cex=2)

	for(k in 1:nclasses) {
		overlay.elipse(sigcontrs$class_sc_mean_x[k], sigcontrs$class_sc_mean_y[k], sigcontrs$class_sc_sigma_x[k], sigcontrs$class_sc_sigma_y[k], sigcontrs$class_sc_rot[k])
	}
	
	if(clean) {
		clean(filename)
	}
	
	return(list("searchresult"=res, "sigcontrs"=sigcontrs))
}