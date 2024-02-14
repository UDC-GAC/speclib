/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     common_cgetopt.cpp
/// \brief    Custom global 'getopt' implementation (based on glibc one)
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include <iostream>
#include <cstring>

class GetOptClass {
public:
	static constexpr bool POSIXLY_CORRECT_DEFAULT = false;

private:
	enum __ord {
		REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
	};

	struct _getopt_data {
		int optind;
		int opterr;
		int optopt;
		char *optarg = nullptr;
		int __initialized = 0;
		char *__nextchar = nullptr;
		enum __ord __ordering = PERMUTE;
		int __first_nonopt = 0;
		int __last_nonopt = 0;
	};

	struct option {
		const char *name;
		int has_arg;
		int *flag = nullptr;
		int val;
	};

public:
	char *optarg;
	int optind = 1;
	int opterr = 1;
	int optopt = '?';
private:

	_getopt_data getopt_data;

	static void exchange(char **argv, _getopt_data *d) {
		int bottom = d->__first_nonopt;
		int middle = d->__last_nonopt;
		int top = d->optind;
		char *tem;
		while (top > middle && middle > bottom) {
			if (top - middle > middle - bottom) {
				int len = middle - bottom;
				int i;
				for (i = 0; i < len; i++) {
					tem = argv[bottom + i];
					argv[bottom + i] = argv[top - (middle - bottom) + i];
					argv[top - (middle - bottom) + i] = tem;
				}
				top -= len;
			} else {
				int len = top - middle;
				int i;
				for (i = 0; i < len; i++) {
					tem = argv[bottom + i];
					argv[bottom + i] = argv[middle + i];
					argv[middle + i] = tem;
				}
				bottom += len;
			}
		}
		d->__first_nonopt += (d->optind - d->__last_nonopt);
		d->__last_nonopt = d->optind;
	}

	static const char * _getopt_initialize(const char *optstring, _getopt_data *d, const bool posixly_correct) {
		if (d->optind == 0) {
			d->optind = 1;
		}
		d->__last_nonopt = d->optind;
		d->__first_nonopt = d->__last_nonopt;
		d->__nextchar = nullptr;
		if (optstring[0] == '-') {
			d->__ordering = RETURN_IN_ORDER;
			++optstring;
		} else if (optstring[0] == '+') {
			d->__ordering = REQUIRE_ORDER;
			++optstring;
		} else if (posixly_correct) {
			d->__ordering = REQUIRE_ORDER;
		} else {
			d->__ordering = PERMUTE;
		}
		d->__initialized = 1;
		return optstring;
	}

