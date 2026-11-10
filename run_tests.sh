#!/bin/bash
# group-68

PASS=0
FAIL=0
TOTAL=0

# Build first
make clean && make
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

for infile in tests/*.in; do
    testname=$(basename "$infile" .in)
    outfile="tests/${testname}.out"

    if [ ! -f "$outfile" ]; then
        echo "SKIP $testname (no expected output)"
        continue
    fi

    TOTAL=$((TOTAL + 1))

    actual=$(./acaderp < "$infile" 2>/dev/null)
    expected=$(cat "$outfile")

    if [ "$actual" = "$expected" ]; then
        echo "PASS $testname"
        PASS=$((PASS + 1))
    else
        echo "FAIL $testname"
        FAIL=$((FAIL + 1))
        echo "  Expected:"
        echo "$expected" | head -5
        echo "  Got:"
        echo "$actual" | head -5
    fi
done

echo ""
echo "Results: $PASS/$TOTAL passed, $FAIL failed"

