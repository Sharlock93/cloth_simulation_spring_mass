def Settings(**kwargs):
	return {
		"flags": ["-I", "src/", "-I", "include", "-I", "gsl/include/"],
		"override_filename" : "src/main.c"
	}
