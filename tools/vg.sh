#!/bin/bash
# Shorthand script for valgrid memory checking
# Iestyn Pryce 2012

valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes $@
