#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern int opterr, optind, optopt, optreset;
extern char *optarg;

extern int getopt(int nargc, char *const nargv[], const char *ostr);

#ifdef __cplusplus
}
#endif
