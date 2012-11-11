#!/bin/bash

tests_total=0;
passed_test_count=0;
failed_test_count=0;
declare -a failed_tests=();

bindir=$(readlink -f $(dirname $0));
cd $bindir;

echo "Running tests..."

for i in test_*; do
	let tests_total+=1;
	bash $i
	rc=$?
	if [[ "$rc" != "0" ]]; then
		let failed_test_count+=1;
		failed_tests=( ${failed_tests[@]} $i );
		echo "FAILED: $i"
	else 
		let passed_test_count+=1;
		echo "PASSED: $i"
	fi
done;

echo ""
printf "Total: %d\tPassed: %d\tFailed: %d\n" $tests_total $passed_test_count $failed_test_count;
if [ "$failed_test_count" -ne "0" ]; then 
	echo "Failed tests:"
	for f in "${failed_tests[@]}"; do
		echo "      $f";
	done
fi