	static int process_long_option(int argc, char **argv, const char *optstring, const option *longopts, int *longind, int long_only, _getopt_data *d, int print_errors, const char *prefix) {
		char *nameend;
		size_t namelen;
		const option *p;
		const option *pfound = nullptr;
		int n_options;
		int option_index;
		for (nameend = d->__nextchar; *nameend && *nameend != '='; nameend++) {
			;	// NOP
		}
		namelen = (size_t)(nameend - d->__nextchar);
		for (p = longopts, n_options = 0; p->name; p++, n_options++) {
			if (!strncmp(p->name, d->__nextchar, namelen) && namelen == strlen(p->name)) {
				pfound = p;
				option_index = n_options;
				break;
			}
		}
		if (pfound == nullptr) {
			unsigned char *ambig_set = nullptr;
			int ambig_malloced = 0;
			int ambig_fallback = 0;
			int indfound = -1;
			for (p = longopts, option_index = 0; p->name; p++, option_index++) {
				if (!strncmp(p->name, d->__nextchar, namelen)) {
					if (pfound == nullptr) {
						pfound = p;
						indfound = option_index;
					} else if (long_only || pfound->has_arg != p->has_arg || pfound->flag != p->flag || pfound->val != p->val) {
						if (!ambig_fallback) {
							if (!print_errors) {
								ambig_fallback = 1;
							} else if (!ambig_set) {
								ambig_set = new (std::nothrow) unsigned char[(size_t)n_options];
								if (!ambig_set) {
									ambig_fallback = 1;
								} else {
									ambig_malloced = 1;
								}
								if (ambig_set) {
									memset(ambig_set, 0, (size_t)n_options);
									ambig_set[indfound] = 1;
								}
							}
							if (ambig_set) {
								ambig_set[option_index] = 1;
							}
						}
					}
				}
			}
			if (ambig_set || ambig_fallback) {
				if (print_errors) {
					if (ambig_fallback) {
						std::cerr << argv[0] << ": option '" << prefix << d->__nextchar << "' is ambiguous" << std::endl;
					} else {
						std::cerr << argv[0] << ": option '" << prefix << d->__nextchar << "' is ambiguous; possibilities:";
						for (option_index = 0; option_index < n_options; option_index++) {
							if (ambig_set[option_index]) {
								std::cerr << " '" << prefix << longopts[option_index].name << "'";
							}
						}
						std::cerr << std::endl;
					}
				}
				if (ambig_malloced) {
					delete[] ambig_set;
				}
				d->__nextchar += strlen(d->__nextchar);
				d->optind++;
				d->optopt = 0;
				return '?';
			}
			option_index = indfound;
		}
		if (pfound == nullptr) {
			if (!long_only || argv[d->optind][1] == '-' || strchr(optstring, *d->__nextchar) == nullptr) {
				if (print_errors) {
					std::cerr << argv[0] << ": unrecognized option '" << prefix << d->__nextchar << "'" << std::endl;
				}
				d->__nextchar = nullptr;
				d->optind++;
				d->optopt = 0;
				return '?';
			}
			return -1;
		}
		d->optind++;
		d->__nextchar = nullptr;
		if (*nameend) {
			if (pfound->has_arg) {
				d->optarg = nameend + 1;
			} else {
				if (print_errors) {
					std::cerr << argv[0] << ": option '" << prefix << pfound->name << "' doesn't allow an argument" << std::endl;
				}
				d->optopt = pfound->val;
				return '?';
			}
		} else if (pfound->has_arg == 1) {
			if (d->optind < argc) {
				d->optarg = argv[d->optind++];
			} else {
			if (print_errors) {
				std::cerr << argv[0] << ": option '" << prefix << pfound->name << "' requires an argument" << std::endl;
			}
			d->optopt = pfound->val;
			return optstring[0] == ':' ? ':' : '?';
			}
		}
		if (longind != nullptr) {
			*longind = option_index;
		}
		if (pfound->flag) {
			*(pfound->flag) = pfound->val;
			return 0;
		}
		return pfound->val;
	}

	static int _getopt_internal_r(int argc, char **argv, const char *optstring, const option *longopts, int *longind, int long_only, _getopt_data *d, const bool posixly_correct) {
		int print_errors = d->opterr;
		if (argc < 1) {
			return -1;
		}
		d->optarg = nullptr;
		if (d->optind == 0 || !d->__initialized) {
			optstring = _getopt_initialize(optstring, d, posixly_correct);
		} else if (optstring[0] == '-' || optstring[0] == '+') {
			optstring++;
		}
		if (optstring[0] == ':') {
			print_errors = 0;
		}
		if (d->__nextchar == nullptr || *d->__nextchar == '\0') {
			if (d->__last_nonopt > d->optind) {
				d->__last_nonopt = d->optind;
			}
			if (d->__first_nonopt > d->optind) {
				d->__first_nonopt = d->optind;
			}
			if (d->__ordering == PERMUTE) {
				if (d->__first_nonopt != d->__last_nonopt && d->__last_nonopt != d->optind) {
					exchange(argv, d);
				} else if (d->__last_nonopt != d->optind) {
					d->__first_nonopt = d->optind;
				}
				while (d->optind < argc && (argv[d->optind][0] != '-' || argv[d->optind][1] == '\0')) {
					d->optind++;
				}
				d->__last_nonopt = d->optind;
			}
			if (d->optind != argc && !strcmp (argv[d->optind], "--")) {
				d->optind++;
				if (d->__first_nonopt != d->__last_nonopt && d->__last_nonopt != d->optind) {
					exchange(argv, d);
				} else if (d->__first_nonopt == d->__last_nonopt) {
					d->__first_nonopt = d->optind;
				}
				d->__last_nonopt = argc;
				d->optind = argc;
			}
			if (d->optind == argc) {
				if (d->__first_nonopt != d->__last_nonopt) {
					d->optind = d->__first_nonopt;
				}
				return -1;
			}
			if (argv[d->optind][0] != '-' || argv[d->optind][1] == '\0') {
				if (d->__ordering == REQUIRE_ORDER) {
					return -1;
				}
				d->optarg = argv[d->optind++];
				return 1;
			}
			if (longopts) {
				if (argv[d->optind][1] == '-') {
					d->__nextchar = argv[d->optind] + 2;
					return process_long_option(argc, argv, optstring, longopts, longind, long_only, d, print_errors, "--");
				}
				if (long_only && (argv[d->optind][2] || !strchr(optstring, argv[d->optind][1]))) {
					int code;
					d->__nextchar = argv[d->optind] + 1;
					code = process_long_option(argc, argv, optstring, longopts, longind, long_only, d, print_errors, "-");
					if (code != -1) {
						return code;
					}
				}
			}
			d->__nextchar = argv[d->optind] + 1;
		}
		{
			char c = *d->__nextchar++;
			const char *temp = strchr(optstring, c);
			if (*d->__nextchar == '\0') {
				++d->optind;
			}
			if (temp == nullptr || c == ':' || c == ';') {
				if (print_errors) {
					std::cerr << argv[0] << ": invalid option -- '" << c << "'" << std::endl;
				}
				d->optopt = c;
				return '?';
			}
			if (temp[0] == 'W' && temp[1] == ';' && longopts != nullptr) {
				if (*d->__nextchar != '\0') {
					d->optarg = d->__nextchar;
				} else if (d->optind == argc) {
					if (print_errors) {
						std::cerr << argv[0] << ": option requires an argument -- '" << c << "'\n" << std::endl;
					}
					d->optopt = c;
					if (optstring[0] == ':') {
						c = ':';
					} else {
						c = '?';
					}
					return c;
				} else {
					d->optarg = argv[d->optind];
				}
				d->__nextchar = d->optarg;
				d->optarg = nullptr;
				return process_long_option(argc, argv, optstring, longopts, longind, 0, d, print_errors, "-W ");
			}
			if (temp[1] == ':') {
				if (temp[2] == ':') {
					if (*d->__nextchar != '\0') {
						d->optarg = d->__nextchar;
						d->optind++;
					} else {
						d->optarg = nullptr;
					}
					d->__nextchar = nullptr;
				} else {
					if (*d->__nextchar != '\0') {
						d->optarg = d->__nextchar;
						d->optind++;
					} else if (d->optind == argc) {
						if (print_errors) {
							std::cerr << argv[0] << ": option requires an argument -- '" << c << "'" << std::endl;
						}
						d->optopt = c;
						if (optstring[0] == ':') {
							c = ':';
						} else {
							c = '?';
						}
					} else {
						d->optarg = argv[d->optind++];
					}
					d->__nextchar = nullptr;
				}
			}
			return c;
		}
	}

	int _getopt_internal(int argc, char **argv, const char *optstring, const option *longopts, int *longind, int long_only, const bool posixly_correct) {
		int result;
		getopt_data.optind = optind;
		getopt_data.opterr = opterr;
		result = _getopt_internal_r(argc, argv, optstring, longopts, longind, long_only, &getopt_data, posixly_correct);
		optind = getopt_data.optind;
		optarg = getopt_data.optarg;
		optopt = getopt_data.optopt;
		return result;
	}

public:
	int getopt(int argc, char **argv, const char *optstring, const bool posixly_correct = POSIXLY_CORRECT_DEFAULT) {
		return _getopt_internal(argc, argv, optstring, nullptr, nullptr, 0, posixly_correct);
	}

};
